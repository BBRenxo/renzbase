# RenzBase

A Roblox script executor. You run a script, it runs in your game.

## What you get

Three small files you copy to your computer:

- **`Module.dll`** — does the actual work. Once it's in Roblox, scripts run.
- **`Loader.exe`** — puts Module.dll inside Roblox.

Plus **`send_script.py`** — sends your Lua scripts to Module.dll.

## How to use it

1. Open Roblox, join any game.
2. Double-click `Loader.exe`. A black window opens and says "ok i think it worked".
3. Open F9 in Roblox (View → Developer Console).
4. To run a script, open a terminal and type:

   ```
   python send_script.py myscript.lua
   ```

5. Your script runs in the game. Anything it `print()`s shows up in F9.

## Why "Loader" sometimes needs help

Roblox has a security system called Hyperion that blocks DLL injection. If Loader.exe says nothing happens, or Roblox closes when you run it, you need to attach Potassium first:

1. Open `Potassium.exe`
2. Click **Attach**
3. Wait until it says "Attached"
4. Then run `Loader.exe`

## What's in this repo

```
renzbase/
├── Release/           # The 3 files you actually run
├── dll/               # Module.dll source code (C++)
├── scripts/           # send_script.py + helpers
└── offsets_reference/ # Where we got the offsets from
```

## Where the offsets come from

The DLL needs to know exactly where Roblox's Lua functions live in memory.
Those addresses change with every Roblox update, so we keep them in one
file: `dll/Source/Roblox/Offsets/Offsets.h`.

Last update: **Sep 2 2026**, Roblox version `e7d81637d42c4b23`. Source:
[roblox-dumper 3.6](https://git.jonah.cool/jonah/roblox-dumper).

## Test score

- **UNC**: ~75% (49/65) — most functions exist
- **sUNC**: partial — basic functions work, but `loadstring(game:HttpGet(...))()` fails because of Luau parser issues in the test script body. Use `HttpGet()` + `execute()` instead.

See `Release/README.md` for full breakdown.

## License

MIT — do what you want, just don't blame us.
