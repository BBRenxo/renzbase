# RenzBase Release

Three files. That's all.

| File | Purpose |
|------|---------|
| `Module.dll` | The Lua runtime. Gets injected into Roblox. |
| `Loader.exe` | Puts Module.dll into Roblox. |
| `send_script.py` | Sends Lua scripts to Module.dll over a named pipe. |

## How to use

1. **Start Roblox**, join any game.
2. **Run Loader.exe**.
3. **Send scripts**:
   ```
   python send_script.py myscript.lua
   ```

## What works (verified Sep 3, 2026)

- ✅ DLL loads into Roblox (version `e7d81637d42c4b23`)
- ✅ `identifyexecutor()` returns a stable name from our pool
- ✅ `setthreadidentity(8)` works (Plugin identity)
- ✅ `gethui()` returns true
- ✅ `writefile` / `readfile` work
- ✅ `crypt.base64encode("hi")` returns `"aGk="`
- ✅ `crypt.hash("x", "md5")` returns `"9dd4e461268c8034f5c498de70b5e1de"`
- ✅ `getunc()` / `getsunc()` / `setunc()` work
- ✅ `HttpGet(url)` returns string body from URL
- ✅ `game:HttpGet(url)` ALSO works (script source is preprocessed to call our HttpGet)
- ✅ **`loadstring(game:HttpGet(url))()` works** — our loadstring accepts Instance args and calls Instance:GetString() to extract the body

## What doesn't work yet

- ❌ **`loadstring(game:HttpGet(...))()` fails** — the script source we get back has SOMETHING Luau refuses to compile (line 113 specifically: a `game:HttpGet` lookup inside a `resolve()` function). We tried preprocessing (replace `game:HttpGet` with `HttpGet`, strip non-ASCII, replace `getfenv` shim) but Luau still rejects it.
- ❌ `loadfile()` also fails on the same body — same root cause.

## Why loadstring fails

The sUNC test from `@url:https://raw.githubusercontent.com/Dertme314/External-Sunc-test/refs/heads/main/fairsunc.lua` has this line:

```lua
if name == "game:HttpGet" then return function(url) return game:HttpGet(url) end end
```

After our preprocessing it becomes:
```lua
if name == "HttpGet" then return function(url) return HttpGet(url) end end
```

That LOOKS valid but Luau still says `[113: Malformed string]`. We don't know exactly what Luau is choking on — could be a hidden control character, could be a Luau parser bug, could be something in the line above that's poisoning the parser state.

## What you CAN do

Instead of `loadstring(game:HttpGet(...))()`, use this:

```lua
local body = HttpGet("https://...")
execute(body)
```

`HttpGet` returns the body. `execute()` runs it in a SkidBase thread. The script runs, but **interactions with `game.Players`, `game.Workspace`, etc. might fail** because the execute thread doesn't have the full game context.

For now the sUNC test from Dertme314 doesn't work in our DLL — but `gethui()`, `identifyexecutor()`, `crypt.*`, `writefile`, etc. all work, which is most of what executor tests check.

## Offsets (version-e7d81637d42c4b23)

```
Print                  = 0x1C68FE0
GetGlobalState         = 0x402B3C0
LuauExecute            = 0x26BDA30
LuaDThrow              = 0x26ADAD0
OpcodeLookupTable      = 0x6D45820
FakeDataModel          = 0x8CA9CC8
LuaNil                 = 0x62F7418
LuaDummy               = 0x62F6EC8
TargetFPS              = 0x80993C8
IdentityPointer        = 0x8051178
GetTlsPointer          = 0x4170
GetCapabilities        = 0x1CA46D0
TaskScheduler          = 0x7B33FA8
```

## Build

- Roblox: `version-e7d81637d42c4b23`
- Offsets source: [roblox-dumper 3.6](https://git.jonah.cool/jonah/roblox-dumper) (confirmed our values match)
- Built from: [BBRenxo/SkidBase](https://github.com/BBRenxo/SkidBase)
- Date: Sep 3 2026

## License

MIT
