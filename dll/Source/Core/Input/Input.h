#pragma once
#include <Windows.h>
#include <string>
#include <unordered_map>

namespace input {
    // Send a virtual key event. vk = Virtual-Key code (e.g. VK_SPACE = 0x20).
    // down=true means key down, false means key up.
    inline void send_key(BYTE vk, bool down) {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    // Map common key names to virtual key codes.
    inline BYTE vk_from_name(const std::string& name) {
        // Single characters
        if (name.size() == 1) {
            char c = name[0];
            if (c >= 'a' && c <= 'z') return (BYTE)toupper(c);
            if (c >= 'A' && c <= 'Z') return (BYTE)c;
            if (c >= '0' && c <= '9') return (BYTE)c;
            if (c == ' ') return VK_SPACE;
            if (c == '.') return VK_OEM_PERIOD;
            if (c == ',') return VK_OEM_COMMA;
            if (c == '/') return VK_OEM_2;
            if (c == '\\') return VK_OEM_5;
            if (c == '-') return VK_OEM_MINUS;
            if (c == '=') return VK_OEM_PLUS;
            if (c == '[') return VK_OEM_4;
            if (c == ']') return VK_OEM_6;
            if (c == ';') return VK_OEM_1;
            if (c == '\'') return VK_OEM_7;
        }
        // Named keys (case-insensitive)
        std::string n = name;
        for (auto& c : n) c = (char)tolower(c);
        if (n == "space") return VK_SPACE;
        if (n == "tab") return VK_TAB;
        if (n == "enter" || n == "return") return VK_RETURN;
        if (n == "escape" || n == "esc") return VK_ESCAPE;
        if (n == "backspace" || n == "back") return VK_BACK;
        if (n == "delete" || n == "del") return VK_DELETE;
        if (n == "left") return VK_LEFT;
        if (n == "right") return VK_RIGHT;
        if (n == "up") return VK_UP;
        if (n == "down") return VK_DOWN;
        if (n == "shift") return VK_SHIFT;
        if (n == "ctrl" || n == "control") return VK_CONTROL;
        if (n == "alt") return VK_MENU;
        if (n == "caps" || n == "capslock") return VK_CAPITAL;
        if (n == "f1") return VK_F1;
        if (n == "f2") return VK_F2;
        if (n == "f3") return VK_F3;
        if (n == "f4") return VK_F4;
        if (n == "f5") return VK_F5;
        if (n == "f6") return VK_F6;
        if (n == "f7") return VK_F7;
        if (n == "f8") return VK_F8;
        if (n == "f9") return VK_F9;
        if (n == "f10") return VK_F10;
        if (n == "f11") return VK_F11;
        if (n == "f12") return VK_F12;
        return 0;
    }

    // Send mouse move event.
    inline void mouse_move(int dx, int dy, bool absolute) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        if (absolute) {
            input.mi.dx = dx;
            input.mi.dy = dy;
            input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        } else {
            input.mi.dx = dx;
            input.mi.dy = dy;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
        }
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void mouse_button(int button, bool down) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        DWORD flag = 0;
        if (button == 1) flag = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        else if (button == 2) flag = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        else if (button == 3) flag = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
        input.mi.dwFlags = flag;
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void mouse_scroll(int delta) {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.mouseData = delta * 120;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        SendInput(1, &input, sizeof(INPUT));
    }

    // Check if Roblox window is focused.
    inline bool is_roblox_active() {
        HWND fg = GetForegroundWindow();
        if (!fg) return false;
        char title[256] = { 0 };
        GetWindowTextA(fg, title, sizeof(title));
        std::string t(title);
        return t.find("Roblox") != std::string::npos;
    }
}
