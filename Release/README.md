# RenzBase Release

**Drop these files in any folder. Run them in order.**

## What's in here

| File | Purpose |
|------|---------|
| `Module.dll` | The executor DLL — gets injected into RobloxPlayerBeta.exe |
| `Loader.exe` | The loader — injects Module.dll into the running Roblox process |
| `Injector.exe` | Alt: volx-style manual mapper (if Loader.exe is blocked) |
| `send_script.py` | Python client — sends Lua scripts to Module.dll via `\\.\pipe\RenzBase` |

## Requirements

- **Roblox version**: `version-e7d81637d42c4b23` (0.737.0.7371584)
- **Hyperion bypass**: Potassium (`Attach` first if Roblox crashes on inject)
- **OS**: Windows 10/11 x64

## Quick Start

1. **Launch Roblox** and join any game (Fair Dunc Lab recommended for testing)
2. **Bypass Hyperion** with Potassium (optional but recommended):
   - Run `Potassium.exe`
   - Click `Attach` — wait for "Attached"
3. **Inject Module.dll**:
   ```cmd
   Loader.exe
   ```
   You should see `[RenzBase] Pipe server listening on \\.\pipe\RenzBase`
4. **Send a script**:
   ```bash
   python send_script.py your_script.lua
   ```
   Or pipe via stdin:
   ```bash
   echo 'print("hello from renzbase")' | python send_script.py
   ```

## Built from

- **Source**: https://github.com/BBRenxo/SkidBase (fork of RavageDevs/SkidBase)
- **Offsets**: `latest_offsets.hpp` (supQ Runtime Dumper v2, Sep 2 2026)

## Offset Summary (version-e7d81637d42c4b23)

```
Print                  = 0x1C8A050
GetGlobalState         = 0x406E600  (LuaVMLoad)
LuauExecute            = 0x26D4770  (LuaD_Call)
LuaDThrow              = 0x26D4690  (luaD_throw)
OpcodeLookupTable      = 0x61D1700
FakeDataModel          = 0x8D22868
LuaNil                 = 0x63516D8
LuaDummy               = 0x6351188
TargetFPS              = 0x8109DE8
IdentityPointer        = 0x80C10C8
GetTlsPointer          = 0x39A0
GetCapabilities        = 0x1CC5820
TaskScheduler          = 0x8ABD728
GameLoaded             = 0x630
PushInstance           = 0x40608A0
CastArgs               = 0x3FDE600
```

## IPC

The DLL creates a **named pipe** server: `\\.\pipe\RenzBase` (PIPE_TYPE_BYTE mode).
send_script.py connects to it and sends a Lua script as a single message.

There is also a **legacy winsock** server on port 9002 (compile-time flag).

## Test Score (current)

- **UNC**: ~75% (49/65 — pre-offset-update baseline)
- **sUNC**: 75% (49/65 in Fair Dunc Lab)

## License

MIT — see ../LICENSE
