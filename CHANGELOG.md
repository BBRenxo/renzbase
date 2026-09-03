# Changelog

All notable changes to RenzBase are documented here.

## [Unreleased] — 2026-09-02

### Added
- **Auto offset updater** — `scripts/update_offsets.py` fetches latest offsets from 3 working mirrors (imtheo.lol, femboythighs.org, uwuhook.club), falls through automatically
- **Multi-mirror support** — main+dedicated+domain-only mirrors with health tracking
- **FakeDataModel.Pointer offset fix** — `0x8CA9CC8` → `0x8D22868` (version-e7d81637d42c4b23)

### Improved
- **sUNC score: 75% (49/65)** — tested with Fair Dunc Lab v4.6 across all 13 categories
- **Input lib: 100%** — mouse1click/keypress/mousemove/mousescroll all native
- **Console lib: 100%** — rconsolecreate/rconsoleprint/rconsolename/rconsoleclear all working
- **Metatable lib: 100%** — setreadonly/isreadonly/getrawmetatable all working
- **Thread lib: 100%** — getthreadidentity/setthreadidentity all working
- **Crypt lib: 80%** — base64/hash/encrypt/decrypt/generatekey/generatebytes working, LZ4 partial
- **FileSystem lib: 90%** — writefile/readfile/appendfile/loadfile/isfile/isfolder/makefolder/delfile/delfolder/listfiles all working
- **Closures lib: 71%** — checkcaller/iscclosure/islclosure/newcclosure/clonefunction/loadstring all working
- **Debug lib: real impl** — uses Luau internal Proto/Closure/LuaTable structs, walks k[]/upvalues/p[] arrays

### Fixed
- **Unc::apply crash** — added validation + default Off mode (was writing to guessed offsets)
- **Module.dll loadstring(HttpGet) auto-exec crash** — removed (was segfaulting)
- **send_script.py PySECURITY_ATTRIBUTES bug** — fixed by removing security attrs
- **PIPE_TYPE_MESSAGE → PIPE_TYPE_BYTE** — clients no longer need message mode
- **Caps detection trigger** — was 0xFFFFFFFFFFFFFFFF (sus), now identity-8 only
- **identifyexecutor shadowing** — Closures::identifyexecutor was overwriting Misc registration
- **Drawing.h lambda→static + luaL_setmetatable→lua_setmetatable** — Luau compat fixes
- **Debug.h macro collisions** — Luau has `#define getproto(cl)` and `#define setupvalue(L,obj,x)` macros that caused C2059 cascade, renamed to dbg_*

## [0.1.0] — 2026-09-01

### Added
- Initial RenzBase skeleton
- Forked RavageDevs/SkidBase → BBRenxo/SkidBase
- Created BBRenxo/renzbase
- Volx-derived injector (no .byfron patching)
- Tauri UI skeleton (Monaco editor + Attach button)

## [Pre-0.1] — Earlier work

- SkidBase fork with P0/P1 libraries
- Hyperion emulator RE (paused)
- Hyperion.dll rebuild with verified offsets (1.6MB)
- Brochacha20 implementation (matches RFC 8439)
- pibbly_trimmed.bin analysis (Hyperion encrypted binary)
- Hyperion key array candidate at VA 0x6141800 (pibbly)
