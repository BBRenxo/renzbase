# RenzBase

**A modern Roblox script executor.**

Stack:
- **DLL** (forked from [RavageDevs/SkidBase](https://github.com/RavageDevs/SkidBase)) — provides Lua API (writefile, firetouchinterest, etc)
- **Injector** (volx-derived) — patches Hyperion's `.byfron` section
- **UI** (Tauri + Monaco) — editor tabs, file explorer, AutoExecute folder, attach button, console

## Why RenzBase?

- **Named pipe IPC** (`\\.\pipe\RenzBase`) — invisible to anti-cheat
- **100% UNC + sUNC coverage goal** — see [coverage tracker](https://github.com/BBRenxo/SkidBase/blob/main/UNC-sUNC-COVERAGE.md)
- **Auto-sync from upstream SkidBase** — weekly GitHub Action merges RavageDevs changes
- **Modern UI** — Tauri binary ~5MB, Monaco editor (same as VS Code), dark theme

## Architecture

```
┌─────────────────────────────────────────────┐
│ RenzUI.exe (Tauri)                          │
│  - Monaco editor tabs                       │
│  - Explorer sidebar with AutoExecute        │
│  - Attach button → spawns RenzInjector.exe  │
│  - Console output panel                     │
└────────┬────────────────────────────────────┘
         │ spawns + watches
         ↓
┌─────────────────────────────────────────────┐
│ RenzInjector.exe                            │
│  - Patch .byfron (Hyperion bypass)          │
│  - Manual map Module.dll                    │
│  - Resume process                           │
└────────┬────────────────────────────────────┘
         │ maps
         ↓
┌─────────────────────────────────────────────┐
│ Module.dll (SkidBase, our fork)             │
│  - Find lua_State                           │
│  - Register Lua API                         │
│  - Listen on \\.\pipe\RenzBase              │
│  - Execute scripts from pipe                │
└─────────────────────────────────────────────┘
```

## Components

| Component     | Source                                | Status      |
|---------------|---------------------------------------|-------------|
| DLL           | [BBRenxo/SkidBase](https://github.com/BBRenxo/SkidBase) | Active fork |
| Injector      | volx/Integrity (modified)             | To build    |
| UI            | Tauri + Monaco                        | To build    |

## Setup

### Build the DLL
```bash
cd SkidBase
msbuild SkidBase.vcxproj -p:Configuration=Release -p:Platform=x64
# Output: Release/Module.dll
```

### Build the injector (TODO)
```bash
cd injector/RenzInjector
msbuild RenzInjector.sln -p:Configuration=Release -p:Platform=x64
```

### Build the UI (TODO)
```bash
cd ui
npm install
npm run tauri build
# Output: target/release/renz-ui.exe
```

## Documentation

- [DLL docs](https://github.com/BBRenxo/SkidBase/blob/main/ARCHITECTURE.md)
- [UNC/sUNC docs](https://github.com/BBRenxo/SkidBase/blob/main/UNC-sUNC.md)
- [Coverage tracker](https://github.com/BBRenxo/SkidBase/blob/main/UNC-sUNC-COVERAGE.md)

## License

Inherited from SkidBase.
