#include "lua_api.h"

LuaVersion luaVersion;
int LUA_REGISTRYINDEX = 0;
int LUA_GLOBALSINDEX = 0;
IMP_LUA_API(lua_gettop);
IMP_LUA_API(lua_settop);
IMP_LUA_API(lua_type);
IMP_LUA_API(lua_typename);
IMP_LUA_API(lua_tolstring);
IMP_LUA_API(lua_toboolean);
IMP_LUA_API(lua_tothread);
IMP_LUA_API(lua_pushnil);
IMP_LUA_API(lua_pushnumber);
IMP_LUA_API(lua_pushlstring);
IMP_LUA_API(lua_pushstring);
IMP_LUA_API(lua_pushcclosure);
IMP_LUA_API(lua_pushboolean);
IMP_LUA_API(lua_pushvalue);
IMP_LUA_API(lua_getfield);
IMP_LUA_API(lua_next);
IMP_LUA_API(lua_createtable);
IMP_LUA_API(lua_setfield);
IMP_LUA_API(lua_setmetatable);
IMP_LUA_API(lua_getstack);
IMP_LUA_API(lua_getinfo);
IMP_LUA_API(lua_getlocal);
IMP_LUA_API(lua_getupvalue);
IMP_LUA_API(lua_setupvalue);
IMP_LUA_API(lua_sethook);
IMP_LUA_API(luaL_loadstring);
IMP_LUA_API(luaL_checklstring);
IMP_LUA_API(luaL_checknumber);
IMP_LUA_API(lua_topointer);
IMP_LUA_API(lua_getmetatable);
IMP_LUA_API(lua_rawget);
IMP_LUA_API(lua_rawset);
IMP_LUA_API(lua_pushlightuserdata);
IMP_LUA_API(lua_touserdata);
IMP_LUA_API(luaL_newstate);
IMP_LUA_API(lua_close);
IMP_LUA_API(lua_pushthread);
IMP_LUA_API(luaL_loadfile);

IMP_LUA_API(lua_rawseti);
IMP_LUA_API(lua_rawgeti);

// luasocket
IMP_LUA_API(luaopen_socket_core);

// tolua
IMP_LUA_API(tolua_loadbuffer);

//51
IMP_LUA_API_E(lua_setfenv);
IMP_LUA_API_E(lua_tointeger);
IMP_LUA_API_E(lua_tonumber);
IMP_LUA_API_E(lua_call);
IMP_LUA_API_E(lua_pcall);
IMP_LUA_API_E(lua_remove);
IMP_LUA_API_E(lua_insert);
IMP_LUA_API_E(luaL_loadbuffer);
IMP_LUA_API_E(lua_load);

// 52 53 54
IMP_LUA_API_E(lua_rawgetp);
IMP_LUA_API_E(lua_rawsetp);
IMP_LUA_API_E(luaL_loadbufferx);

//53
IMP_LUA_API_E(lua_tointegerx);
IMP_LUA_API_E(lua_tonumberx);
IMP_LUA_API_E(lua_getglobal);
IMP_LUA_API_E(lua_setglobal);
IMP_LUA_API_E(lua_callk);
IMP_LUA_API_E(lua_pcallk);
IMP_LUA_API_E(luaL_setfuncs);
IMP_LUA_API_E(lua_absindex);
IMP_LUA_API_E(lua_rotate);
//51 & 52 & 53
IMP_LUA_API_E(lua_newuserdata);
//54
IMP_LUA_API_E(lua_newuserdatauv);
//jit
IMP_LUA_API_E(luaopen_jit);

struct lua_State {};

int lua_setfenv(lua_State* L, int idx)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_JIT)
    {
        return e_lua_setfenv(L, idx);
    }
    return lua_setupvalue(L, idx, 1) != nullptr;
}

int getDebugEvent(lua_Debug* ar)
{
    switch (luaVersion)
    {
        case LuaVersion::LUA_JIT:
        case LuaVersion::LUA_51:
            return ar->u.ar51.event;
        case LuaVersion::LUA_52:
            return ar->u.ar52.event;
        case LuaVersion::LUA_53:
            return ar->u.ar53.event;
        case LuaVersion::LUA_54:
            return ar->u.ar54.event;
        default:
            return 0;
    }
}

int getDebugCurrentLine(lua_Debug* ar)
{
    switch (luaVersion)
    {
        case LuaVersion::LUA_JIT:
        case LuaVersion::LUA_51:
            return ar->u.ar51.currentline;
        case LuaVersion::LUA_52:
            return ar->u.ar52.currentline;
        case LuaVersion::LUA_53:
            return ar->u.ar53.currentline;
        case LuaVersion::LUA_54:
            return ar->u.ar54.currentline;
        default:
            return 0;
    }
}

int getDebugLineDefined(lua_Debug* ar)
{
    switch (luaVersion)
    {
        case LuaVersion::LUA_JIT:
        case LuaVersion::LUA_51:
            return ar->u.ar51.linedefined;
        case LuaVersion::LUA_52:
            return ar->u.ar52.linedefined;
        case LuaVersion::LUA_53:
            return ar->u.ar53.linedefined;
        case LuaVersion::LUA_54:
            return ar->u.ar54.linedefined;
        default:
            return 0;
    }
}

const char* getDebugSource(lua_Debug* ar)
{
    switch (luaVersion)
    {
        case LuaVersion::LUA_JIT:
        case LuaVersion::LUA_51:
            return ar->u.ar51.source;
        case LuaVersion::LUA_52:
            return ar->u.ar52.source;
        case LuaVersion::LUA_53:
            return ar->u.ar53.source;
        case LuaVersion::LUA_54:
            return ar->u.ar54.source;
        default:
            return nullptr;
    }
}

const char* getDebugName(lua_Debug* ar)
{
    switch (luaVersion)
    {
        case LuaVersion::LUA_JIT:
        case LuaVersion::LUA_51:
            return ar->u.ar51.name;
        case LuaVersion::LUA_52:
            return ar->u.ar52.name;
        case LuaVersion::LUA_53:
            return ar->u.ar53.name;
        case LuaVersion::LUA_54:
            return ar->u.ar54.name;
        default:
            return nullptr;
    }
}

lua_Integer lua_tointeger(lua_State* L, int idx)
{
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_lua_tointegerx(L, idx, nullptr);
    }
    return e_lua_tointeger(L, idx);
}

lua_Number lua_tonumber(lua_State* L, int idx)
{
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_lua_tonumberx(L, idx, nullptr);
    }
    return e_lua_tonumber(L, idx);
}

int lua_getglobal(lua_State* L, const char* name)
{
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_lua_getglobal(L, name);
    }
    return lua_getfield(L, LUA_GLOBALSINDEX, name);
}

void lua_setglobal(lua_State* L, const char* name)
{
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_lua_setglobal(L, name);
    }
    return lua_setfield(L, LUA_GLOBALSINDEX, name);
}

void lua_call(lua_State* L, int nargs, int nresults)
{
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_lua_callk(L, nargs, nresults, 0, nullptr);
    }
    return (void)e_lua_call(L, nargs, nresults);
}

int lua_pcall(lua_State* L, int nargs, int nresults, int errfunc)
{
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_lua_pcallk(L, nargs, nresults, errfunc, 0, nullptr);
    }
    return e_lua_pcall(L, nargs, nresults, errfunc);
}

int luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char* name)
{
    if (tolua_loadbuffer != NULL)
    {
        return tolua_loadbuffer(L, buff, int(sz), name);
    }
    if (luaVersion > LuaVersion::LUA_51)
    {
        return e_luaL_loadbufferx(L, buff, sz, name, NULL);
    }
    return e_luaL_loadbuffer(L, buff, sz, name);
}

void luaL_setfuncs(lua_State* L, const luaL_Reg* l, int nup)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_JIT)
    {
        for (; l->name != nullptr; l++)
        {
            for (int i = 0; i < nup; i++)
                lua_pushvalue(L, -nup);
            lua_pushcclosure(L, l->func, nup);
            lua_setfield(L, -(nup + 2), l->name);
        }
    }
    else e_luaL_setfuncs(L, l, nup);
}

int lua_upvalueindex(int i)
{
    if (luaVersion == LuaVersion::LUA_54)
        return -1001000 - i;
    if (luaVersion == LuaVersion::LUA_53)
        return -1001000 - i;
    if (luaVersion == LuaVersion::LUA_52)
        return -1001000 - i;
    return -10002 - i;
}

int lua_absindex(lua_State* L, int idx)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_JIT)
    {
        if (idx > 0)
        {
            return idx;
        }
        return lua_gettop(L) + idx + 1;
    }
    return e_lua_absindex(L, idx);
}

void lua_remove(lua_State* L, int idx)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_52 || luaVersion == LuaVersion::LUA_JIT)
    {
        e_lua_remove(L, idx);
    }
    else
    {
        e_lua_rotate(L, (idx), -1);
        lua_pop(L, 1);
    }
}

void* lua_newuserdata(lua_State* L, int size)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_52 || luaVersion == LuaVersion::LUA_53 || luaVersion == LuaVersion::LUA_JIT)
    {
        return e_lua_newuserdata(L, size);
    }
    else
    {
        return e_lua_newuserdatauv(L, size, 1);
    }
}

void lua_pushglobaltable(lua_State* L)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_JIT)
    {
        lua_pushvalue(L, LUA_GLOBALSINDEX);
    }
    else
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_GLOBALSINDEX);
    }
}

int lua_rawgetp(lua_State* L, int idx, const void* p)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_JIT)
    {
        if (idx < 0)
        {
            idx += lua_gettop(L) + 1;
        }
        lua_pushlightuserdata(L, (void*)p);
        return lua_rawget(L, idx);
    }
    else
    {
        return e_lua_rawgetp(L, idx, p);
    }
}

void lua_rawsetp(lua_State* L, int idx, const void* p)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_JIT)
    {
        if (idx < 0)
        {
            idx += lua_gettop(L) + 1;
        }
        lua_pushlightuserdata(L, (void*)p);
        lua_insert(L, -2);
        lua_rawset(L, idx);
    }
    else
    {
        return e_lua_rawsetp(L, idx, p);
    }
}

void lua_insert(lua_State* L, int idx)
{
    if (luaVersion == LuaVersion::LUA_51 || luaVersion == LuaVersion::LUA_52 || luaVersion == LuaVersion::LUA_JIT)
    {
        return e_lua_insert(L, idx);
    }
    else
    {
        return e_lua_rotate(L, idx, 1);
    }
}

bool SetupLuaAPI(void *handle)
{
    LOAD_LUA_API(lua_gettop)
    LOAD_LUA_API(lua_settop);
    LOAD_LUA_API(lua_type);
    LOAD_LUA_API(lua_typename);
    LOAD_LUA_API(lua_tolstring);
    LOAD_LUA_API(lua_toboolean);
    LOAD_LUA_API(lua_tothread);
    LOAD_LUA_API(lua_pushnil);
    LOAD_LUA_API(lua_pushnumber);
    LOAD_LUA_API(lua_pushlstring);
    LOAD_LUA_API(lua_pushstring);
    LOAD_LUA_API(lua_pushcclosure);
    LOAD_LUA_API(lua_pushboolean);
    LOAD_LUA_API(lua_pushvalue);
    LOAD_LUA_API(lua_getfield);
    LOAD_LUA_API(lua_next);
    LOAD_LUA_API(lua_createtable);
    LOAD_LUA_API(lua_setfield);
    LOAD_LUA_API(lua_setmetatable);
    LOAD_LUA_API(lua_getstack);
    LOAD_LUA_API(lua_getinfo);
    LOAD_LUA_API(lua_getlocal);
    LOAD_LUA_API(lua_getupvalue);
    LOAD_LUA_API(lua_setupvalue);
    LOAD_LUA_API(lua_sethook);
    LOAD_LUA_API(luaL_loadstring);
    LOAD_LUA_API(luaL_checklstring);
    LOAD_LUA_API(luaL_checknumber);
    LOAD_LUA_API(lua_topointer);
    LOAD_LUA_API(lua_getmetatable);
    LOAD_LUA_API(lua_rawget);
    LOAD_LUA_API(lua_rawset);
    LOAD_LUA_API(lua_pushlightuserdata);
    LOAD_LUA_API(lua_touserdata);
    LOAD_LUA_API(luaL_newstate);
    LOAD_LUA_API(lua_close);
    LOAD_LUA_API(lua_pushthread);
    LOAD_LUA_API(luaL_loadfile);

    // luasocket
    LOAD_LUA_API(luaopen_socket_core);

    //tolua
    LOAD_LUA_API(tolua_loadbuffer);

    LOAD_LUA_API(lua_rawseti);
    LOAD_LUA_API(lua_rawgeti);
    //51
    LOAD_LUA_API_E(lua_setfenv);
    LOAD_LUA_API_E(lua_tointeger);
    LOAD_LUA_API_E(lua_tonumber);
    LOAD_LUA_API_E(lua_call);
    LOAD_LUA_API_E(lua_pcall);
    LOAD_LUA_API_E(luaL_loadbuffer);
    LOAD_LUA_API_E(lua_load);

    //51 & 52
    LOAD_LUA_API_E(lua_remove);
    //52 & 53 & 54
    LOAD_LUA_API_E(lua_tointegerx);
    LOAD_LUA_API_E(lua_tonumberx);
    LOAD_LUA_API_E(lua_getglobal);
    LOAD_LUA_API_E(lua_setglobal);
    LOAD_LUA_API_E(lua_callk);
    LOAD_LUA_API_E(lua_pcallk);
    LOAD_LUA_API_E(luaL_setfuncs);
    LOAD_LUA_API_E(lua_absindex);
    LOAD_LUA_API_E(lua_rawgetp);
    LOAD_LUA_API_E(lua_rawsetp);
    LOAD_LUA_API_E(luaL_loadbufferx);

    // 51 & 52 & 53
    LOAD_LUA_API_E(lua_newuserdata);
    //53
    LOAD_LUA_API_E(lua_rotate);
    //54
    LOAD_LUA_API_E(lua_newuserdatauv);

    //jit
    LOAD_LUA_API_E(luaopen_jit);


    // 51 & 52 & 53
    LOAD_LUA_API_E(lua_newuserdata);
    //53
    LOAD_LUA_API_E(lua_rotate);
    //54
    LOAD_LUA_API_E(lua_newuserdatauv);

    //jit
    LOAD_LUA_API_E(luaopen_jit);

    if (e_lua_newuserdatauv)
    {
        luaVersion = LuaVersion::LUA_54;
        LUA_REGISTRYINDEX = -1001000;
        LUA_GLOBALSINDEX = 2;
    }
    else if (e_lua_rotate)
    {
        luaVersion = LuaVersion::LUA_53;
        LUA_REGISTRYINDEX = -1001000;
        LUA_GLOBALSINDEX = 2;
    }
    else if (e_lua_callk)
    {
        luaVersion = LuaVersion::LUA_52;
        //todo
        LUA_REGISTRYINDEX = -1001000;
        LUA_GLOBALSINDEX = 2;
    }
    else if (e_luaopen_jit)
    {
        luaVersion = LuaVersion::LUA_JIT;
        LUA_REGISTRYINDEX = -10000;
        LUA_GLOBALSINDEX = -10002;
    }
    else
    {
        luaVersion = LuaVersion::LUA_51;
        LUA_REGISTRYINDEX = -10000;
        LUA_GLOBALSINDEX = -10002;
    }
    if (luaVersion > LuaVersion::LUA_JIT) {
        LOGW("lua version: %d\n", luaVersion);
    }
    else
    {
        LOGW("lua version: jit\n");
    }
    return true;
}