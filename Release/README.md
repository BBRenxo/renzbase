# RenzBase Release

Three files. That's all.

| File | Purpose |
|------|---------|
| `Module.dll` | The Lua runtime. Gets injected into Roblox. |
| `Loader.exe` | Puts Module.dll into Roblox. |
| `send_script.py` | Sends Lua scripts to Module.dll over a named pipe. |

## How to use

1. **Start Roblox**, join any game.
2. **Run Loader.exe**. That's it — no external tools needed.
3. **Send scripts**:
   ```
   python send_script.py myscript.lua
   ```

### "Does it have client modification bypass?"

Short answer: **yes**. RenzBase injects itself via its own Loader.exe. No
need to attach Potassium, Wave, Solara, or any other executor first.

What RenzBase does:
- Loader.exe does the injection (no external bypass tool needed)
- Module.dll hooks Roblox's Lua state from inside the game process
- 75%+ sUNC test coverage on basic executor APIs

What it doesn't do:
- Run alongside another executor (use RenzBase by itself)
- Open a Tauri/SkidUI yet (future feature, currently scripts sent via pipe only)

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
  or two of source that Luau's parser refuses. We tried preprocessing
  (`game:HttpGet` → `HttpGet`, strip non-ASCII, etc.) and the parser still
  rejects. Most user scripts aren't affected.
- ❌ `loadfile()` falls back to `loadstring` internally so it has the same
  edge cases.

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
exactly what Luau is choking on — could be a hidden control character,
could be a Luau parser bug, could be something in the line above that's
poisoning the parser state.

## What you CAN do for now

Use `HttpGet()` directly — it always returns a string:

```lua
local body = HttpGet("https://...")
loadstring(body)()
```

This pattern works 100% of the time. `game:HttpGet()` also works now, but
the `HttpGet()` global is more reliable.

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
