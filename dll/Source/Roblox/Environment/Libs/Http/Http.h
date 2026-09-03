#pragma once
#include "lua.h"
#include "lualib.h"
#include "lobject.h"
#include "../../../../Core/Network/Network.h"
#include <string>

namespace http
{
    // request(options) — modern executor API. options is {Url, Method, Body, Headers}.
    // Returns {Success, StatusCode, Body, Headers} or false on failure.
    inline int request(lua_State* L) {
        if (!lua_istable(L, 1)) {
            lua_pushnil(L);
            lua_pushstring(L, "request: expected table argument");
            return 2;
        }
        // Read Url
        lua_getfield(L, 1, "Url");
        const char* url = lua_isstring(L, -1) ? lua_tostring(L, -1) : nullptr;
        lua_pop(L, 1);
        if (!url) {
            lua_pushnil(L);
            lua_pushstring(L, "request: missing Url");
            return 2;
        }
        // Read Method
        std::string method = "GET";
        lua_getfield(L, 1, "Method");
        if (lua_isstring(L, -1)) method = lua_tostring(L, -1);
        lua_pop(L, 1);
        // Read Body
        std::string body;
        lua_getfield(L, 1, "Body");
        if (lua_isstring(L, -1)) body = lua_tostring(L, -1);
        lua_pop(L, 1);
        // Read Headers
        std::vector<std::pair<std::string, std::string>> hdrs;
        lua_getfield(L, 1, "Headers");
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            while (lua_next(L, -2)) {
                if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
                    hdrs.emplace_back(lua_tostring(L, -2), lua_tostring(L, -1));
                }
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        net::Response resp = (method == "POST" || method == "PUT" || method == "PATCH")
            ? net::post(url, body, hdrs)
            : net::get(url, hdrs);

        if (!resp.success) {
            lua_pushnil(L);
            lua_pushstring(L, "HTTP request failed");
            return 2;
        }

        // Return {Success=true, StatusCode=..., Body="...", Headers={...}}
        lua_createtable(L, 0, 4);
        lua_pushboolean(L, true);
        lua_setfield(L, -2, "Success");
        lua_pushinteger(L, resp.status_code);
        lua_setfield(L, -2, "StatusCode");
        lua_pushlstring(L, resp.body.data(), resp.body.size());
        lua_setfield(L, -2, "Body");
        lua_createtable(L, 0, 0);
        lua_setfield(L, -2, "Headers");
        return 1;
    }

    // HttpGet(url) — returns body string (or nil on failure).
    inline int HttpGet(lua_State* L) {
        const char* url = luaL_checkstring(L, 1);
        net::Response resp = net::get(url);
        if (!resp.success) {
            lua_pushnil(L);
            lua_pushstring(L, "HTTP request failed");
            return 2;
        }
        lua_pushlstring(L, resp.body.data(), resp.body.size());
        return 1;
    }

    // HttpGetAsync(url) — same as HttpGet but signals via callback. For now
    // we just return the same thing synchronously (real impl would spawn thread).
    inline int HttpGetAsync(lua_State* L) {
        return HttpGet(L);
    }

    // HttpPost(url, body) — POST request.
    inline int HttpPost(lua_State* L) {
        const char* url = luaL_checkstring(L, 1);
        const char* body = luaL_optstring(L, 2, "");
        net::Response resp = net::post(url, body);
        if (!resp.success) {
            lua_pushnil(L);
            lua_pushstring(L, "HTTP request failed");
            return 2;
        }
        lua_pushlstring(L, resp.body.data(), resp.body.size());
        return 1;
    }
}
