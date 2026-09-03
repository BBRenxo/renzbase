# SkidBase Architecture — For My Confused Rat Friend

## What Each Thing Is

### Executor (the big thing)
A program that lets you **run your own Lua/Luau code inside Roblox
games**. Normally Roblox only runs code from the game server. An
executor bypasses that restriction by injecting code into the running
Roblox process and hooking the Luau VM so your scripts get executed.

In your case: **Hyperion.dll** is the executor.

### DLL
A Windows Dynamic-Link Library — compiled C++ code that loads into a
running process at runtime. SkidBase's output is a DLL because:

- DLLs get loaded INTO Roblox (via LoadLibrary)
- Once loaded, they share Roblox's memory + threads
- So they can read/write anything in Roblox
- And they can call Roblox's internal functions (Luau execute,
  instance reads, etc)

### Injector
The program that **loads the DLL into Roblox**. SkidBase comes with
`Release/Loader.exe` which is a basic injector. You rename your built
DLL to `module.dll` and the loader pushes it into RobloxPlayerBeta.

### Wrapper (vs Base vs Module — same thing)
A **wrapper** (also called **base** or **module**) is a DLL that wraps
an executor. It does two things:

1. **Exposes the executor's features** through Lua APIs (like
   `getgenv`, `loadstring`, `HttpGet`)
2. **Hides that an executor is being used** (anti-detection) by
   faking Roblox internals so anti-cheat can't tell a wrapper is loaded

**So: SkidBase is a wrapper/module for Hyperion. Hyperion is the
executor. SkidBase sits on top and gives you the nice Lua API.**

### API
The Lua functions your scripts call. `game:GetService()`,
`loadstring()`, `writefile()` — these all exist in real Roblox, but
the dangerous ones (loadstring, writefile, HttpGet) are restricted by
identity level. SkidBase **bypasses those restrictions** by injecting
its own C++ implementations of those functions into the Lua state.

## How SkidBase Works (current state)

```
RobloxPlayerBeta.exe (running game)
        │
        │ [Injector pushes our DLL in via LoadLibrary]
        ▼
SkidBase.dll (loaded into Roblox process)
        │
        │ 1. DllMain spawns main_thread
        ▼
TeleportHandler (loops every 300ms)
        │
        │ 2. Watches FakeDatamodel pointer for changes
        │ 3. When game loads, walks to ScriptContext
        │ 4. Calls GetGlobalState to get lua_State
        ▼
Register Lua APIs (in lua_State)
        │
        ├─ getgenv / getreg / getrenv
        ├─ loadstring
        ├─ HttpGet (winsock to localhost:9002)
        ├─ identifyexecutor → "SkidBase"
        ├─ setidentity (caps injection)
        └─ metatable hooks for game.__index / __namecall
        │
        ▼
User scripts (loaded via winsock)
        │
        │ 3. Each script:
        │    - compile source → Luau bytecode
        │    - encode bytecode (xor opcode table)
        │    - sandbox + set capabilities (0xFFFFFFFFFFFFFFFF = all)
        │    - pcall via task.defer
```

## What's Wrong + What We're Going to Fix

### Critical (broken / detectable):

1. **`caps = 0xFFFFFFFFFFFFFFFF`** ← **THIS IS THE BIG ONE**
   - Means "I have ALL capabilities" — every Roblox check sees this
   - Hyperion's anti-cheat flags this immediately
   - **Fix**: use real identity (6/7/8) and only enable specific caps
     that scripts need (writefile, gethiddenprop, firetouchinterest)

2. **identifyexecutor returns "SkidBase"** — instant detection
   - **Fix**: random per-session name OR user-configurable OR detect
     common AC scanners and lie to them

3. **HttpGet via raw winsock on localhost:9002** — visible to net
   monitor
   - **Fix**: use Roblox's own HttpService via identity escalation

4. **No proper error handling** — one script error crashes the thread
   - **Fix**: wrap each script execution in pcall + timeout

5. **gethui returns wrong thing** — gets CoreGui (PlayerGui) instead
   of the hidden UI insulators
   - **Fix**: properly clone game + lock + insert fake hidden stuff

### Missing libraries (every modern executor has these):

- `writefile / readfile / appendfile / loadfile / savefile`
- `listfiles / makefolder / isfolder / isfile / delfolder / delfile`
- `getconnections / getlistconnections / firesignal / firetouchinterest`
- `gethiddenprop / sethiddenprop / getproperties`
- `hookfunction / hooksec / newcclosure`
- `rconsoleprint / rconsolewarn / rconsoleerror / rconsoleinfo / rconsoleclear`
- `Drawing` (library — points, lines, circles, text, images)
- `WebSocket` connection
- `protect_instance / protect_gui / gethui`
- `queueonteleport / setrbxclipboard`
- `mousemoveabs / mousemoverel / mouse1click / mouse1down / mouse1up`
- `keypress / keyrelease / keyhold`
- `isrbxactive / iswindowactive`
- `getscriptclosure / getscripts / getrunningscripts`
- `decompile / dumpstring`
- `fireclickdetector / firetouchtransmitter`
- `getnilinstances / getnilconnections`
- `request / http_request / http_request_async`

### Missing infrastructure:

- **proper script timeout** (yielding forever detection)
- **multi-script execution** (current is queue-based, blocking)
- **script error reporting** to user (not just print)
- **execution context flags** (script type, source, etc)
- **uncapped FPS** (`unc`/`sunc` — set Roblox's FPS limit to 0/inf)
- **super-uncapped** (also disable VSync in render thread)
- **rendering** (Draw via Roblox's RenderJob queue)
- **proper hookchain** (chain to original __index / __namecall)
- **input simulation** (mouse/keyboard via SendInput)
- **anti-detect hooks** on common AC functions

## Build Plan

We're going to rewrite SkidBase in-place with these priority levels:

1. **P0 — Critical fixes** (caps, identify, HttpGet, error handling)
2. **P1 — Missing libs** (writefile, firetouchinterest, gethui proper, etc)
3. **P2 — Performance** (Drawing lib native, multi-script queue, timeouts)
4. **P3 — Anti-detection** (random executor name, fake closure paths)
5. **P4 — Input/render** (mouse, keyboard, Drawing)

Each step = a small commit. We'll test as we go.
