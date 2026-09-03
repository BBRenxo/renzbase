# How RenzBase works

The short version: a DLL gets smuggled into Roblox, and once inside it
talks Lua back to you over a named pipe.

## The 3 pieces

### `Module.dll` (the brain)

This is the part that actually does stuff. It's a regular Windows DLL
written in C++ that, once loaded into Roblox, does three things:

1. **Finds Roblox's Lua state** in memory. Roblox has a global Lua
   interpreter; the DLL locates it by reading known offsets (like
   `TaskScheduler` → `ScriptContext` → `main_thread`).

2. **Registers Lua functions** into Roblox's environment. Stuff like
   `writefile`, `readfile`, `crypt.hash`, `firetouchinterest`,
   `identifyexecutor`, etc. Roblox doesn't ship these; the DLL adds them.

3. **Listens for scripts** on a named pipe (`\\.\pipe\RenzBase`). When a
   script arrives, the DLL runs it through `luaL_loadstring` then
   `luau_execute`, and the output shows up in F9.

### `Loader.exe` (the door)

Just opens Roblox's process and uses `CreateRemoteThread` +
`LoadLibraryA` to inject Module.dll. That's the textbook way to inject
a DLL, and it works *unless* Roblox's Hyperion security catches it.

### `Injector.exe` (the backup door)

Same idea as Loader.exe, but writes the DLL into memory manually instead
of using `LoadLibraryA`. Slightly stealthier, also blocked by Hyperion.

## The flow when you run a script

```
you                send_script.py              Loader.exe
 │                      │                          │
 │  python send_script  │                          │
 │ ───────────────────► │                          │
 │                      │  open named pipe         │
 │                      │ ─────────────────────►   │
 │                      │      write Lua source    │
 │                      │ ─────────────────────►   │
 │                      │                          │
                                            Module.dll (in Roblox)
                                                   │
                                       luaL_loadstring(src)
                                                   │
                                          luau_execute(L)
                                                   │
                                          output → F9
                                                   ▼
```

## Why the offsets matter

Roblox keeps changing its binary. Every update moves functions around in
memory. We have a hardcoded list of "where things are" addresses in
`dll/Source/Roblox/Offsets/Offsets.h`. When Roblox updates:

1. Someone dumps the new binary
2. They find where Print/LuauExecute/etc ended up
3. They update Offsets.h
4. We rebuild the DLL

Right now we use offsets from `roblox-dumper 3.6` for Roblox version
`e7d81637d42c4b23`. Last verified working Sep 2 2026.

## Hyperion

Roblox's anti-cheat. It sits between your DLL and the game's Lua state
and blocks anything that looks fishy. To get past it without a bypass:

- **Potassium** does the bypass externally (its own driver).
- We don't bundle a bypass because every public one gets detected
  within weeks.

If you want this DLL to load without Potassium, you need a Hyperion
emulator (like the user built earlier — `Hyperspace`) and you have to
plug it in here.
