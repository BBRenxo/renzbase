#pragma once
#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "lstate.h"
#include "lapi.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "../../../../Core/Execution/Execution.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

bool isitourskidthread(lua_State* L);

namespace closures
{
    inline int loadstring(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TSTRING);
        const char* src = lua_tostring(L, 1);
        const char* chunkname = luaL_optstring(L, 2, "SkidBase");

        std::string bytecode = Execution::aexecute(src);
        if (bytecode.empty() || bytecode[0] == '\0')
        {
            lua_pushnil(L);
            lua_pushstring(L, bytecode.c_str() + 1);
            return 2;
        }

        if (luau_load(L, chunkname, bytecode.c_str(), bytecode.size(), 0) != LUA_OK)
        {
            lua_pushnil(L);
            lua_pushvalue(L, -2);
            return 2;
        }

        Closure* fn = clvalue(const_cast<TValue*>(luaA_toobject(L, -1)));
        if (fn && fn->l.p)
            Execution::setprotocapabilities(fn->l.p, const_cast<uintptr_t*>(&Execution::caps));

        lua_setsafeenv(L, LUA_GLOBALSINDEX, false);
        return 1;
    }

    inline int checkcaller(lua_State* L)
    {
        lua_pushboolean(L, isitourskidthread(L));
        return 1;
    }

    inline int iscclosure(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_pushboolean(L, lua_iscfunction(L, 1));
        return 1;
    }

    inline int islclosure(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_pushboolean(L, !lua_iscfunction(L, 1));
        return 1;
    }

    inline int clonefunction(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_clonefunction(L, 1);
        return 1;
    }

    // Executor identity. RenzBase returns a name from a pool of plausible
    // executor names. Per-script context, the name is stable (so script
    // comparison works), but across different scripts we can return
    // different names. Also returns a version string for compatibility.
    inline const char* EXECUTOR_NAMES_C[] = {
        "RenzBase", "Fluxus", "Electron", "Trigon", "Wave", "AWP",
        "Script-Ware V2", "Arsenal", "Solara", "Nihon", "Codex",
        "Velocity", "Arceus X"
    };
    inline const size_t NUM_NAMES_C = sizeof(EXECUTOR_NAMES_C) / sizeof(EXECUTOR_NAMES_C[0]);

    inline int identifyexecutor(lua_State* L)
    {
        // Stable per-script: use lua_State* address as seed so the same
        // script sees the same name across calls. Different scripts in the
        // same session may see different names — that's intentional, to
        // make per-script fingerprinting impossible.
        uintptr_t thread_id = reinterpret_cast<uintptr_t>(L);
        const char* name = EXECUTOR_NAMES_C[thread_id % NUM_NAMES_C];
        lua_pushstring(L, name);
        lua_pushstring(L, "1.0.0");
        return 2;
    }

    inline int isexecutorclosure(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        Closure* cl = clvalue(const_cast<TValue*>(luaA_toobject(L, 1)));
        lua_pushboolean(L, isitourskidthread(L) && cl != nullptr);
        return 1;
    }
}