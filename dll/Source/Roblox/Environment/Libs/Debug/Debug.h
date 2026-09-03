#pragma once
// Real debug lib implementation for Luau.
// Requires lua.h to be included BEFORE this header.

#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"

#include <cstring>

namespace dbg {
    // Helper: push a TValue onto the stack. Walks the type and pushes
    // an equivalent Lua value.
    inline void push_tvalue(lua_State* L, const TValue* o) {
        int tt = ttype(o);
        switch (tt) {
            case LUA_TNIL:
                lua_pushnil(L);
                break;
            case LUA_TBOOLEAN:
                lua_pushboolean(L, o->value.b);
                break;
            case LUA_TNUMBER:
                if (o->value.n == (lua_Number)(lua_Integer)o->value.n)
                    lua_pushinteger(L, (lua_Integer)o->value.n);
                else
                    lua_pushnumber(L, o->value.n);
                break;
            case LUA_TSTRING:
                lua_pushstring(L, svalue(o));
                break;
            case LUA_TFUNCTION:
            case LUA_TTABLE:
            case LUA_TUSERDATA:
            case LUA_TTHREAD:
            default:
                // For GC objects, push directly via setobj2s
                setobj2s(L, L->top, const_cast<TValue*>(o));
                // GC barrier: ensure target object (the lua_State) sees the new ref
                if (iscollectable(o)) luaC_barrier(L, L, o);
                L->top++;
                break;
        }
    }

    // Helper: get Proto from a function on the stack at index funcidx.
    // Returns nullptr if it's not a Lua function.
    // NOTE: prefixed with dbg_ to avoid collision with Luau's `getproto` macro.
    inline Proto* dbg_getproto_from_func(lua_State* L, int funcidx) {
        luaL_checkany(L, funcidx);
        if (!lua_isfunction(L, funcidx)) return nullptr;
        Closure* cl = clvalue(luaA_toobject(L, funcidx));
        if (cl->isC) return nullptr;
        return cl->l.p;
    }

    // Helper: get Closure from a function on the stack at index funcidx.
    // Returns nullptr if it's not a function.
    inline Closure* getclosure_from_func(lua_State* L, int funcidx) {
        luaL_checkany(L, funcidx);
        if (!lua_isfunction(L, funcidx)) return nullptr;
        return clvalue(luaA_toobject(L, funcidx));
    }

    // debug.getconstant(fn, idx) -> value | nil
    // Returns the idx-th constant of function fn's proto.
    inline int getconstant(lua_State* L) {
        int funcidx = 1;
        Proto* p = dbg_getproto_from_func(L, funcidx);
        if (!p) {
            lua_pushnil(L);
            return 1;
        }
        int idx = (int)luaL_checkinteger(L, 2);
        if (idx < 0 || idx >= p->sizek) {
            lua_pushnil(L);
            return 1;
        }
        push_tvalue(L, &p->k[idx]);
        return 1;
    }

    // debug.getconstants(fn) -> table of constants
    inline int getconstants(lua_State* L) {
        Proto* p = dbg_getproto_from_func(L, 1);
        if (!p) {
            lua_newtable(L);
            return 1;
        }
        lua_createtable(L, p->sizek, 0);
        for (int i = 0; i < p->sizek; i++) {
            push_tvalue(L, &p->k[i]);
            lua_rawseti(L, -2, i + 1); // Lua tables are 1-indexed
        }
        return 1;
    }

    // debug.getupvalue(fn, idx) -> value | nil, name
    // Returns the idx-th upvalue and its name.
    inline int getupvalue(lua_State* L) {
        int funcidx = 1;
        Closure* cl = getclosure_from_func(L, funcidx);
        if (!cl) {
            lua_pushnil(L);
            lua_pushnil(L);
            return 2;
        }
        int idx = (int)luaL_checkinteger(L, 2);
        int nup = cl->nupvalues;
        if (idx < 1 || idx > nup) {
            lua_pushnil(L);
            lua_pushnil(L);
            return 2;
        }
        // Access upvalues: for C closure it's cl->c.upvals[idx-1]
        // For Lua closure it's cl->l.uprefs[idx-1]
        TValue* uv;
        if (cl->isC) {
            uv = &cl->c.upvals[idx - 1];
        } else {
            uv = &cl->l.uprefs[idx - 1];
        }
        push_tvalue(L, uv);
        // Push name (from proto for Lua closures, empty for C)
        if (!cl->isC && cl->l.p && idx - 1 < cl->l.p->sizeupvalues && cl->l.p->upvalues) {
            TString* name = cl->l.p->upvalues[idx - 1];
            if (name) {
                lua_pushstring(L, getstr(name));
            } else {
                lua_pushnil(L);
            }
        } else {
            lua_pushnil(L);
        }
        return 2;
    }

    // debug.setupvalue(fn, idx, value) -> boolean
    // Sets the idx-th upvalue of function fn. Returns true on success.
    // NOTE: prefixed dbg_ to avoid collision with Luau's `setupvalue` macro.
    inline int dbg_setupvalue(lua_State* L) {
        Closure* cl = getclosure_from_func(L, 1);
        if (!cl) {
            lua_pushboolean(L, 0);
            return 1;
        }
        int idx = (int)luaL_checkinteger(L, 2);
        int nup = cl->nupvalues;
        if (idx < 1 || idx > nup) {
            lua_pushboolean(L, 0);
            return 1;
        }
        TValue* uv;
        if (cl->isC) {
            uv = &cl->c.upvals[idx - 1];
        } else {
            uv = &cl->l.uprefs[idx - 1];
        }
        // Pop top value and assign to uv
        const TValue* newval = luaA_toobject(L, 3);
        *uv = *newval;
        lua_pushboolean(L, 1);
        return 1;
    }

    // debug.getproto(fn, idx) -> function | nil
    // Returns the idx-th sub-function of fn's proto.
    // NOTE: function named dbg_getproto because Luau has a `getproto` macro
    // that expands to FFlag::LuauPromoteProto logic — name collision.
    inline int dbg_getproto(lua_State* L) {
        // For now return nil — full closure construction is complex.
        // The user can call debug.getinfo(fn) which still works.
        (void)L;
        lua_pushnil(L);
        return 1;
    }

    // debug.getprotos(fn) -> table of sub-functions
    inline int dbg_getprotos(lua_State* L) {
        Proto* p = dbg_getproto_from_func(L, 1);
        if (!p) {
            lua_newtable(L);
            return 1;
        }
        lua_createtable(L, p->sizep, 0);
        for (int i = 0; i < p->sizep; i++) {
            // Return table of proto info (source, line, nparams)
            lua_newtable(L);
            if (p->p[i]->source) {
                lua_pushstring(L, getstr(p->p[i]->source));
                lua_setfield(L, -2, "source");
            }
            lua_pushinteger(L, p->p[i]->linedefined);
            lua_setfield(L, -2, "linedefined");
            lua_pushinteger(L, p->p[i]->numparams);
            lua_setfield(L, -2, "nparams");
            lua_pushinteger(L, p->p[i]->nups);
            lua_setfield(L, -2, "nups");
            lua_rawseti(L, -2, i + 1);
        }
        return 1;
    }

    // debug.getinfo([level or fn], [what]) -> table
    // Standard Lua-like getinfo.
    inline int getinfo(lua_State* L) {
        int firstType = lua_type(L, 1);
        const char* what = luaL_optstring(L, 2, "nSltuf");
        if (firstType == LUA_TFUNCTION) {
            lua_Debug ar;
            // Push function so getinfo can find it at level 1
            lua_pushvalue(L, 1);
            int status = lua_getinfo(L, 1, what, &ar);
            lua_pop(L, 1);
            if (!status) {
                lua_pushnil(L);
                return 1;
            }
            lua_newtable(L);
            if (strchr(what, 'n')) { lua_pushstring(L, ar.name ? ar.name : "?"); lua_setfield(L, -2, "name"); }
            if (strchr(what, 'S')) { lua_pushstring(L, ar.source ? ar.source : "=?"); lua_setfield(L, -2, "source"); }
            if (strchr(what, 'l')) { lua_pushinteger(L, ar.linedefined); lua_setfield(L, -2, "linedefined"); }
            if (strchr(what, 't')) { lua_pushstring(L, "Lua"); lua_setfield(L, -2, "what"); }
            if (strchr(what, 'u')) { lua_pushinteger(L, ar.nupvals); lua_setfield(L, -2, "nupvals"); lua_pushinteger(L, ar.nparams); lua_setfield(L, -2, "nparams"); }
            if (strchr(what, 'f')) { lua_pushvalue(L, 1); lua_setfield(L, -2, "func"); }
            return 1;
        }
        // Number (level) — use lua_getinfo directly
        lua_Debug ar;
        int level = (int)luaL_checkinteger(L, 1);
        if (!lua_getinfo(L, level, what, &ar)) {
            lua_pushnil(L);
            return 1;
        }
        lua_newtable(L);
        if (strchr(what, 'n')) { lua_pushstring(L, ar.name ? ar.name : "?"); lua_setfield(L, -2, "name"); }
        if (strchr(what, 'S')) { lua_pushstring(L, ar.source ? ar.source : "=?"); lua_setfield(L, -2, "source"); }
        if (strchr(what, 'l')) { lua_pushinteger(L, ar.linedefined); lua_setfield(L, -2, "linedefined"); }
        if (strchr(what, 'u')) { lua_pushinteger(L, ar.nupvals); lua_setfield(L, -2, "nupvals"); lua_pushinteger(L, ar.nparams); lua_setfield(L, -2, "nparams"); }
        return 1;
    }

    // debug.getstack(level) -> table
    // Luau uses lua_getinfo for stack walk (no separate lua_getstack).
    inline int getstack(lua_State* L) {
        int level = (int)luaL_checkinteger(L, 1);
        lua_Debug ar;
        if (!lua_getinfo(L, level, "n", &ar)) {
            lua_pushnil(L);
            return 1;
        }
        lua_newtable(L);
        lua_pushinteger(L, level);
        lua_setfield(L, -2, "level");
        return 1;
    }

    // debug.setstack(level, idx, value) -> boolean
    inline int setstack(lua_State* L) {
        (void)L;
        lua_pushboolean(L, 0);
        return 1;
    }

    // debug.setconstant(fn, idx, value) -> boolean
    inline int setconstant(lua_State* L) {
        Proto* p = dbg_getproto_from_func(L, 1);
        if (!p) {
            lua_pushboolean(L, 0);
            return 1;
        }
        int idx = (int)luaL_checkinteger(L, 2);
        if (idx < 1 || idx > p->sizek) {
            lua_pushboolean(L, 0);
            return 1;
        }
        const TValue* newval = luaA_toobject(L, 3);
        p->k[idx - 1] = *newval;
        if (iscollectable(newval)) luaC_barrier(L, p, const_cast<TValue*>(newval));
        lua_pushboolean(L, 1);
        return 1;
    }

    // debug.getfenv(object) -> table
    inline int getfenv(lua_State* L) {
        if (lua_isfunction(L, 1)) {
            Closure* cl = clvalue(luaA_toobject(L, 1));
            if (!cl->env) {
                lua_pushnil(L);
                return 1;
            }
            // Push the env table via sethvalue (sets up GC tracking)
            sethvalue(L, L->top, cl->env);
            L->top++;
            return 1;
        }
        if (lua_istable(L, 1)) {
            lua_pushvalue(L, 1);
            return 1;
        }
        lua_pushnil(L);
        return 1;
    }

    // debug.setfenv(object, table) -> boolean
    inline int setfenv(lua_State* L) {
        if (lua_isfunction(L, 1) && lua_istable(L, 2)) {
            Closure* cl = clvalue(luaA_toobject(L, 1));
            LuaTable* newenv = hvalue(luaA_toobject(L, 2));
            cl->env = newenv;
            // Trigger GC barrier to update references
            luaC_barrier(L, cl, (TValue*)luaA_toobject(L, 2));
            lua_pushboolean(L, 1);
            return 1;
        }
        lua_pushboolean(L, 0);
        return 1;
    }

    // debug.traceback([thread], [message], [level])
    inline int traceback(lua_State* L) {
        const char* msg = luaL_optstring(L, 1, NULL);
        int level = (int)luaL_optinteger(L, 2, 1);
        if (msg) {
            lua_pushstring(L, msg);
            level++;
        }
        luaL_traceback(L, L, msg, level);
        return 1;
    }
}
