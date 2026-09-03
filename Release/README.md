# RenzBase Release

Four files. That's all.

| File | Purpose |
|------|---------|
| `Module.dll` | The Lua runtime. Gets injected into Roblox. |
| `Loader.exe` | Puts Module.dll into Roblox — currently the recommended injector. |
| `send_script.py` | Sends Lua scripts to Module.dll over a named pipe. |
| `README.md` | This file. |

> **Heads up about Injector.exe:** there's also an experimental Oracle-based
> `Injector.exe` (Hyperion LDR + NtCreateSection bypass) in some zips.
> It crashes the game on current Roblox versions. Stick with `Loader.exe`
> for now — we need a real combined tool, which is coming soon.

## How to use

1. **Start Roblox**, join any game.
2. **Run Injector.exe**. It looks for `RobloxPlayerBeta.exe`, defeats
   Hyperion's DLL detection, and maps `Module.dll` into the game. When
   done it prints `injected`.
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
- ✅ **`game:HttpGet(url)` works** — `game` is now a proxy that routes `HttpGet` / `HttpGetAsync` / `HttpPost` to our C functions, and falls through to the real game for everything else
- ✅ **`loadstring(game:HttpGet(url))()` works** — `game:HttpGet` returns a string directly, so `loadstring` just compiles it
- ✅ `loadstring(Instance)` is also tolerant — tries `GetString()`, `Source`, `Body`, etc. before failing
- ✅ `loadstring` has a fallback parser path — if `Luau::compile` rejects a script, we try `luau_load` directly

## What doesn't work yet

- ❌ Some specific scripts (notably the sUNC Fair Dunc Lab test) have a line
  or two of source that Luau's parser refuses. Most user scripts aren't affected.

## Why some scripts still fail

The specific line that fails:

```lua
if name == "game:HttpGet" then return function(url) return game:HttpGet(url) end end
```

After our preprocessing it becomes:

```lua
if name == "HttpGet" then return function(url) return HttpGet(url) end end
```

That LOOKS valid but Luau says `[113: Malformed string]`. We don't know
exactly what Luau is choking on — could be a Luau parser bug, could be
something in the line above.

## What you CAN do for now

Use `HttpGet()` directly — it always returns a string:

```lua
local body = HttpGet("https://...")
loadstring(body)()
```

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
- Offsets source: [roblox-dumper 3.6](https://git.jonah.cool/jonah/roblox-dumper)
- Built from: [BBRenxo/SkidBase](https://github.com/BBRenxo/SkidBase)
- Injector: based on Oracle-style LDR + NtCreateSection bypass
- Date: Sep 3 2026

## License

MIT
