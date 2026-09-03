# Changelog

What's new, in plain English.

## Sep 3 2026 — proxy game + tolerant loadstring

- **`game:HttpGet(url)` works.** We replaced the global `game` with a proxy
  table that routes `HttpGet`, `HttpGetAsync`, `HttpPost`, etc. to our C
  functions. Everything else (`GetService`, `Players`, `Workspace`) still
  works via the proxy's metatable fallback to the real game.
- **`loadstring(...)` is more tolerant.** It now accepts Instance args (the
  thing `game:HttpGet` returns) and tries `:GetString()`, `.Source`,
  `.Body` to extract the script body. Falls back to error if none work.
- **`loadstring(...)` has a fallback parser.** If `Luau::compile` rejects the
  source, we try `luau_load` directly. Some scripts that previously errored
  on line 113 type stuff now load.
- **`getfenv`/`setfenv` shim added** because Luau removed them but modern
  scripts still use them.
- **Non-ASCII bytes stripped** from scripts before compilation. UTF-8 emoji
  and similar were breaking Luau's parser.

## Sep 3 2026 — offsets + game:HttpGet fix

- **Offsets are now correct for the current Roblox version.** Earlier we had
  the wrong values for `Print` and a few others; we re-checked against the
  fresh `roblox-dumper 3.6` dump and reverted to the original SkidBase
  values. DLL is back to working.
- **`game:HttpGet(url)` now actually returns the script text.** Before it
  returned a Roblox object instead of a string, so `loadstring(game:HttpGet(...))`
  errored.

## Sep 2 2026 — first real release

- All three pieces (DLL, Loader, UI skeleton) in one repo.
- DLL forks RavageDevs/SkidBase and adds UNC/sUNC coverage.
- Loader is a stripped-down volx-style mapper.
- UI is a Tauri app (Rust + web) with Monaco editor — not done yet.
- sUNC test: 75% in Fair Dunc Lab.

## Aug 30 2026 — repo created

- Forked SkidBase to `BBRenxo/SkidBase`.
- Created `BBRenxo/renzbase` for the project.
