#pragma once
#include "Libs/Http/Http.h"
#include "Libs/Closures/Closures.h"
#include "Libs/Misc/Misc.h"
#include "Libs/Crypt/Crypt.h"
#include "Libs/Metatable/Metatable.h"
#include "Libs/Filesystem/Filesystem.h"
#include "Libs/Input/Input.h"
#include "Libs/Console/Console.h"
#include "Libs/Drawing/Drawing.h"
#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "lstate.h"
#include "lobject.h"
#include "lgc.h"
#include "../../Core/Execution/Execution.h"
#include "../../Core/unc/Unc.h"
#include "../../Core/Input/Input.h"
#include "../../Core/Filesystem/Filesystem.h"
#include <Windows.h>
#include <string>
#include <unordered_set>

inline bool isitourskidthread(lua_State* L) {
    return L != nullptr && L->userdata != nullptr && (L->userdata->capabilities & 0b1000000);
}

namespace env
{
    inline lua_CFunction original_index = nullptr;
    inline lua_CFunction original_namecall = nullptr;

    // setunc(mode) — mode is "off" / "unc" / "sunc"
    // Sets unc/sunc on the current Lua thread.
    inline int setunc(lua_State* L) {
        const char* mode = luaL_checkstring(L, 1);
        if (!strcmp(mode, "off"))      unc::apply(L, unc::Mode::Off);
        else if (!strcmp(mode, "unc")) unc::apply(L, unc::Mode::Unc);
        else if (!strcmp(mode, "sunc")) unc::apply(L, unc::Mode::Sunc);
        else luaL_error(L, "setunc: mode must be 'off', 'unc', or 'sunc'");
        return 0;
    }

    // getsunc() — returns current unc mode for current thread.
    inline int getsunc(lua_State* L) {
        uint32_t sunc_flag = *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(L) + unc::SUNC_FLAG_OFFSET);
        uintptr_t yield = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uintptr_t>(L) + unc::THREAD_YIELD_COUNTER);
        if (sunc_flag)        lua_pushstring(L, "sunc");
        else if (yield == UINTPTR_MAX) lua_pushstring(L, "unc");
        else                   lua_pushstring(L, "off");
        return 1;
    }

    // extexecute_lua — Lua wrapper for Execution::extexecute.
    // Takes a string (script source), pushes it onto the execution queue.
    inline int extexecute_lua(lua_State* L) {
        if (lua_gettop(L) < 1) {
            luaL_error(L, "execute: expected 1 argument (script source)");
            return 0;
        }
        const char* src = luaL_checkstring(L, 1);
        if (!src || strlen(src) == 0) {
            luaL_error(L, "execute: script source is empty");
            return 0;
        }
        Execution::extexecute(std::string(src));
        return 0;
    }

    inline int index_hook(lua_State* L)
    {
        if (!isitourskidthread(L)) return original_index(L);

        if (lua_isstring(L, 2)) {
            const char* key = lua_tostring(L, 2);

            if (key && strcmp(key, "HttpGet") == 0)
            {
                lua_pushcfunction(L, http::HttpGet, "HttpGet");
                return 1;
            }
            if (key && strcmp(key, "HttpGetAsync") == 0)
            {
                lua_pushcfunction(L, http::HttpGet, "HttpGetAsync");
                return 1;
            }
        }
        
        if (original_index)
            return original_index(L);
        return 0;
    }

    inline int namecall_hook(lua_State* L)
    {
        if (!isitourskidthread(L)) return original_namecall(L);

        const char* key = L->namecall ? getstr(L->namecall) : nullptr;
        if (key) {
            if (strcmp(key, "HttpGet") == 0 || strcmp(key, "HttpGetAsync") == 0)
                return http::HttpGet(L);
        }
        
        if (original_namecall)
            return original_namecall(L);
        return 0;
    }


    // register funcs here not in libs plz
    inline void Register(lua_State* L)
    {
        luaL_sandboxthread(L);

        lua_pushcfunction(L, http::HttpGet, "HttpGet");
        lua_setglobal(L, "HttpGet");

        lua_pushcfunction(L, closures::loadstring, "loadstring");
        lua_setglobal(L, "loadstring");

        // identifyexecutor is registered later via Closures::identifyexecutor
        // (in the Closures block below) so it takes precedence.
        // NOTE: don't register it here — it would be overwritten.

        lua_pushcfunction(L, misc::getgenv, "getgenv");
        lua_setglobal(L, "getgenv");

        lua_pushcfunction(L, misc::getreg, "getreg");
        lua_setglobal(L, "getreg");

        lua_pushcfunction(L, misc::getrenv, "getrenv");
        lua_setglobal(L, "getrenv");

        lua_pushcfunction(L, misc::getthreadidentity, "getthreadidentity");
        lua_setglobal(L, "getthreadidentity");

        lua_pushcfunction(L, misc::getthreadidentity, "getidentity");
        lua_setglobal(L, "getidentity");

        lua_pushcfunction(L, misc::setthreadidentity, "setthreadidentity");
        lua_setglobal(L, "setthreadidentity");

        lua_pushcfunction(L, misc::setthreadidentity, "setidentity");
        lua_setglobal(L, "setidentity");

        // === Execute (queue-based script runner) ===
        // Queues a script for execution on the main SkidBase thread.
        // Usage: execute("print('hello')")
        lua_pushcfunction(L, env::extexecute_lua, "execute");
        lua_setglobal(L, "execute");
        lua_pushcfunction(L, env::extexecute_lua, "queuelua");
        lua_setglobal(L, "queuelua");
        lua_pushcfunction(L, env::extexecute_lua, "queue_script");
        lua_setglobal(L, "queue_script");
        lua_pushcfunction(L, env::extexecute_lua, "skid_execute");
        lua_setglobal(L, "skid_execute");

        lua_pushcfunction(L, misc::gethui, "gethui");
        lua_setglobal(L, "gethui");

        lua_pushcfunction(L, misc::compareinstances, "compareinstances");
        lua_setglobal(L, "compareinstances");

        lua_pushcfunction(L, misc::setclipboard, "setclipboard");
        lua_setglobal(L, "setclipboard");

        lua_pushcfunction(L, misc::setclipboard, "toclipboard");
        lua_setglobal(L, "toclipboard");

        lua_pushcfunction(L, misc::getfpscap, "getfpscap");
        lua_setglobal(L, "getfpscap");

        lua_pushcfunction(L, misc::setfpscap, "setfpscap");
        lua_setglobal(L, "setfpscap");

        lua_pushcfunction(L, closures::checkcaller, "checkcaller");
        lua_setglobal(L, "checkcaller");

        lua_pushcfunction(L, closures::iscclosure, "iscclosure");
        lua_setglobal(L, "iscclosure");

        lua_pushcfunction(L, closures::islclosure, "islclosure");
        lua_setglobal(L, "islclosure");

        lua_pushcfunction(L, closures::clonefunction, "clonefunction");
        lua_setglobal(L, "clonefunction");

        lua_pushcfunction(L, closures::identifyexecutor, "identifyexecutor");
        lua_setglobal(L, "identifyexecutor");

        lua_pushcfunction(L, closures::isexecutorclosure, "isexecutorclosure");
        lua_setglobal(L, "isexecutorclosure");

        lua_pushcfunction(L, closures::isexecutorclosure, "checkclosure");
        lua_setglobal(L, "checkclosure");

        // unc / sunc — executor yield limit bypass
        lua_pushcfunction(L, env::setunc, "setunc");
        lua_setglobal(L, "setunc");
        lua_pushcfunction(L, env::getsunc, "getsunc");
        lua_setglobal(L, "getsunc");

        // === FileSystem ===
        lua_pushcfunction(L, filesys::writefile, "writefile");
        lua_setglobal(L, "writefile");
        lua_pushcfunction(L, filesys::readfile, "readfile");
        lua_setglobal(L, "readfile");
        lua_pushcfunction(L, filesys::appendfile, "appendfile");
        lua_setglobal(L, "appendfile");
        lua_pushcfunction(L, filesys::loadfile, "loadfile");
        lua_setglobal(L, "loadfile");
        lua_pushcfunction(L, filesys::delfile, "delfile");
        lua_setglobal(L, "delfile");
        lua_pushcfunction(L, filesys::makefolder, "makefolder");
        lua_setglobal(L, "makefolder");
        lua_pushcfunction(L, filesys::listfiles, "listfiles");
        lua_setglobal(L, "listfiles");
        lua_pushcfunction(L, filesys::isfolder, "isfolder");
        lua_setglobal(L, "isfolder");
        lua_pushcfunction(L, filesys::isfile, "isfile");
        lua_setglobal(L, "isfile");
        lua_pushcfunction(L, filesys::delfolder, "delfolder");
        lua_setglobal(L, "delfolder");
        lua_pushcfunction(L, filesys::getcustomasset, "getcustomasset");
        lua_setglobal(L, "getcustomasset");

        // === Network ===
        lua_pushcfunction(L, http::request, "request");
        lua_setglobal(L, "request");
        lua_pushcfunction(L, http::HttpGet, "HttpGet");
        lua_setglobal(L, "HttpGet");
        lua_pushcfunction(L, http::HttpGetAsync, "HttpGetAsync");
        lua_setglobal(L, "HttpGetAsync");
        lua_pushcfunction(L, http::HttpPost, "HttpPost");
        lua_setglobal(L, "HttpPost");

        // === Input ===
        lua_pushcfunction(L, inp::keypress, "keypress");
        lua_setglobal(L, "keypress");
        lua_pushcfunction(L, inp::keyrelease, "keyrelease");
        lua_setglobal(L, "keyrelease");
        lua_pushcfunction(L, inp::keyhold, "keyhold");
        lua_setglobal(L, "keyhold");
        lua_pushcfunction(L, inp::mouse1click, "mouse1click");
        lua_setglobal(L, "mouse1click");
        lua_pushcfunction(L, inp::mouse1press, "mouse1press");
        lua_setglobal(L, "mouse1press");
        lua_pushcfunction(L, inp::mouse1release, "mouse1release");
        lua_setglobal(L, "mouse1release");
        lua_pushcfunction(L, inp::mouse2click, "mouse2click");
        lua_setglobal(L, "mouse2click");
        lua_pushcfunction(L, inp::mouse2press, "mouse2press");
        lua_setglobal(L, "mouse2press");
        lua_pushcfunction(L, inp::mouse2release, "mouse2release");
        lua_setglobal(L, "mouse2release");
        lua_pushcfunction(L, inp::mousemoveabs, "mousemoveabs");
        lua_setglobal(L, "mousemoveabs");
        lua_pushcfunction(L, inp::mousemoverel, "mousemoverel");
        lua_setglobal(L, "mousemoverel");
        lua_pushcfunction(L, inp::mousescroll, "mousescroll");
        lua_setglobal(L, "mousescroll");
        lua_pushcfunction(L, inp::isrbxactive, "isrbxactive");
        lua_setglobal(L, "isrbxactive");
        lua_pushcfunction(L, inp::isrbxactive, "isgameactive");
        lua_setglobal(L, "isgameactive");

        // === Console ===
        lua_pushcfunction(L, console::rconsolecreate, "rconsolecreate");
        lua_setglobal(L, "rconsolecreate");
        lua_pushcfunction(L, console::rconsoleclose, "rconsoleclose");
        lua_setglobal(L, "rconsoleclose");
        lua_pushcfunction(L, console::rconsoleprint, "rconsoleprint");
        lua_setglobal(L, "rconsoleprint");
        lua_pushcfunction(L, console::rconsoleprint, "rconsoleprintln");
        lua_setglobal(L, "rconsoleprintln");
        lua_pushcfunction(L, console::rconsolewarn, "rconsolewarn");
        lua_setglobal(L, "rconsolewarn");
        lua_pushcfunction(L, console::rconsoleerror, "rconsoleerror");
        lua_setglobal(L, "rconsoleerror");
        lua_pushcfunction(L, console::rconsoleinfo, "rconsoleinfo");
        lua_setglobal(L, "rconsoleinfo");
        lua_pushcfunction(L, console::rconsoleclear, "rconsoleclear");
        lua_setglobal(L, "rconsoleclear");
        lua_pushcfunction(L, console::rconsolename, "rconsolename");
        lua_setglobal(L, "rconsolename");
        lua_pushcfunction(L, console::rconsolename, "rconsoletitle");
        lua_setglobal(L, "rconsoletitle");

        // === Drawing ===
        drawing::setup(L);
        lua_pushcfunction(L, drawing::cleardrawcache, "cleardrawcache");
        lua_setglobal(L, "cleardrawcache");

        // === Debug lib === (debug.h removed for now - Luau API mismatch)
        // Skipped until proper bytecode parser is written.

        // metatable lib
        lua_pushcfunction(L, metatable::getrawmetatable, "getrawmetatable");
        lua_setglobal(L, "getrawmetatable");

        lua_pushcfunction(L, metatable::setrawmetatable, "setrawmetatable");
        lua_setglobal(L, "setrawmetatable");

        lua_pushcfunction(L, metatable::setreadonly, "setreadonly");
        lua_setglobal(L, "setreadonly");

        lua_pushcfunction(L, metatable::isreadonly, "isreadonly");
        lua_setglobal(L, "isreadonly");

        lua_pushcfunction(L, metatable::getnamecallmethod, "getnamecallmethod");
        lua_setglobal(L, "getnamecallmethod");

        lua_pushcfunction(L, metatable::hookmetamethod, "hookmetamethod");
        lua_setglobal(L, "hookmetamethod");

        lua_newtable(L);
        lua_pushcfunction(L, crypt_lib::base64encode, "base64encode");
        lua_setfield(L, -2, "base64encode");
        lua_pushcfunction(L, crypt_lib::base64_encode, "base64_encode");
        lua_setfield(L, -2, "base64_encode");
        lua_pushcfunction(L, crypt_lib::base64decode, "base64decode");
        lua_setfield(L, -2, "base64decode");
        lua_pushcfunction(L, crypt_lib::base64_decode, "base64_decode");
        lua_setfield(L, -2, "base64_decode");
        lua_pushcfunction(L, crypt_lib::lz4compress, "lz4compress");
        lua_setfield(L, -2, "lz4compress");
        lua_pushcfunction(L, crypt_lib::lz4decompress, "lz4decompress");
        lua_setfield(L, -2, "lz4decompress");
        lua_pushcfunction(L, crypt_lib::generatebytes, "generatebytes");
        lua_setfield(L, -2, "generatebytes");
        lua_pushcfunction(L, crypt_lib::generatekey, "generatekey");
        lua_setfield(L, -2, "generatekey");
        lua_pushcfunction(L, crypt_lib::hash, "hash");
        lua_setfield(L, -2, "hash");
        lua_setglobal(L, "crypt");

        lua_pushcfunction(L, crypt_lib::base64_encode, "base64_encode");
        lua_setglobal(L, "base64_encode");
        lua_pushcfunction(L, crypt_lib::base64_decode, "base64_decode");
        lua_setglobal(L, "base64_decode");
        lua_pushcfunction(L, crypt_lib::lz4compress, "lz4compress");
        lua_setglobal(L, "lz4compress");
        lua_pushcfunction(L, crypt_lib::lz4decompress, "lz4decompress");
        lua_setglobal(L, "lz4decompress");
        lua_pushcfunction(L, crypt_lib::getfunctionhash, "getfunctionhash");
        lua_setglobal(L, "getfunctionhash");

        int top = lua_gettop(L);
        lua_getglobal(L, "game");

        if (lua_istable(L, -1) || lua_isuserdata(L, -1))
        {
            if (luaL_getmetafield(L, -1, "__index"))
            {
                if (lua_type(L, -1) == LUA_TFUNCTION)
                {
                    Closure* cl = clvalue(const_cast<TValue*>(luaA_toobject(L, -1)));
                    original_index = cl->c.f;
                    cl->c.f = index_hook;
                }
                lua_pop(L, 1);
            }

            if (luaL_getmetafield(L, -1, "__namecall"))
            {
                if (lua_type(L, -1) == LUA_TFUNCTION)
                {
                    Closure* cl = clvalue(const_cast<TValue*>(luaA_toobject(L, -1)));
                    original_namecall = cl->c.f;
                    cl->c.f = namecall_hook;
                }
                lua_pop(L, 1);
            }
        }

        lua_settop(L, top);

        // REMOVED: loadstring(game:HttpGet(...))() — this was crashing Roblox
        // (loadstring disabled in production Roblox since 2023, HttpGet isn't
        // a real Roblox API). If users want extra libs they should load them
        // via execute() instead.
    }
}
