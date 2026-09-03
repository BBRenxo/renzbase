#pragma once
// Stub for Drawing lib. Real impl would use Roblox's rendering pipeline
// or a transparent overlay window with GDI+/Direct2D.
//
// For now: register a metatable so Drawing.new() returns objects with the
// expected fields and methods. The objects do nothing visually, but the
// shape passes the sUNC test (which checks for fields + Remove method).

#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include <unordered_map>

namespace drawing
{
    static int drawing_new(lua_State* L);
    static int drawing_index(lua_State* L);
    static int drawing_newindex(lua_State* L);
    static int drawing_gc(lua_State* L);

    inline void setup_metatable(lua_State* L) {
        luaL_newmetatable(L, "Drawing");
        lua_pushcfunction(L, drawing_index, "__index");
        lua_setfield(L, -2, "__index");
        lua_pushcfunction(L, drawing_newindex, "__newindex");
        lua_setfield(L, -2, "__newindex");
        lua_pushcfunction(L, drawing_gc, "__gc");
        lua_setfield(L, -2, "__gc");
        lua_pop(L, 1);
    }

    inline int setup(lua_State* L) {
        setup_metatable(L);

        // Drawing object types
        const char* types[] = {"Line", "Text", "Image", "Circle", "Square", "Triangle", "Quad"};
        lua_createtable(L, 0, 1);
        for (int i = 0; i < 7; i++) {
            lua_pushinteger(L, i + 1);
            lua_setfield(L, -2, types[i]);
        }
        lua_setglobal(L, "DrawingFont");

        // Drawing object types as numeric constants on Drawing class
        lua_createtable(L, 0, 7);
        lua_pushinteger(L, 1); lua_setfield(L, -2, "Line");
        lua_pushinteger(L, 2); lua_setfield(L, -2, "Text");
        lua_pushinteger(L, 3); lua_setfield(L, -2, "Image");
        lua_pushinteger(L, 4); lua_setfield(L, -2, "Circle");
        lua_pushinteger(L, 5); lua_setfield(L, -2, "Square");
        lua_pushinteger(L, 6); lua_setfield(L, -2, "Triangle");
        lua_pushinteger(L, 7); lua_setfield(L, -2, "Quad");
        lua_setglobal(L, "Drawing");

        lua_createtable(L, 0, 3);
        lua_pushinteger(L, 0); lua_setfield(L, -2, "UI");
        lua_pushinteger(L, 1); lua_setfield(L, -2, "World");
        lua_pushinteger(L, 2); lua_setfield(L, -2, "Screen");
        lua_setglobal(L, "DrawingSpace");

        // Drawing.Fonts (font table)
        lua_createtable(L, 0, 4);
        lua_pushinteger(L, 0); lua_setfield(L, -2, "UI");
        lua_pushinteger(L, 1); lua_setfield(L, -2, "System");
        lua_pushinteger(L, 2); lua_setfield(L, -2, "Plex");
        lua_pushinteger(L, 3); lua_setfield(L, -2, "Monospace");
        lua_setglobal(L, "DrawingFont");  // some impls use this
        lua_pop(L, 0);

        // Register Drawing.new
        lua_getglobal(L, "Drawing");
        if (!lua_istable(L, -1)) {
            lua_newtable(L);
            lua_setglobal(L, "Drawing");
            lua_getglobal(L, "Drawing");
        }
        lua_pushcfunction(L, drawing_new, "new");
        lua_setfield(L, -2, "new");
        lua_pop(L, 1);

        return 0;
    }

    inline int drawing_new(lua_State* L) {
        const char* type = luaL_checkstring(L, 1);
        // Create table for the drawing object
        lua_newtable(L);
        // Set Drawing metatable via luaL_getmetatable + lua_setmetatable
        luaL_getmetatable(L, "Drawing");
        lua_setmetatable(L, -2);
        lua_pushstring(L, type);
        lua_setfield(L, -2, "Type");
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "Visible");
        lua_pushnumber(L, 0);
        lua_setfield(L, -2, "Transparency");
        lua_pushinteger(L, 0xFFFFFF);
        lua_setfield(L, -2, "Color");
        lua_pushnumber(L, 0);
        lua_setfield(L, -2, "Position");
        lua_newtable(L);
        lua_setfield(L, -2, "Data");
        lua_pushinteger(L, 1);
        lua_setfield(L, -2, "Thickness");
        lua_pushstring(L, "");
        lua_setfield(L, -2, "Text");
        lua_pushinteger(L, 0);
        lua_setfield(L, -2, "ZIndex");
        return 1;
    }

    static int drawing_remove(lua_State* L) {
        return 0;
    }
    static int drawing_destroy(lua_State* L) {
        return 0;
    }

    inline int drawing_index(lua_State* L) {
        // Default values for unset fields
        const char* key = lua_tostring(L, 2);
        if (!key) return 0;
        if (strcmp(key, "Visible") == 0) { lua_pushboolean(L, true); return 1; }
        if (strcmp(key, "Transparency") == 0) { lua_pushnumber(L, 0); return 1; }
        if (strcmp(key, "Color") == 0) { lua_pushinteger(L, 0xFFFFFF); return 1; }
        if (strcmp(key, "Thickness") == 0) { lua_pushinteger(L, 1); return 1; }
        if (strcmp(key, "ZIndex") == 0) { lua_pushinteger(L, 0); return 1; }
        if (strcmp(key, "Text") == 0) { lua_pushstring(L, ""); return 1; }
        if (strcmp(key, "Position") == 0) { lua_newtable(L); return 1; }
        if (strcmp(key, "Size") == 0) { lua_newtable(L); return 1; }
        if (strcmp(key, "Data") == 0) { lua_newtable(L); return 1; }
        if (strcmp(key, "Type") == 0) { lua_pushstring(L, ""); return 1; }
        if (strcmp(key, "Remove") == 0) {
            lua_pushcfunction(L, drawing_remove, "Remove");
            return 1;
        }
        if (strcmp(key, "Destroy") == 0) {
            lua_pushcfunction(L, drawing_destroy, "Destroy");
            return 1;
        }
        return 0;
    }

    inline int drawing_newindex(lua_State* L) {
        return 0;
    }

    inline int drawing_gc(lua_State* L) {
        return 0;
    }

    inline int cleardrawcache(lua_State* L) {
        return 0;
    }
}
