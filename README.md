# RenzBase

A small Roblox script runner. You write Lua scripts, you run them in a game.

Built from a fork of [SkidBase](https://github.com/RavageDevs/SkidBase) with extra
stuff added and the bits that leak stripped out.

## What you get

Three files go in any folder:

| File | What it does |
|------|--------------|
| `Module.dll` | Does the work. Gets put inside Roblox. |
| `Loader.exe` | Puts `Module.dll` inside Roblox. |
| `send_script.py` | Sends your Lua scripts to `Module.dll`. |

You can grab them from the **Releases** page (zip), or grab them straight from
the `Release/` folder.

## How to use it

1. Open Roblox, join any game.
2. Double-click `Loader.exe`. A black window pops up and says something like
   "ok found module" / "ok i think it worked".
3. In Roblox, hit **F9** to open the developer console. You'll see your prints.
4. To run a script, open a terminal and type:
   ```
   python send_script.py path/to/your/script.lua
   ```
   or pipe one in:
   ```
   echo 'print("hi from RenzBase")' | python send_script.py
   ```

## If nothing happens when you run Loader.exe

Roblox has a security thing called Hyperion that sometimes blocks DLLs.
When that happens, Roblox closes itself the moment Loader.exe tries to
inject.

If that happens to you, you need another tool called **Potassium** to attach
first:

1. Open Potassium
2. Click **Attach**
3. Wait until it says "Attached"
4. Then run `Loader.exe` again

If even that fails, get the latest Hyperion-bypassing tool from the
Roblox modding community and use it first.

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
