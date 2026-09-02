// RenzBase UI - Tauri backend
//
// Exposes commands to the JS frontend:
//   - execute_script(script: String) -> Result<(), String>
//       Connects to \\.\pipe\RenzBase, writes the script, closes.
//       The injected Module.dll listens on the pipe and runs the Lua.
//   - spawn_injector() -> Result<(), String>
//       Launches ../injector/RenzInjector/x64/Release/RenzInjector.exe
//       relative to this binary's location.
//   - list_scripts() -> Result<Vec<ScriptEntry>, String>
//       Reads %APPDATA%\RenzBase\scripts\ and returns the file tree.
//   - save_script(name, content) -> Result<String, String>
//       Writes a script file under %APPDATA%\RenzBase\scripts\<name>.

use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;
use std::time::Duration;

use tauri::{Emitter, Manager};

#[cfg(windows)]
use windows::core::{PCWSTR, PWSTR};
#[cfg(windows)]
use windows::Win32::Foundation::{CloseHandle, HANDLE};
#[cfg(windows)]
use windows::Win32::Storage::FileSystem::{
    CreateFileW, WriteFileW, FILE_FLAGS_AND_ATTRIBUTES, FILE_GENERIC_READ, FILE_GENERIC_WRITE,
    FILE_SHARE_READ, FILE_SHARE_WRITE, OPEN_EXISTING,
};
#[cfg(windows)]
use windows::Win32::System::Pipes::{
    SetNamedPipeHandleState, WaitNamedPipeW, PIPE_READMODE_MESSAGE,
};
#[cfg(windows)]
use windows::Win32::System::Threading::{CreateProcessW, PROCESS_INFORMATION, STARTUPINFOW};

#[cfg(windows)]
const PIPE_NAME: &str = r"\\.\pipe\RenzBase";

#[cfg(windows)]
fn wide(s: &str) -> Vec<u16> {
    s.encode_utf16().chain(std::iter::once(0)).collect()
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct ScriptEntry {
    pub name: String,
    pub path: String,
    pub is_folder: bool,
    pub is_autoexecute: bool,
    pub children: Vec<ScriptEntry>,
}

/// Returns %APPDATA%\RenzBase\scripts
fn scripts_dir() -> Result<PathBuf, String> {
    let appdata = std::env::var("APPDATA").map_err(|_| "APPDATA env var not set".to_string())?;
    let dir = PathBuf::from(appdata).join("RenzBase").join("scripts");
    if !dir.exists() {
        fs::create_dir_all(&dir).map_err(|e| format!("create scripts dir: {e}"))?;
    }
    Ok(dir)
}

/// Read the scripts directory tree.
#[tauri::command]
fn list_scripts() -> Result<Vec<ScriptEntry>, String> {
    let dir = scripts_dir()?;
    let mut out: Vec<ScriptEntry> = Vec::new();

    let entries = fs::read_dir(&dir).map_err(|e| format!("read scripts dir: {e}"))?;
    for entry in entries.flatten() {
        let path = entry.path();
        let name = entry.file_name().to_string_lossy().to_string();
        if path.is_dir() {
            let is_auto = name.eq_ignore_ascii_case("AutoExecute");
            let mut children: Vec<ScriptEntry> = Vec::new();
            if let Ok(rd) = fs::read_dir(&path) {
                for child in rd.flatten() {
                    let cp = child.path();
                    if cp.is_file() {
                        children.push(ScriptEntry {
                            name: child.file_name().to_string_lossy().to_string(),
                            path: cp.to_string_lossy().to_string(),
                            is_folder: false,
                            is_autoexecute: is_auto,
                            children: vec![],
                        });
                    }
                }
            }
            out.push(ScriptEntry {
                name,
                path: path.to_string_lossy().to_string(),
                is_folder: true,
                is_autoexecute: is_auto,
                children,
            });
        } else {
            out.push(ScriptEntry {
                name,
                path: path.to_string_lossy().to_string(),
                is_folder: false,
                is_autoexecute: false,
                children: vec![],
            });
        }
    }

    // Sort: folders first, then alphabetical (case-insensitive).
    out.sort_by(|a, b| match (a.is_folder, b.is_folder) {
        (true, false) => std::cmp::Ordering::Less,
        (false, true) => std::cmp::Ordering::Greater,
        _ => a.name.to_lowercase().cmp(&b.name.to_lowercase()),
    });
    Ok(out)
}

/// Persist a script to disk under %APPDATA%\RenzBase\scripts\<name>.
#[tauri::command]
fn save_script(name: String, content: String) -> Result<String, String> {
    let dir = scripts_dir()?;
    // Reject path traversal in the filename.
    if name.contains("..") || name.contains('/') || name.contains('\\') {
        return Err("invalid script name".to_string());
    }
    let path = dir.join(&name);
    fs::write(&path, content).map_err(|e| format!("write script: {e}"))?;
    Ok(path.to_string_lossy().to_string())
}

/// Send a script over the named pipe to the injected DLL.
#[tauri::command]
fn execute_script(script: String, app: tauri::AppHandle) -> Result<(), String> {
    if script.trim().is_empty() {
        return Err("script is empty".to_string());
    }

    #[cfg(windows)]
    {
        use std::ptr;
        unsafe {
            // Wait briefly for the pipe to exist (DLL may still be initializing).
            let mut waited = false;
            for _ in 0..50 {
                let pipe_w = wide(PIPE_NAME);
                if WaitNamedPipeW(PCWSTR(pipe_w.as_ptr()), 100) {
                    waited = true;
                    break;
                }
                std::thread::sleep(Duration::from_millis(100));
            }
            if !waited {
                let msg = "pipe not available (DLL not attached?)".to_string();
                let _ = app.emit("console", format!("[error] {msg}"));
                return Err(msg);
            }

            let pipe_w = wide(PIPE_NAME);
            let handle: HANDLE = CreateFileW(
                PCWSTR(pipe_w.as_ptr()),
                FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                ptr::null(),
                OPEN_EXISTING,
                FILE_FLAGS_AND_ATTRIBUTES(0),
                None,
            );
            if handle.is_invalid() {
                let err = std::io::Error::last_os_error();
                let msg = format!("CreateFileW failed: {err}");
                let _ = app.emit("console", format!("[error] {msg}"));
                return Err(msg);
            }

            // Switch to message-read mode so each WriteFile is one chunk.
            let mode: u32 = PIPE_READMODE_MESSAGE;
            let _ = SetNamedPipeHandleState(handle, Some(&mode), None, None);

            let mut bytes = script.into_bytes();
            let mut written: u32 = 0;
            let ok = WriteFileW(handle, bytes.as_mut_slice(), Some(&mut written), None);
            let _ = CloseHandle(handle);

            if ok.is_err() {
                let err = std::io::Error::last_os_error();
                let msg = format!("WriteFileW failed: {err}");
                let _ = app.emit("console", format!("[error] {msg}"));
                return Err(msg);
            }
            let _ = app.emit(
                "console",
                format!("[ok] wrote {written} bytes to {PIPE_NAME}"),
            );
            Ok(())
        }
    }

    #[cfg(not(windows))]
    {
        let _ = (script, app);
        Err("execute_script is only implemented on Windows".to_string())
    }
}

/// Launch the injector binary that ships alongside the UI exe.
#[tauri::command]
fn spawn_injector(app: tauri::AppHandle) -> Result<(), String> {
    #[cfg(windows)]
    {
        use std::ptr;
        unsafe {
            // Resolve ../injector/RenzInjector/x64/Release/RenzInjector.exe
            // relative to the running UI binary.
            let exe = std::env::current_exe().map_err(|e| format!("current_exe: {e}"))?;
            let base = exe.parent().ok_or("no parent dir for current_exe")?;
            let injector = base
                .join("..")
                .join("injector")
                .join("RenzInjector")
                .join("x64")
                .join("Release")
                .join("RenzInjector.exe");

            if !injector.exists() {
                let msg = format!("injector not found at {}", injector.display());
                let _ = app.emit("console", format!("[error] {msg}"));
                return Err(msg);
            }

            let cmd = wide(&format!("\"{}\"", injector.display()));
            let mut si: STARTUPINFOW = std::mem::zeroed();
            si.cb = std::mem::size_of::<STARTUPINFOW>() as u32;
            let mut pi: PROCESS_INFORMATION = std::mem::zeroed();

            let ok = CreateProcessW(
                PCWSTR(ptr::null()),
                PWSTR(cmd.as_ptr() as *mut u16),
                ptr::null_mut(),
                ptr::null_mut(),
                false,
                0,
                ptr::null_mut(),
                PCWSTR(ptr::null()),
                &si,
                &mut pi,
            );
            let _ = CloseHandle(pi.hThread);
            let _ = CloseHandle(pi.hProcess);

            if ok.is_err() {
                let err = std::io::Error::last_os_error();
                let msg = format!("CreateProcessW failed: {err}");
                let _ = app.emit("console", format!("[error] {msg}"));
                return Err(msg);
            }
            let _ = app.emit("console", "[ok] spawned injector".to_string());
            Ok(())
        }
    }

    #[cfg(not(windows))]
    {
        let _ = app;
        Err("spawn_injector is only implemented on Windows".to_string())
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![
            execute_script,
            spawn_injector,
            list_scripts,
            save_script,
        ])
        .setup(|app| {
            // Ensure the scripts directory exists at startup so the sidebar
            // doesn't crash before the first list_scripts() call.
            let _ = scripts_dir();
            let _ = app.get_webview_window("main");
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running RenzBase UI");
}
