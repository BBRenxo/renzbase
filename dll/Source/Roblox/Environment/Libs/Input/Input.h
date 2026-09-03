#pragma once
#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "../../../../Core/Input/Input.h"

namespace inp {
    inline int keypress(lua_State* L) {
        const char* key = luaL_checkstring(L, 1);
        BYTE vk = input::vk_from_name(key);
        if (vk == 0) {
            lua_pushnil(L);
            lua_pushstring(L, "unknown key");
            return 2;
        }
        input::send_key(vk, true);
        input::send_key(vk, false);
        return 0;
    }

    inline int keyrelease(lua_State* L) {
        const char* key = luaL_checkstring(L, 1);
        BYTE vk = input::vk_from_name(key);
        if (vk == 0) return 0;
        input::send_key(vk, false);
        return 0;
    }

    inline int keyhold(lua_State* L) {
        const char* key = luaL_checkstring(L, 1);
        BYTE vk = input::vk_from_name(key);
        if (vk == 0) return 0;
        input::send_key(vk, true);
        // Note: caller must call keyrelease to release
        return 0;
    }

    inline int mouse1click(lua_State* L) {
        input::mouse_button(1, true);
        input::mouse_button(1, false);
        return 0;
    }
    inline int mouse1press(lua_State* L) {
        input::mouse_button(1, true);
        return 0;
    }
    inline int mouse1release(lua_State* L) {
        input::mouse_button(1, false);
        return 0;
    }

    inline int mouse2click(lua_State* L) {
        input::mouse_button(2, true);
        input::mouse_button(2, false);
        return 0;
    }
    inline int mouse2press(lua_State* L) {
        input::mouse_button(2, true);
        return 0;
    }
    inline int mouse2release(lua_State* L) {
        input::mouse_button(2, false);
        return 0;
    }

    inline int mousemoveabs(lua_State* L) {
        int x = (int)luaL_checkinteger(L, 1);
        int y = (int)luaL_checkinteger(L, 2);
        // Convert to absolute (0-65535 range)
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        int ax = (int)((double)x / sw * 65535.0);
        int ay = (int)((double)y / sh * 65535.0);
        input::mouse_move(ax, ay, true);
        return 0;
    }

    inline int mousemoverel(lua_State* L) {
        int dx = (int)luaL_checkinteger(L, 1);
        int dy = (int)luaL_checkinteger(L, 2);
        input::mouse_move(dx, dy, false);
        return 0;
    }

    inline int mousescroll(lua_State* L) {
        int delta = (int)luaL_checkinteger(L, 1);
        input::mouse_scroll(delta);
        return 0;
    }

    inline int isrbxactive(lua_State* L) {
        lua_pushboolean(L, input::is_roblox_active());
        return 1;
    }
}
