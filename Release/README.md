# RenzBase Release

This is the actual program. Drop these 3 files in a folder. Run them.

## What's here

| File | What it does |
|------|--------------|
| `Module.dll` | The Lua runtime. Gets injected into Roblox. |
| `Loader.exe` | Puts Module.dll into Roblox. Run this first. |
| `send_script.py` | Sends Lua scripts to Module.dll over a named pipe. |

## Steps

1. **Start Roblox**, join any game.
2. **Run Loader.exe** (double-click).
   - If Roblox closes when you do this, Hyperion is blocking the load.
     Open Potassium, click Attach, then run Loader.exe again.
   - When it works, Loader.exe shows `ok i think it worked`.
3. **Send a script**:
   ```
   python send_script.py myscript.lua
   ```
   Or just text:
   ```
   echo 'print("hi from renzbase")' | python send_script.py
   ```
4. **Check F9 in Roblox** to see your script's output.

## Version

- Roblox: `version-e7d81637d42c4b23`
- Built: Sep 3 2026
- sUNC score: 75% in Fair Dunc Lab

## License

MIT
