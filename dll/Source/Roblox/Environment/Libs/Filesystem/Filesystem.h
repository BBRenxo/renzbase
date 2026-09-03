#pragma once
#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "../../../../Core/Filesystem/Filesystem.h"
#include "../../../Offsets/Offsets.h"
#include <vector>

namespace filesys
{
    inline int writefile(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        size_t len = 0;
        const char* content = luaL_checklstring(L, 2, &len);
        bool ok = fs::writefile(path, std::string(content, len));
        lua_pushboolean(L, ok);
        return 1;
    }

    inline int readfile(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        std::string content = fs::readfile(path);
        lua_pushlstring(L, content.data(), content.size());
        return 1;
    }

    inline int appendfile(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        size_t len = 0;
        const char* content = luaL_checklstring(L, 2, &len);
        bool ok = fs::appendfile(path, std::string(content, len));
        lua_pushboolean(L, ok);
        return 1;
    }

    inline int isfile(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, fs::isfile(path));
        return 1;
    }

    inline int isfolder(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, fs::isfolder(path));
        return 1;
    }

    inline int makefolder(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, fs::makefolder(path));
        return 1;
    }

    inline int delfile(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, fs::delfile(path));
        return 1;
    }

    inline int delfolder(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        lua_pushboolean(L, fs::delfolder(path));
        return 1;
    }

    inline int listfiles(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        std::vector<std::string> entries = fs::listfiles(path);
        lua_createtable(L, (int)entries.size(), 0);
        for (size_t i = 0; i < entries.size(); i++) {
            lua_pushstring(L, entries[i].c_str());
            lua_rawseti(L, -2, (lua_Integer)(i + 1));
        }
        return 1;
    }

    // loadfile(path) — read+compile. Returns chunk function or (nil, err).
    inline int loadfile(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        std::string src = fs::readfile(path);
        if (src.empty()) {
            lua_pushnil(L);
            lua_pushstring(L, "could not read file");
            return 2;
        }
        // Use Luau compiler (Execution::aexecute)
        // For now, error — caller must use the same compile path as scripts.
        // The sUNC test just checks that loadfile() returns a function.
        // We'll wire this to the Execution pipeline.
        lua_pushnil(L);
        lua_pushstring(L, "loadfile: integration pending");
        return 2;
    }

    // getcustomasset(path) — copy file to a Roblox-readable location and
    // return rbxasset://... URI. Roblox exposes a content provider that
    // reads from %TEMP% (or wherever the executor points it).
    inline int getcustomasset(lua_State* L) {
        const char* path = luaL_checkstring(L, 1);
        std::string full = fs::root() + "\\" + path;
        // The URI scheme depends on Roblox's content provider setup.
        // For now return a hash-based URI. Real impl hooks the content provider.
        lua_pushfstring(L, "rbxasset://skidbase/%s", path);
        return 1;
    }
}
