# Sep 4 2026 — Plan For Tomorrow

## Current State (Sep 3 EOD)

### What Works
- ✅ `Loader.exe` + `Module.dll` inject successfully (confirmed in Fair Dunc Lab)
- ✅ `identifyexecutor()`, `gethui()`, `setthreadidentity(8)`, `writefile`,
  `crypt.*`, `HttpGet()`, `getunc/getsunc/setunc` all work
- ✅ `game:HttpGet(url)` works via proxy `game` table
- ✅ `HttpGet(url)` returns string directly
- ✅ sUNC was at 75% earlier session (before proxy game changes)

### What's Broken
- ❌ **`loadstring(game:HttpGet(url))()` on the sUNC test script**
  - body returns 54257 bytes fine
  - fails to compile at line 113 with `Malformed string`
  - tried preprocessing, non-ASCII strip, `getfenv` shim, fallback parser
  - root cause unknown (could be Luau parser bug, hidden control char,
    or something else in the body)
- ❌ **`Injector.exe` (Oracle-based) crashes Roblox** with error
  "Roblox crashed because of incompatible software" pointing at
  Module.dll. Loader.exe works fine with the SAME DLL.
- ❌ Tauri UI not started yet
- ❌ Some scripts that use specific Roblox APIs (game:GetService, etc.)
  might fail because the proxy `game` __index fallback might have edge cases

## Tasks For Tomorrow (In Order)

### 0. Check BaseAddress offset claim (Sep 3 announcement)

Discord announcement: "Roblox has updated the Base address offset. To
have the real base address you must do:
    inline constexpr uintptr_t BaseAddress = 0x30;
    uintptr_t base = memory::read<uintptr_t>(memory::get_base() + Offsets::BaseAddress);"

Possibility: `GetModuleHandleA(nullptr)` returns the Roblox module's
load address, but Roblox might have done some kind of relocations
where the real code base is now at `[module+0x30]`. Or it could be
fake/test info.

**Don't apply yet**. Reason: our current `krah() = GetModuleHandleA`
works fine (we proved it earlier today). If we switch to the new
method and `0x30` doesn't actually contain the real base, EVERY
address becomes wrong and we crash.

Instead:
- Test the current DLL (after OpcodeLookupTable revert) and see if it
  works WITHOUT the BaseAddress change
- If it crashes, try this BaseAddress=0x30 trick as a fallback
- Verify by reading the value at `GetModuleHandleA + 0x30` and seeing
  if it's a plausible pointer

### 1. Fix `loadstring(game:HttpGet(...))()` (highest priority)

The user wants the sUNC test to actually run.

**Possible angles to try**:
- **Hex dump line 113 byte-by-byte** — find any weird character (BOM,
  \r alone, etc) that Luau rejects
- **Try Luau's internal `loadstring` instead of our custom one** — see
  if Luau's compiles this body via the game's existing state
- **Compile to bytecode with `Luau::compile`, then load that directly**,
  bypassing the string-pass-to-loadstring path entirely
- **Use `task.defer` and run the script as a string passed to a known
  Roblox script via `Instance.new("LocalScript")`**
- **Manual lexing of the body**: write a tiny C++ function that scans
  the body for the specific issue using Luau's Lexer.cpp

### 1.5. Wire up WebSocket wrapper (new, just added)

`Source/Roblox/Environment/Libs/WebSocket/WebSocket.h` has the full
WebSocket wrapper code (pushed to BBRenxo/SkidBase as dev code
in commit `2b5a77f`). It exposes:
- `WebSocket.connect("wss://...")` returns a ws object
- `:Send(msg)`, `:Close()`, `:OnMessage:Connect(fn)`, `:OnClose:Connect(fn)`

Backend uses Roblox's `WebSocketService` (bypasses Hyperion's network filter).

Tasks:
- Wire `websocket::connect` into `Register()` so `WebSocket` is a global
- Test that scripts can do `WebSocket.connect("wss://...") :Send("hi")`
- Once working, add a `WebSocketGet(url)` alias that does
  connect + send a GET-like request + collect response + return body string
- THEN we can replace `HttpGet` with `WebSocketGet` for full
  Hyperion-bypass

If the loadstring problem is actually a WinHTTP filter thing (Hyperion
blocking the byte stream from raw.githubusercontent.com), WebSocketGet
would bypass the issue entirely.

### 2. Figure out why Injector.exe crashes Roblox

- Run Injector.exe against a test exe (not Roblox) to confirm it works
  mechanically
- Check if Module.dll's DllMain has any code that fails under
  Oracle's injection context (e.g. sandbox startup, thread setup)
- Maybe the DLL needs a flag fix or a dependency that Loader.exe
  brings but Oracle doesn't

### 3. Build the Tauri UI

Currently `ui/` folder is empty (renamed Potassium). For RenzBase
we'd build something similar:
- Tauri Rust backend
- HTML/JS frontend with Monaco editor for scripts
- "Attach" button that runs Injector.exe (or Loader.exe)
- Output console showing F9 prints (read from log file or pipe)
- Script library: save/load scripts to disk

Estimated: 4-6 hours of work if we have the time.

### 4. Clean up the desktop `RenzBase/` folder

Currently 5 files: `Injector.exe Loader.exe Module.dll README.md send_script.py`.
Already pretty clean. Maybe rename/remove Injector.exe since it
crashes the game.

### 5. GitHub cleanup

- Make sure all READMEs reference Loader.exe consistently
- Tag a v0.2.1 saying "reverted to Loader.exe as default"
- Commit Oracle source under `injector/` (already done)
- Maybe add a `docs/HACKING.md` for anyone wanting to swap in their own
  injector

## Files To Touch Tomorrow

```
C:\Users\Administrator\Documents\GitHub\SkidBase\Source\
  ├── Core\Execution\Execution.cpp           ← try direct bytecode compile
  ├── Roblox\Environment\Environment.h       ← proxy game __index fallback
  └── Roblox\Environment\Libs\Closures\Closures.h ← loadstring debug

C:\Users\Administrator\Documents\GitHub\renzbase\
  ├── injector\Injector.cpp                  ← find why it crashes Roblox
  ├── ui\                                     ← start Tauri here
  └── Release\*.zip                           ← rebuild with fixes
```

## Decisions To Make With User

1. **Why does Injector.exe crash when Loader.exe doesn't?**
   - We don't know. Will debug tomorrow by diffing what each does.

2. **Should we just stick with Loader.exe and skip Injector.exe entirely?**
   - Loader.exe works NOW. Injector.exe is a future improvement.
   - User wants a "real combined tool" — meaning maybe Loader.exe +
     Oracle-style cleaning baked in. That's a bigger rewrite.

3. **Should we build the Tauri UI now or after loadstring works?**
   - Tauri UI is independent of loadstring. Can be done in parallel.
   - But loadstring fix is more important — UI is just chrome.

4. **Want me to investigate line 113 char-by-char, or try the
   manual-lex approach with Luau's Lexer.cpp?**
   - Manual lex is faster IMO. Dump the AST of the body and find what
     line 113 actually parses to.

## State At Time Of Save

- BBRenxo/SkidBase at commit `9d59897` (loadstring fallback path)
- BBRenxo/renzbase at commit `a877932` (Injector + docs but reverting)
- GitHub tag `v0.2.0` exists but will be superseded by `v0.2.1`
- Desktop has Loader.exe + Injector.exe + Module.dll + README +
  send_script.py
- Roblox version: `e7d81637d42c4b23`
- DLL offsets match roblox-dumper 3.6 (verified)
- User finished session happy with Loader.exe + clean Desktop
