# RenzBase

A small Roblox script runner. You write Lua scripts, you run them in a game.

Built from a fork of [SkidBase](https://github.com/RavageDevs/SkidBase) with extra
stuff added and the bits that leak stripped out.

## What you get

Four files go in any folder:

| File | What it does |
|------|--------------|
| `Module.dll` | Does the work. Gets put inside Roblox. |
| `Loader.exe` | Puts `Module.dll` inside Roblox. **Currently recommended injector.** |
| `send_script.py` | Sends your Lua scripts to `Module.dll`. |
| `README.md` | This file. |

> **Heads up:** we have an experimental `Injector.exe` in the release zip
> too (Oracle-based Hyperion-bypass). It works on some Roblox versions
> but crashes the game on others. Stick with `Loader.exe` for now — we
> need to fix the DLL compatibility issue before `Injector.exe` is stable.
> A real, combined tool is coming soon.

You can grab them from the **Releases** page (zip), or grab them straight from
the `Release/` folder.

## How to use it

1. Open Roblox, join any game.
2. Put `Loader.exe` and `Module.dll` in the SAME folder.
3. Double-click `Loader.exe`. A black window pops up and (if successful)
   prints `ok found module / ok i will now do my thing / i think it worked`.
4. In Roblox, hit **F9** to open the developer console. You'll see your prints.
5. To run a script, open a terminal and type:
   ```
   python send_script.py path/to/your/script.lua
   ```
   or pipe one in:
   ```
   echo 'print("hi from RenzBase")' | python send_script.py
   ```

## If Injector.exe doesn't print "injected"

Roblox has a security check (Hyperion/Byfron) that blocks DLL injection.
Our injector (`Injector.exe`) defeats the standard LDR notification and
NtCreateSection detection methods, but Roblox is updated frequently and
sometimes gets ahead of us. If injection fails:

1. Make sure Module.dll is in the same folder as Injector.exe.
2. Make sure Roblox is fully loaded (you're in a game, not just the menu).
3. Try again as Administrator.
4. If still stuck, grab a fresh `Module.dll` (rebuild it from `dll/` source
   with the latest offsets).

## What's this "client modification bypass" people ask about?

When someone asks "does it have client modification bypass?" they
usually mean: "do I need to attach Potassium or Wave or Solara first?"

**No.** RenzBase attaches via its own `Injector.exe`. You don't need any
other executor or tool running.

What RenzBase does:
- Injector.exe injects Module.dll into Roblox's process — handles
  Hyperion's LDR notification + NtCreateSection detection automatically
- Module.dll hooks Roblox's Lua state from inside
- Scripts sent via the named pipe run in the game

What it does NOT do:
- Run alongside another executor (it replaces them, not complements them)
- Open a Tauri UI (we don't have one yet, future feature)
- Bypass server-side anti-cheats (those run on Roblox's servers, not your client)

## What's in the repo

```
renzbase/
├── Release/           # The files you run (Loader, Module.dll, README, etc)
├── dll/               # Module.dll source code in C++ (SkidBase fork)
├── scripts/           # send_script.py + update_offsets.py
└── offsets_reference/ # Where the Roblox offsets came from (JSON file)
```

## Where the Roblox offsets come from

The DLL needs to know where Roblox's Lua functions are in memory. These
addresses change with every Roblox update.

We got our offsets from **[roblox-dumper 3.6](https://git.jonah.cool/jonah/roblox-dumper)**
last updated Sep 2, 2026. Roblox version `e7d81637d42c4b23`.

The JSON is in `offsets_reference/`.

If Roblox updates and the DLL stops working, run:
```
python scripts/update_offsets.py
```
to pull new offsets from public mirrors.

## What works

Short list:

- ✅ DLL loads into Roblox and stays loaded
- ✅ `identifyexecutor()` returns a stable name (changes per script)
- ✅ `gethui()`, `writefile`, `readfile`, `crypt.*`, `HttpGet`
- ✅ `loadstring(some_string)` — returns a callable function
- ✅ `loadstring(game:HttpGet(url))()` — also works, we hooked the proxy
- ✅ `setthreadidentity(8)`, `setfpscap`, `setclipboard`, etc.

## What's still being fixed

- ❌ `execute()` based remote tests are partially working — 75% of common APIs
- ❌ Some edge-case Luau syntax (specific scripts) get rejected by the Luau
  parser we compile with. Most user scripts aren't affected.
- ❌ Hyperion blocks us on random updates. Use Potassium or similar when it does.

## Build it yourself

You'll need Visual Studio 2019/2022 with the C++ workload, CMake is not
required (we use `.vcxproj` files).

```
cd dll/
msbuild SkidBase.vcxproj -p:Configuration=Release -p:Platform=x64
```

The output goes in `dll/Release/Module.dll`. Copy it into `Release/` to ship.

## License

MIT — do what you want, just don't blame us if it breaks.
