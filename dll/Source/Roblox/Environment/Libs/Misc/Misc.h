#pragma once
#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "lstate.h"
#include "lapi.h"
#include "ltable.h"
#include "lgc.h"
#include "../../../../Core/Execution/Execution.h"
#include "../../../Offsets/Offsets.h"
#include <Windows.h>
#include <string>
#include <random>

namespace misc
{
    // RenzBase stable identity. Returns the same name every call within a
    // single script context (thread-local), different scripts may see
    // different names. This is what most executor-detection scripts expect:
    // a stable string they can compare with ==.
    //
    // The pool gives anti-cheat scripts multiple plausible hits — if they
    // hardcode == "Fluxus" only, we return Fluxus. If they allowlist a
    // list of executors, we cover several. If they check == "RenzBase"
    // specifically, we return that too.
    inline const char* EXECUTOR_NAMES[] = {
        "RenzBase", "Fluxus", "Electron", "Trigon", "Wave", "AWP",
        "Script-Ware V2", "Arsenal", "Solara", "Nihon", "Codex",
        "Velocity", "Arceus X"
    };
    inline const size_t NUM_NAMES = sizeof(EXECUTOR_NAMES) / sizeof(EXECUTOR_NAMES[0]);

    // Per-call random pick. Anti-cheat fingerprinting protection — different
    // calls return different names so a single observed value can't be used
    // to block all RenzBase users.
    inline int getexecutorname(lua_State* L)
    {
        static std::mt19937 rng(std::random_device{}());
        const char* name = EXECUTOR_NAMES[rng() % NUM_NAMES];
        lua_pushstring(L, name);
        return 1;
    }

    // Stable pick: same script always sees the same name (uses lua_State*
    // address as seed). Standard executor-detection scripts expect this.
    inline int getexecutorname_stable(lua_State* L) {
        uintptr_t thread_id = reinterpret_cast<uintptr_t>(L);
        const char* name = EXECUTOR_NAMES[thread_id % NUM_NAMES];
        lua_pushstring(L, name);
        return 1;
    }

    // Per-session stable: same name for the whole session, different sessions
    // get different names. Combines randomness with stability.
    inline int getexecutorname_session(lua_State* L) {
        // Seed from session start time + process id (constant during session)
        static std::mt19937 rng(GetTickCount() ^ GetCurrentProcessId());
        static std::string session_name = EXECUTOR_NAMES[rng() % NUM_NAMES];
        lua_pushstring(L, session_name.c_str());
        return 1;
    }

    inline int getgenv(lua_State* L)
    {
        lua_State* mainState = Execution::skidsstate;

        if (mainState == L) {
            lua_pushvalue(L, LUA_GLOBALSINDEX);
            return 1;
        }

        luaC_threadbarrier(mainState);
        lua_pushvalue(mainState, LUA_GLOBALSINDEX);
        lua_xmove(mainState, L, 1);

        return 1;
    }

    inline int getreg(lua_State* L)
    {
        lua_pushvalue(L, LUA_REGISTRYINDEX);
        return 1;
    }

    inline int getrenv(lua_State* L)
    {
        lua_State* RobloxState = L->global->mainthread;
        LuaTable* clone = luaH_clone(L, RobloxState->gt);

        lua_rawcheckstack(L, 1);
        luaC_threadbarrier(L);
        luaC_threadbarrier(RobloxState);

        L->top->value.p = clone;
        L->top->tt = LUA_TTABLE;
        L->top++;

        lua_rawgeti(L, LUA_REGISTRYINDEX, 2);
        lua_setfield(L, -2, "_G");
        lua_rawgeti(L, LUA_REGISTRYINDEX, 4);
        lua_setfield(L, -2, "shared");

        return 1;
    }

    inline int getthreadidentity(lua_State* L)
    {
        if (!L->userdata) {
            lua_pushinteger(L, 0);
            return 1;
        }
        lua_pushinteger(L, L->userdata->identity);
        return 1;
    }

    inline int setthreadidentity(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TNUMBER);
        int identity = static_cast<int>(lua_tointeger(L, 1));
        if (L->userdata == nullptr)
            return 0;

        uint64_t caps = (identity == 8) ? 0xFFFFFFFFFFFFFFFFULL : (0xFFFFFFFFFFFFFF00ULL | SkidBase::GetCapabilities(&identity));

        L->userdata->identity = identity;
        L->userdata->capabilities = caps;

        uintptr_t thread_tls = *reinterpret_cast<uintptr_t*>(Main::Identity1::IdentityPointer);
        uintptr_t idpointer = SkidBase::GetIdentityStruct(thread_tls);
        if (idpointer != 0) {
            *reinterpret_cast<uint64_t*>(idpointer) = identity;
            *reinterpret_cast<uint64_t*>(idpointer + Main::Identity2::Capabilities) = caps;
        }

        return 0;
    }

    inline int gethui(lua_State* L)
    {
        lua_getfield(L, LUA_GLOBALSINDEX, "game");
        lua_getfield(L, -1, "CoreGui");
        return 1;
    }

    inline int compareinstances(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TUSERDATA);

        uintptr_t skid = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
        uintptr_t paster = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 2));

        lua_pushboolean(L, skid == paster);
        return 1;
    }

    inline int setclipboard(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TSTRING);
        std::string content = lua_tostring(L, 1);

        HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, content.size() + 1);
        if (!hmem)
            return 0;

        void* lockedMem = GlobalLock(hmem);
        if (!lockedMem) {
            GlobalFree(hmem);
            return 0;
        }

        memcpy(lockedMem, content.data(), content.size());
        static_cast<char*>(lockedMem)[content.size()] = '\0';
        GlobalUnlock(hmem);

        if (!OpenClipboard(nullptr)) {
            GlobalFree(hmem);
            return 0;
        }

        EmptyClipboard();
        SetClipboardData(CF_TEXT, hmem);
        CloseClipboard();

        return 0;
    }

    inline int getfpscap(lua_State* L)
    {
        int target = *reinterpret_cast<int*>(Main::Miscellaneous::TargetFPS);
        lua_pushinteger(L, target);
        return 1;
    }

    inline int setfpscap(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TNUMBER);
        *reinterpret_cast<int*>(Main::Miscellaneous::TargetFPS) = static_cast<int>(lua_tonumber(L, 1));
        return 0;
    }
}
