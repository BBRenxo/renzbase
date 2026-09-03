#pragma once
#include "lua.h"
#include "lualib.h"
#include <Windows.h>

// Stub for rconsole — full implementation needs a console window.
// We allocate a hidden console on first call so functions work but
// don't show unless rconsolecreate is explicitly called.

namespace console {
    static bool g_console_allocated = false;
    static bool g_visible = false;

    inline void ensure_console() {
        if (!g_console_allocated) {
            AllocConsole();
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
            freopen_s(&fp, "CONIN$", "r", stdin);
            g_console_allocated = true;
        }
    }

    inline int rconsolecreate(lua_State* L) {
        ensure_console();
        HWND hwnd = GetConsoleWindow();
        if (hwnd) { ShowWindow(hwnd, SW_SHOW); g_visible = true; }
        return 0;
    }

    inline int rconsolename(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        ensure_console();
        HWND hwnd = GetConsoleWindow();
        if (hwnd) SetWindowTextA(hwnd, name);
        return 0;
    }

    inline int rconsoleprint(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        ensure_console();
        printf("%s", msg);
        return 0;
    }

    inline int rconsolewarn(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        ensure_console();
        printf("[WARN] %s", msg);
        return 0;
    }

    inline int rconsoleerror(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        ensure_console();
        printf("[ERROR] %s", msg);
        return 0;
    }

    inline int rconsoleinfo(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        ensure_console();
        printf("[INFO] %s", msg);
        return 0;
    }

    inline int rconsoleclear(lua_State* L) {
        ensure_console();
        // Clear console by filling with whitespace
        COORD coord = {0, 0};
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (GetConsoleScreenBufferInfo(h, &csbi)) {
            DWORD written;
            FillConsoleOutputCharacter(h, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &written);
            SetConsoleCursorPosition(h, coord);
        }
        return 0;
    }

    inline int rconsoleclose(lua_State* L) {
        HWND hwnd = GetConsoleWindow();
        if (hwnd) { ShowWindow(hwnd, SW_HIDE); g_visible = false; }
        return 0;
    }
}
