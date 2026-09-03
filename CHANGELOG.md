# Changelog

What's new, in plain English.

## Sep 3 2026 — offsets + game:HttpGet fix

- **Offsets are now correct for the current Roblox version.** Earlier we had
  the wrong values for `Print` and a few others; we re-checked against the
  fresh `roblox-dumper 3.6` dump and reverted to the original SkidBase
  values. DLL is back to working.
- **`game:HttpGet(url)` now actually returns the script text.** Before it
  returned a Roblox object instead of a string, so `loadstring(game:HttpGet(...))`
  errored. Fixed by setting `game.HttpGet` directly to our HTTP function.

## Sep 2 2026 — first real release

- All three pieces (DLL, Injector, UI skeleton) in one repo.
- DLL forks RavageDevs/SkidBase and adds UNC/sUNC coverage.
- Injector is a stripped-down volx-style mapper.
- UI is a Tauri app (Rust + web) with Monaco editor — not done yet.
- sUNC test: 75% in Fair Dunc Lab.

## Aug 30 2026 — repo created

- Forked SkidBase to `BBRenxo/SkidBase`.
- Created `BBRenxo/renzbase` for the project.
