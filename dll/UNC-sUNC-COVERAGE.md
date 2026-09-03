# RenzBase — UNC/sUNC Coverage Tracker

Tracks each test from the sUNC test suite (`fairsunc.lua`).
Format: `[ ]` = not started, `[~]` = in progress, `[x]` = passing, `[!]` = failing.

Last updated: see git log.

## Environment (5 tests)

- [x] getgenv — returns game env
- [x] gethui — returns PlayerGui (FIXED)
- [x] identifyexecutor — returns random name from pool
- [x] setthreadidentity / setidentity — sets identity
- [ ] getscriptclosure — needs Luau bytecode parser (P3)
- [ ] getcallingscript — needs call stack introspection (P3)

## Closures (6 tests)

- [x] hookfunction — hooks + returns original
- [x] hookmetamethod — hooks + returns original
- [x] newcclosure — returns new closure
- [x] iscclosure — checks if c closure
- [x] checkcaller — checks if called from executor
- [ ] hooksec — hooks with security level (P3)

## FileSystem (12 tests)

- [x] writefile — sandboxed in %TEMP%\RenzBase_<session>\
- [x] readfile
- [x] appendfile
- [x] loadfile
- [x] delfile
- [x] makefolder
- [x] listfiles
- [x] isfolder
- [x] isfile
- [x] delfolder
- [x] getcustomasset — returns asset path
- [x] readfileex — extended read with offset/length

## Network (8 tests)

- [x] HttpGet — via WinHTTP
- [x] HttpGetAsync
- [x] HttpPost — via WinHTTP
- [x] HttpPut — via WinHTTP
- [x] HttpDelete — via WinHTTP
- [x] HttpRequest — generic
- [x] request — modern {Url, Method, Body, Headers}
- [x] JSONDecode / JSONEncode — wrapper around nlohmann/json

## Input (12 tests)

- [x] keypress
- [x] keyrelease
- [x] keyhold
- [x] mouse1click / mouse1press / mouse1release
- [x] mouse2click / mouse2press / mouse2release
- [x] mousemoveabs / mousemoverel
- [x] mousescroll
- [x] isrbxactive / isgameactive
- [ ] movehook — needs Roblox viewport polling (P3)

## LabInteraction (4 tests)

- [ ] firetouchinterest — needs instance write + event fire (P2)
- [ ] fireproximityprompt — needs ProximityPrompt handler (P2)
- [ ] fireclickdetector — needs ClickDetector handler (P2)
- [ ] firetouchtransmitter — needs TouchTransmitter handler (P2)

## Crypt (8 tests)

- [x] base64encode / base64_encode
- [x] base64decode / base64_decode
- [x] lz4compress / lz4decompress
- [x] hash — MD5/SHA1/SHA256/SHA384/SHA512
- [x] generatebytes
- [x] generatekey
- [ ] crypt.encrypt / crypt.decrypt — needs AES (P3)
- [x] getfunctionhash

## Drawing (8 tests)

- [~] Drawing.new — stub returns table with metatable
- [~] Line — stub
- [~] Text — stub
- [~] Image — stub
- [~] Circle — stub
- [~] Square — stub
- [~] Triangle — stub
- [~] Quad — stub
- [~] UI / World / Screen — stub
- [~] cleardrawcache — stub
- [ ] **Real renderer** — needs viewmatrix hooks (P4) — for visible drawing

## Console (8 tests)

- [x] rconsolecreate — AllocConsole
- [x] rconsoleclose
- [x] rconsoleprint
- [x] rconsolewarn
- [x] rconsoleerror
- [x] rconsoleinfo
- [x] rconsoleclear
- [x] rconsolename

## Metatable (6 tests)

- [x] getrawmetatable
- [x] setreadonly
- [x] isreadonly
- [x] getnamecallmethod
- [x] hookmetamethod (already in Closures)
- [ ] makereadonly / makewritable — same as setreadonly

## Thread (6 tests)

- [x] setthreadidentity (already in Environment)
- [x] getthreadidentity
- [x] delay
- [x] spawn
- [x] defer
- [x] task — wraps Roblox's task

## Misc (8 tests)

- [x] identifyexecutor (already in Environment)
- [x] gethui (already in Environment)
- [x] setunc — yield counter control
- [x] getsunc — read unc mode
- [x] execute — queue script
- [x] queuelua / queue_script / skid_execute — queue variants
- [ ] getrawmetatable (already in Metatable)
- [ ] getconnections — needs Roblox event introspection (P3)
- [ ] getrunningscripts — needs ScriptContext walk (P3)
- [ ] queueonteleport — TeleportHandler integration (P3)
- [ ] isrbxactive (already in Input)

## Debug (10 tests)

- [ ] debug.getconstant — needs Luau bytecode parser (P3)
- [ ] debug.getconstants
- [ ] debug.getupvalue
- [ ] debug.setupvalue
- [ ] debug.getproto
- [ ] debug.getprotos
- [ ] debug.getinfo
- [ ] debug.getstack
- [ ] debug.setstack
- [ ] debug.setconstant
- [ ] debug.getfenv
- [ ] debug.setfenv
- [ ] debug.traceback

## Summary

| Category       | Total | Done | In Progress | Pending |
|----------------|-------|------|-------------|---------|
| Environment    | 6     | 4    | 0           | 2       |
| Closures       | 6     | 5    | 0           | 1       |
| FileSystem     | 12    | 12   | 0           | 0       |
| Network        | 8     | 8    | 0           | 0       |
| Input          | 12    | 11   | 0           | 1       |
| LabInteraction | 4     | 0    | 0           | 4       |
| Crypt          | 8     | 7    | 0           | 1       |
| Drawing        | 11    | 0    | 11          | 0       |
| Console        | 8     | 8    | 0           | 0       |
| Metatable      | 6     | 5    | 0           | 1       |
| Thread         | 6     | 6    | 0           | 0       |
| Misc           | 11    | 7    | 0           | 4       |
| Debug          | 13    | 0    | 0           | 13      |
| **Total**      | **109** | **73** | **11**     | **27** |

**Current: 67% passing (73/109)**

## Pending Work By Priority

### P2 (do these next — quick wins)
- [ ] LabInteraction: firetouchinterest, fireproximityprompt, fireclickdetector
- [ ] Crypt: crypt.encrypt/decrypt (real AES)
- [ ] Misc: getconnections, getrunningscripts, queueonteleport

### P3 (harder — needs RE work)
- [ ] Debug.* — needs Luau bytecode parser
- [ ] Drawing renderer — needs viewmatrix hooks
- [ ] Environment: getscriptclosure, getcallingscript
- [ ] Closures: hooksec
- [ ] Misc: hooksec

### P4 (nice-to-have)
- [ ] Input: movehook
- [ ] Drawing real renderer (visible)
