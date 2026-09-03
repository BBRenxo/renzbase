# UNC and sUNC — What They Actually Are

## TL;DR

- **UNC** = Unified Naming Convention. Just checks if function NAMES exist.
  Easy to fake: `getgenv().writefile = function() end` → 100% UNC.
- **sUNC** = **senS'** Unified Naming Convention. Tests if functions actually
  WORK (round-trip file IO, real HTTP, real drawing, real closures, etc).
  This is what matters.
- **Fair Dunc Lab** = Dertme's external-friendly sUNC variant. Same tests but
  with extra anti-fake-detection (e.g. timing checks for Lua polyfills).

## The Actual Functions Tested

Source: Dertme's `fairsunc.lua` (54KB, 91 distinct tests).
Official sUNC at https://script.sunc.su/.

### Environment (6)
- `getgenv` — must return table, identity must persist
- `getrenv` — must return table containing `game`
- `gethui` — must return Instance
- `getcustomasset` — must return `rbxasset://...` URI (needs writefile)
- `getrunningscripts` — must return array of LuaSourceContainer instances; **10 calls < 1s** (catches polyfills)
- (testRaw `checkcaller context check` — must return false inside BindableEvent)

### Closures (8)
- `checkcaller` — must return true in executor, false in callback
- `iscclosure` / `islclosure`
- `newcclosure` — must wrap as C closure + survive 50 calls + multi-arg with nil
- `clonefunction` — must produce different reference, same behavior
- `loadstring` — must compile + execute

### FileSystem (9)
- `writefile` + `readfile` — round trip
- `appendfile` — must append to existing
- `loadfile` — must return function that runs the file
- `delfile`, `makefolder`, `listfiles`, `isfolder`, `isfile`, `delfolder`

### Network (5)
- `request` (or `http.request`, `syn.request`, `fluxus.request`)
- `game:HttpGet` — must actually fetch (live URL test)
- `HttpGet`, `HttpPost`, `http_request`, `http_request_async`

### Input (12)
- `keypress`, `keyrelease`, `keyhold` — actual key events via SendInput
- `mouse1click`, `mouse1press`, `mouse1release`
- `mouse2click`, `mouse2press`, `mouse2release`
- `mousemoveabs`, `mousemoverel`, `mousescroll`

### Crypt (9)
- `crypt.base64encode`, `crypt.base64decode` — round trip
- `crypt.encrypt`, `crypt.decrypt` — actual AES
- `crypt.generatekey`, `crypt.generatebytes`
- `crypt.hash` (or `md5`/`sha256`)
- `lz4compress`, `lz4decompress`

### Drawing (Drawing.new)
- Must produce objects with `.Visible`, `.Position`, `.Color`, `.Transparency`,
  `.Remove()`. Speed checks catch Lua overlays.
- `Drawing.Fonts` — must be a table
- `cleardrawcache`

### Debug (15)
- `debug.getconstant`, `debug.getconstants`
- `debug.getupvalue`, `debug.setupvalue` (round trip)
- `debug.getproto`, `debug.getprotos`
- `debug.getstack`, `debug.setstack`
- `debug.getinfo`
- `debug.setconstant` (must actually mutate)

### Console (4)
- `rconsoleprint`, `rconsoleclear`, `rconsolecreate`, `rconsoleclose`
- `rconsolename`, `rconsolewarn`, `rconsoleerror`, `rconsoleinfo`

### Metatable (5)
- `getrawmetatable`, `setreadonly`, `isreadonly`
- `getnamecallmethod`, `hookmetamethod`

### Thread
- `getthreadidentity`, `setthreadidentity` (must actually let you touch CoreGui)
- `identifyexecutor`

### Misc
- `setclipboard`, `isrbxactive`, `getfpscap`, `setfpscap`
- `compareinstances`, `isexecutorclosure`, `checkclosure`
- `queue_on_teleport`, `getconnections`
- `fireclickdetector`, `fireproximityprompt`, `firetouchinterest`
- `saveinstance`

### LabInteraction
- Tests that the wrapper can talk to the sUNC test game's remote events.

## Implementation Strategy for SkidBase

We'll add each function to the appropriate lib file. Priority order:

1. **P0 — Critical (must exist)** — getgenv, getrenv, identify, loadstring,
   checkcaller, newcclosure, clonefunction, iscclosure, islclosure
2. **P1 — FileSystem** — writefile, readfile, appendfile, delfile, makefolder,
   listfiles, isfolder, isfile, delfolder, loadfile
3. **P1 — Network** — request, game:HttpGet, HttpPost
4. **P1 — Input** — mouse + keyboard via SendInput
5. **P2 — Drawing** — native Drawing class (not Lua polyfill)
6. **P2 — Crypt** — AES, base64, lz4, hashing (real impls, not stubs)
7. **P2 — Debug** — getconstant/getupvalue/getproto on real Luau bytecode
8. **P3 — Console** — rconsole via allocated console window
9. **P3 — Misc** — getfpscap, setfpscap, setclipboard, isrbxactive

Each function: real implementation, not a stub. Round-trip tests must pass.
