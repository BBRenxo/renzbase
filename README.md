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

Roblox has a security check (internally called Hyperion or Byfron) that
blocks DLL injection. RenzBase does NOT bypass it on its own — you'll
need a separate tool.

Popular options:
- **Potassium** — a UI tool from the Roblox modding community. Attach
  first, then run Loader.exe.
- Other Hyperion bypass tools — search the Roblox modding community
  for the latest.

**Honestly, we don't fully know how these tools work** — we just know
that if you don't use one, Loader.exe either fails silently or crashes
Roblox.

If you find a bypass tool that works, run it FIRST, leave it running,
THEN double-click Loader.exe.

## What's this "client modification bypass" people ask about?

When someone asks "does it have client modification bypass?" they
usually mean one of these:

| What they mean | What we have |
|----------------|--------------|
| Bypass Roblox's Hyperion/Byfron integrity checks | ❌ No — that's Potassium or another bypass tool's job |
| Bypass Roblox's anti-cheat (server-side usually) | ❌ N/A — server-side, not our problem |
| Hook and modify memory in the running game | ✅ Yes — Module.dll does this AFTER bypass |
| Run custom Lua scripts in a running game | ✅ Yes — that's the whole point |

So: we depend on a Hyperion bypass tool (Potassium or similar), and we
do the in-memory Lua hooking on top of that.

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
