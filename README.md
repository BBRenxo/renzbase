# RenzBase

> A modern Roblox script executor stack. DLL + injector + UI, all open source.

[![DLL Build](https://img.shields.io/badge/DLL-Built-44cc11)](https://github.com/BBRenxo/SkidBase)
[![Injector Build](https://img.shields.io/badge/Injector-Built-44cc11)](https://github.com/BBRenxo/renzbase/tree/main/injector)
[![UI Build](https://img.shields.io/badge/UI-In%20Progress-orange)](https://github.com/BBRenxo/renzbase/tree/main/ui)
[![sUNC Score](https://img.shields.io/badge/sUNC-75%25-yellowgreen)](https://github.com/BBRenxo/SkidBase)
[![License](https://img.shields.io/badge/License-MIT-blue)](LICENSE)

## What is RenzBase?

A complete Roblox executor in three parts:

1. **Module.dll** — Lua API runtime, hooks the script context, exposes `writefile`/`crypt`/`firetouchinterest`/etc
2. **Injector.exe** — Maps Module.dll into Roblox, patches Hyperion (when possible), listens on named pipe
3. **SkidUI** — Tauri-based editor (Monaco + tabs + console), sends scripts to DLL via named pipe

## Quick Start

### 1. Inject

```
C:\Users\Administrator\Desktop\RenzBase\
├── Module.dll      736KB  ← Lua API
├── Loader.exe      52KB   ← runs injection
├── Injector.exe    44KB   ← alt injector (volx-style)
└── send_script.py  4KB    ← Python pipe client
```

Steps:
1. Open Roblox + join any game
2. Open Potassium, click **Attach** (bypasses Hyperion)
3. Run `Loader.exe`
4. Check F9 console: should show `(ExecutorName) has handled teleporting`

### 2. Send scripts

```bash
# From git-bash / cmd, in the same folder:
python send_script.py "print('hello from renzbase')"

# Or send a file:
python send_script.py myscript.lua

# Or pipe via stdin (for large scripts):
cat myscript.lua | python send_script.py
```

The DLL receives the script over `\\.\pipe\RenzBase` and executes it via `luaL_loadstring` + `luau_execute`.

## Architecture

```
┌─────────────────────────────────────────────┐
│ SkidUI.exe (Tauri + Monaco)                  │
│  - Editor tabs                              │
│  - Explorer + AutoExecute folder            │
│  - Attach button → spawns Injector.exe      │
│  - Console panel (output from DLL)          │
│  - Save / Load scripts                      │
└────────┬────────────────────────────────────┘
         │ spawns + watches
         ↓
┌─────────────────────────────────────────────┐
│ Injector.exe (volx-derived)                 │
│  - Patch Hyperion integrity checks          │
│  - Manual map Module.dll into Roblox        │
│  - Resume process, close handle             │
└────────┬────────────────────────────────────┘
         │ loads
         ↓
┌─────────────────────────────────────────────┐
│ Module.dll (in Roblox address space)        │
│  - Find lua_State via GetGlobalState        │
│  - Register Lua API (writefile, crypt...)   │
│  - Listen on \\.\pipe\RenzBase (PIPE_BYTE)  │
│  - Listen on TCP port 9002 (legacy)         │
│  - On script receive:                       │
│      luaL_loadstring → luau_execute →      │
│      output via print/warn/error            │
└─────────────────────────────────────────────┘
```

## Components

| Component     | Source                                    | Build Status          |
|---------------|-------------------------------------------|-----------------------|
| `dll/`        | [BBRenxo/SkidBase](https://github.com/BBRenxo/SkidBase) | ✅ 736KB built |
| `injector/`   | volx/Integrity (simplified)               | ✅ 44KB built   |
| `ui/`         | Tauri + Monaco                            | 🚧 source only |
| `scripts/`    | Python pipe client                        | ✅ ready        |

## Lua API (what's in the DLL)

Every function `pcall`-safe, sandboxed to Roblox's lua_State.

### Environment
```lua
getgenv()                      -- _G wrapper
getrenv()                       -- full env wrapper
gethui()                       -- PlayerGui
getcustomasset(path)           -- writes file → returns rbxasset URI
identifyexecutor()             -- returns "RenzBase", "1.0.0"
```

### Closures
```lua
newcclosure(fn)               -- wrap Lua fn as C closure
clonefunction(fn)              -- independent copy
iscclosure(fn), islclosure(fn) -- check closure type
checkcaller()                   -- true if inside executor
loadstring(src)()               -- load + run
```

### FileSystem
```lua
writefile(path, data)
readfile(path)
appendfile(path, data)
loadfile(path)()
isfile(path), isfolder(path)
makefolder(path), delfile(path), delfolder(path)
listfiles(path)
```

### Crypt
```lua
crypt.base64encode(data)      -- string → b64
crypt.base64decode(b64)       -- b64 → string
crypt.hash(data, alg)         -- alg = md5/sha1/sha256/sha384/sha512
crypt.generatekey()
crypt.generatebytes(n)
crypt.encrypt(data, key, iv, mode)
crypt.decrypt(data, key, iv, mode)
crypt.lz4compress(data)
crypt.lz4decompress(data, original_size)
```

### Debug
```lua
debug.getconstant(fn, idx)    -- read constant
debug.getconstants(fn)         -- all constants
debug.setconstant(fn, idx, v) -- write constant
debug.getupvalue(fn, idx)
debug.setupvalue(fn, idx, v)
debug.getprotos(fn)            -- sub-functions
debug.getproto(fn, idx)
debug.getinfo(fn, level)
debug.getstack(level)
debug.setstack(level, idx, v)
debug.getfenv(fn), debug.setfenv(fn, env)
debug.traceback(msg, level)
```

### Console
```lua
rconsolecreate()
rconsoleprint(text)
rconsolename(title)
rconsoleclear()
rconsoleclose()
```

### Misc
```lua
getgc(include_tables)           -- walk Lua GC, return all objects
getconnections(signal)          -- all RBXScriptSignal handlers
getrunningscripts()             -- all currently-executing scripts
getsenv(script)                 -- get script's env
setclipboard(text)
isrbxactive()                   -- is Roblox window focused
setfpscap(n), getfpscap()
queue_on_teleport(code)
```

### Input
```lua
keypress(vk)                   -- tap key
keyrelease(vk)
mouse1click(), mouse1press(), mouse1release()
mouse2click(), mouse2press(), mouse2release()
mousemoveabs(x, y)
mousemoverel(dx, dy)
mousescroll(amount)
fireclickdetector(cd)
fireproximityprompt(pp)
firetouchinterest(part1, part2, toggle)
```

### Network
```lua
request({Url=..., Method=..., Headers=..., Body=...})
game:HttpGet(url)
game:HttpPost(url, data)
```

### Drawing
```lua
Drawing.new("Line")  -- returns Line object (userdata)
local l = Drawing.new("Line")
l.Visible = true
l.Color = Color3.new(1,0,0)
l.Thickness = 2
l.From = Vector2.new(0,0)
l.To = Vector2.new(100,100)
l:Remove()
```

### Thread / Identity
```lua
getthreadidentity()            -- 0/1/2/3/4/5/6/7/8
setthreadidentity(level)
```

### UNC/sUNC flags
```lua
getunc()                        -- "off" / "unc" / "sunc"
setunc(mode)                    -- toggles Hyperion flag patching
```

## sUNC Score (Live)

Tested with [Fair Dunc Lab](https://github.com/Dertme314/External-Sunc-test):

| Category    | Pass Rate |
|-------------|-----------|
| Input       | 100% ✅ |
| Console     | 100% ✅ |
| Metatable   | 100% ✅ |
| Thread      | 100% ✅ |
| FileSystem  | 90% |
| Crypt       | 80% |
| Closures    | 71% |
| Environment | 60% |
| Misc        | 50% |
| Network     | 50% |
| Drawing     | 33% |
| **Total**   | **75% (49/65)** |

## Build from Source

### DLL (SkidBase)
```bash
cd dll/
msbuild SkidBase.vcxproj -p:Configuration=Release -p:Platform=x64
# Output: Release/Module.dll (736KB)
```

### Injector
```bash
cd injector/RenzInjector/
msbuild RenzInjector.vcxproj -p:Configuration=Release -p:Platform=x64
# Output: Build/Injector.exe (44KB)
```

### UI
```bash
cd ui/
npm install
npm run tauri build
# Output: src-tauri/target/release/renz-ui.exe (~5MB)
```

## Updating Offsets

When Roblox patches:
1. Get fresh offsets from [theo](https://offsets.imtheo.lol) or [cheatoffsets.com](https://www.cheatoffsets.com)
2. Update `dll/Source/Roblox/Offsets/Offsets.h`
3. Rebuild DLL

The injector and UI don't need rebuilds (they're offset-agnostic).

## License

MIT — see [LICENSE](LICENSE).

DLL portions inherited from [RavageDevs/SkidBase](https://github.com/RavageDevs/SkidBase) (MIT).
Injector portions derived from volx/Integrity (educational reference).
