# RenzBase Console Client (no UI needed)
#
# Use this to test scripts while the Tauri UI is being built on a more-powerful machine.
# When the Injector.exe has injected Module.dll into Roblox and you see
# "(your shitsploit name) has handled teleporting sir" in Roblox F9 console,
# run this script to send scripts to the DLL via \\.\pipe\RenzBase.
#
# Usage:
#   python send_script.py print.lua             # sends print.lua contents
#   python send_script.py "print('hi')"          # sends inline source
#   python send_script.py --autoexec            # runs all scripts in ./autoexec/
#   python send_script.py --loop                # interactive: read scripts to send

import os
import sys
import time
import win32file
import win32pipe
import win32con

PIPE = r"\\.\pipe\RenzBase"

def send(script: str) -> bool:
    """Send script over the named pipe to Module.dll in Roblox."""
    try:
        # Wait briefly for pipe to exist (DLL may still be initializing)
        for _ in range(50):
            try:
                handle = win32file.CreateFile(
                    PIPE,
                    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                    0, 0, win32file.OPEN_EXISTING, 0, None
                )
                break
            except win32pipe.error:
                time.sleep(0.1)
        else:
            print("[!] Pipe not available — is Injector.exe running and Module.dll injected?")
            return False

        try:
            win32pipe.SetNamedPipeHandleState(handle, win32pipe.PIPE_READMODE_MESSAGE, None, None)
            data = script.encode('utf-8')
            win32file.WriteFile(handle, data)
            print(f"[+] Sent {len(data)} bytes to {PIPE}")
            return True
        finally:
            handle.Close()
    except Exception as e:
        print(f"[!] Send failed: {e}")
        return False

def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print("  python send_script.py <script.lua>           # file")
        print('  python send_script.py "print(\'hi\')"          # inline')
        print("  python send_script.py --autoexec              # folder ./autoexec/")
        print("  python send_script.py --loop                  # interactive")
        sys.exit(1)

    if sys.argv[1] == "--autoexec":
        if not os.path.isdir("./autoexec"):
            os.makedirs("./autoexec")
            print(f"[+] Created ./autoexec/ — drop .lua files there, then run --autoexec again")
            sys.exit(0)
        for fn in sorted(os.listdir("./autoexec")):
            if fn.endswith(".lua") or fn.endswith(".luau"):
                path = os.path.join("./autoexec", fn)
                with open(path, 'r', encoding='utf-8') as f:
                    print(f"[>] Sending {path}")
                    send(f.read())
        sys.exit(0)

    if sys.argv[1] == "--loop":
        print("[*] Interactive mode. Type script source, empty line + Enter to send, /quit to exit")
        buf = []
        while True:
            try:
                line = input()
            except EOFError:
                break
            if line == "/quit":
                break
            buf.append(line)
            if not line and buf:
                # Empty line flushes buffer
                src = "\n".join(buf)
                send(src)
                buf = []
        sys.exit(0)

    arg = sys.argv[1]
    if os.path.isfile(arg):
        with open(arg, 'r', encoding='utf-8') as f:
            send(f.read())
    else:
        send(arg)

if __name__ == "__main__":
    main()
