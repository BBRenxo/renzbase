# Architecture

## Components

RenzBase is a 3-component executor stack:

### 1. `dll/` — Module.dll (Lua API runtime)

Forked from [RavageDevs/SkidBase](https://github.com/RavageDevs/SkidBase). 736KB.

**What it does**:
- Resolves Roblox functions (`Print`, `LuauExecute`, `GetGlobalState`, `GetGlobalState`, `LuaDThrow`, `GetCapabilities`) via hardcoded RVAs + `GetModuleHandleA(nullptr)`
- Walks `global_State` → `main_thread` (lua_State)
- Registers Lua APIs under Roblox's lua_State: `writefile`, `crypt.hash`, `firetouchinterest`, `identifyexecutor`, etc
- Spawns a thread that listens on `\\.\pipe\RenzBase` (named pipe, PIPE_TYPE_BYTE mode)
- Spawns a thread that listens on TCP port 9002 (legacy winsock)
- On script receive: `luaL_loadstring` → `luau_execute` → output via `print`/`warn`/`error`

**Key files**:
- `Source/Core/dllmain.cpp` — entry, pipe listener, TeleportHandler thread
- `Source/Core/Execution/Execution.h/.cpp` — caps (identity-8), extexecute queue, flagfix()
- `Source/Core/unc/Unc.h/.cpp` — safe_apply_unc with validation, default Off mode
- `Source/Core/TeleportHandler.cpp` — calls env::Register(skidstate) on teleport
- `Source/Roblox/Offsets/Offsets.h` — hardcoded RVAs (Print, LuauExecute, FakeDataModel, etc)
- `Source/Roblox/Environment/Libs/*/*.h` — every Lua library

### 2. `injector/` — Injector.exe (Loader)

Volx-derived, simplified. 44KB.

**What it does**:
- Opens Roblox process handle (PROCESS_ALL_ACCESS)
- Patches Hyperion `.byfron` section (RVAs from volx's Patches.h)
- Manual-maps `Module.dll` into Roblox via remote thread + shellcode
- Resumes process, closes handle

**Limitations**:
- volx's `.byfron` RVAs are stale (for old Roblox). Without current offsets, Hyperion detects injection and crashes Roblox.
- **Workaround**: use Potassium Attach first (Potassium.dll handles Hyperion bypass), then run Loader.exe or our Injector.exe — Hyperion is already bypassed, so manual map succeeds.

### 3. `ui/` — SkidUI (Tauri editor)

Tauri + Rust + HTML/JS. Source only (binary build needs more RAM than available).

**What it does**:
- Monaco editor (same engine as VS Code)
- Tab system for multiple scripts
- File explorer with AutoExecute folder
- Attach button → spawns Injector.exe
- Console panel showing DLL output
- Settings panel (executor name, theme, etc)

## Named Pipe IPC

The DLL listens on `\\.\pipe\RenzBase` (PIPE_TYPE_BYTE mode):

```
DLL side:                              Client side:
CreateNamedPipeW(...)                  CreateFileW("\\\\.\\pipe\\RenzBase", ...)
ConnectNamedPipe(...)                  WriteFile(handle, script_bytes, ...)
ReadFile(...)                          CloseHandle(...)
LuaL_loadstring(script)                ...repeat
luau_execute(L)
```

`sed_script.py` is a 4KB Python client. Tauri UI does the same thing in Rust.

## Offset Lifecycle

When Roblox patches (every ~2-4 weeks):

1. **theo** dumps offsets to JSON: `https://offsets.imtheo.lol/offsets.json`
2. **We fetch** via `scripts/update_offsets.py`
3. **We diff** old vs new offsets
4. **We update** `dll/Source/Roblox/Offsets/Offsets.h` (just the changed RVA)
5. **We rebuild** DLL
6. **We rebuild** Injector.exe (Patches.h RVAs also change)

Auto-update script supports 3 mirrors with automatic failover:
- `https://offsets.imtheo.lol/offsets.json` (main)
- `https://offsets.femboythighs.org/offsets.json` (dedicated mirror)
- `https://offsets.uwuhook.club/offsets.json` (domain-only)

## sUNC Coverage

Tested with [Fair Dunc Lab](https://github.com/Dertme314/External-Sunc-test) v4.6:

| Category    | Pass | Total | % |
|-------------|------|-------|---|
| Input       | 7    | 7     | 100% |
| Console     | 4    | 4     | 100% |
| Metatable   | 3    | 3     | 100% |
| Thread      | 2    | 2     | 100% |
| FileSystem  | 9    | 10    | 90% |
| Crypt       | 8    | 10    | 80% |
| Closures    | 5    | 7     | 71% |
| Environment | 3    | 5     | 60% |
| Misc        | 4    | 8     | 50% |
| Network     | 1    | 2     | 50% |
| Drawing     | 3    | 9     | 33% |
| **Total**   | **49** | **65** | **75%** |

## Future Work

- **Hyperion bypass** (proper, no need for Potassium Attach)
  - Find current .byfron RVAs (need fresh RE for version-e7d81637d42c4b23)
  - Patch integrity checks at runtime
  - Implement VM enter hook for bytecode obfuscation
- **Auto-update in DLL** — DLL could fetch offsets on first run
- **UI binary** — needs build on 16GB+ machine (4.8GB available here isn't enough for Rust compile)
- **Drawing lib** — implement Drawing.new("Line"/"Text"/"Square"/"Circle") with real userdata
- **Misc lib** — getgc, getconnections, getrunningscripts, getsenv
- **Network lib** — `request()` returns proper {Success, StatusCode, Body, Headers} table
- **Debug lib** — server-verified tests require accurate getgc + getinfo
