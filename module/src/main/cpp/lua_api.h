#ifndef ZYGISK_IL2CPPDUMPER_LUA_API_H
#define ZYGISK_IL2CPPDUMPER_LUA_API_H

#include <dlfcn.h>
#include "xdl/include/xdl.h"
#include "log.h"

struct lua_State;

// lua version
enum class LuaVersion {
    UNKNOWN,
    LUA_JIT = 50,
    LUA_51 = 51,
    LUA_52 = 52,
    LUA_53 = 53,
    LUA_54 = 54,
};

extern LuaVersion luaVersion;
typedef double lua_Number;
// 不同的lua版本定义是不同的，不同的平台定义也是不同的
// x64 上最终都是long long
// x86 上lua5.3以上是long long，lua5.2以下是ptrdiff_t通常是int
// 传参时
typedef long long lua_Integer;
typedef ptrdiff_t lua_KContext;

/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
@@ of a function in debug information.
** CHANGE it if you want a different size.
*/
#define LUA_IDSIZE	1024
/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#define LUA_MULTRET	(-1)

/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRGCMM	5
#define LUA_ERRERR	6


// typedef struct lua_State lua_State;


/*
** basic types
*/
#define LUA_TNONE		(-1)
#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTAGS		9

/*
** Type for C functions registered with Lua
*/
typedef int(*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
typedef int(*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int(*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);

//debug
/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
// LUA 5.1
#define LUA_HOOKTAILRET 4
// LUA 5.2+
#define LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET		(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

struct lua_Debug_51 {
    int event;
    const char *name;	/* (n) */
    const char *namewhat;	/* (n) `global', `local', `field', `method' */
    const char *what;	/* (S) `Lua', `C', `main', `tail' */
    const char *source;	/* (S) */
    int currentline;	/* (l) */
    int nups;		/* (u) number of upvalues */
    int linedefined;	/* (S) */
    int lastlinedefined;	/* (S) */
    char short_src[LUA_IDSIZE]; /* (S) */
    /* private part */
    int i_ci;  /* active function */
};

struct lua_Debug_52 {
    int event;
    const char *name;	/* (n) */
    const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
    const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
    const char *source;	/* (S) */
    int currentline;	/* (l) */
    int linedefined;	/* (S) */
    int lastlinedefined;	/* (S) */
    unsigned char nups;	/* (u) number of upvalues */
    unsigned char nparams;/* (u) number of parameters */
    char isvararg;        /* (u) */
    char istailcall;	/* (t) */
    char short_src[LUA_IDSIZE]; /* (S) */
    /* private part */
    struct CallInfo *i_ci;  /* active function */
};

struct lua_Debug_53 {
    int event;
    const char *name;	/* (n) */
    const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
    const char *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
    const char *source;	/* (S) */
    int currentline;	/* (l) */
    int linedefined;	/* (S) */
    int lastlinedefined;	/* (S) */
    unsigned char nups;	/* (u) number of upvalues */
    unsigned char nparams;/* (u) number of parameters */
    char isvararg;        /* (u) */
    char istailcall;	/* (t) */
    char short_src[LUA_IDSIZE]; /* (S) */
    /* private part */
    struct CallInfo *i_ci;  /* active function */
};

struct lua_Debug_54 {
    int event;
    const char* name;	/* (n) */
    const char* namewhat;	/* (n) 'global', 'local', 'field', 'method' */
    const char* what;	/* (S) 'Lua', 'C', 'main', 'tail' */
    const char* source;	/* (S) */
    size_t srclen;	/* (S) */
    int currentline;	/* (l) */
    int linedefined;	/* (S) */
    int lastlinedefined;	/* (S) */
    unsigned char nups;	/* (u) number of upvalues */
    unsigned char nparams;/* (u) number of parameters */
    char isvararg;        /* (u) */
    char istailcall;	/* (t) */
    unsigned short ftransfer;   /* (r) index of first value transferred */
    unsigned short ntransfer;   /* (r) number of transferred values */
    char short_src[LUA_IDSIZE]; /* (S) */
    /* private part */
    struct CallInfo* i_ci;  /* active function */
};

struct lua_Debug {
    union {
        lua_Debug_51 ar51;
        lua_Debug_52 ar52;
        lua_Debug_53 ar53;
        lua_Debug_54 ar54;
    } u;
};

int getDebugEvent(lua_Debug* ar);
int getDebugCurrentLine(lua_Debug* ar);
int getDebugLineDefined(lua_Debug* ar);
const char* getDebugSource(lua_Debug* ar);
const char* getDebugName(lua_Debug* ar);



/* Functions to be called by the debugger in specific events */
typedef void(*lua_Hook) (lua_State *L, lua_Debug *ar);

//#include "lauxlib.h"
typedef struct luaL_Reg {
    const char *name;
    lua_CFunction func;
} luaL_Reg;

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))

#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)
#define lua_pop(L,n)		lua_settop(L, -(n)-1)
#define lua_newtable(L)		lua_createtable(L, 0, 0)
#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))
#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)
////////////////////////////////////////////////////////////////////////////////////////
extern int LUA_REGISTRYINDEX;
extern int LUA_GLOBALSINDEX;

#define DEF_LUA_API(r,n,p)  typedef r (*dll_##n)p; extern dll_##n n;
#define IMP_LUA_API(n) dll_##n n = NULL

#define LOAD_LUA_API(n) n= (dll_##n)xdl_sym(handle, #n, nullptr);     \
if(!n) {                                   \
    LOGW("api not found %s", #n);          \
}                                          \

#define DEF_LUA_API_E(r,n,p)  typedef r (*dll_##n)p; extern dll_##n e_##n;

#define IMP_LUA_API_E(n) dll_##n e_##n

#define LOAD_LUA_API_E(n) e_##n= (dll_##n)xdl_sym(handle, #n, nullptr);     \
if(!e_##n) {                                   \
    LOGW("api not found %s", #n);          \
}                                          \

DEF_LUA_API(int, lua_gettop, (lua_State* L))
DEF_LUA_API(void, lua_settop, (lua_State* L, int idx))
DEF_LUA_API(int, lua_type, (lua_State* L, int idx));
DEF_LUA_API(const char*, lua_typename, (lua_State* L, int tp));
DEF_LUA_API(const char*, lua_tolstring, (lua_State* L, int idx, size_t* len));
DEF_LUA_API(int, lua_toboolean, (lua_State* L, int idx));
DEF_LUA_API(lua_State*, lua_tothread, (lua_State* L, int idx));
DEF_LUA_API(void, lua_pushnil, (lua_State* L));
DEF_LUA_API(void, lua_pushnumber, (lua_State* L, lua_Number n));
DEF_LUA_API(const char*, lua_pushlstring, (lua_State* L, const char* s, size_t len));
DEF_LUA_API(const char*, lua_pushstring, (lua_State* L, const char* s));
DEF_LUA_API(void, lua_pushcclosure, (lua_State* L, lua_CFunction fn, int n));
DEF_LUA_API(void, lua_pushboolean, (lua_State* L, int b));
DEF_LUA_API(void, lua_pushvalue, (lua_State* L, int idx));
DEF_LUA_API(int, lua_getfield, (lua_State* L, int idx, const char* k));
DEF_LUA_API(int, lua_next, (lua_State* L, int idx));
DEF_LUA_API(void, lua_createtable, (lua_State* L, int narr, int nrec));
DEF_LUA_API(void, lua_setfield, (lua_State* L, int idx, const char* k));
DEF_LUA_API(int, lua_setmetatable, (lua_State* L, int objindex));
DEF_LUA_API(int, lua_getstack, (lua_State* L, int level, lua_Debug* ar));
DEF_LUA_API(int, lua_getinfo, (lua_State* L, const char* what, lua_Debug* ar));
DEF_LUA_API(const char*, lua_getlocal, (lua_State* L, const lua_Debug* ar, int n));
DEF_LUA_API(const char*, lua_getupvalue, (lua_State* L, int funcindex, int n));
DEF_LUA_API(const char*, lua_setupvalue, (lua_State* L, int funcindex, int n));
DEF_LUA_API(void, lua_sethook, (lua_State* L, lua_Hook func, int mask, int count));
DEF_LUA_API(int, luaL_loadstring, (lua_State* L, const char* s));
DEF_LUA_API(const char*, luaL_checklstring, (lua_State* L, int arg, size_t* l));
DEF_LUA_API(lua_Number, luaL_checknumber, (lua_State* L, int arg));
DEF_LUA_API(void*, lua_topointer, (lua_State* L, int index));
DEF_LUA_API(int, lua_getmetatable, (lua_State *L, int objindex));
DEF_LUA_API(int, lua_rawget, (lua_State *L, int idx));
DEF_LUA_API(void, lua_rawset, (lua_State *L, int idx));
DEF_LUA_API(void, lua_pushlightuserdata, (lua_State *L, void *p));
DEF_LUA_API(void*, lua_touserdata, (lua_State *L, int idx));
DEF_LUA_API(void*, lua_rawseti, (lua_State *L, int idx, lua_Integer n));
DEF_LUA_API(void*, lua_rawgeti, (lua_State *L, int idx, lua_Integer n));
DEF_LUA_API(void*, luaL_newstate, (void));
DEF_LUA_API(void, lua_close, (lua_State* L));
DEF_LUA_API(int, lua_pushthread, (lua_State* L));
DEF_LUA_API(int, luaL_loadfile, (lua_State *L, const char *filename));

// luasocket
DEF_LUA_API(int, luaopen_socket_core, (lua_State *L));

// tolua
DEF_LUA_API(int, tolua_loadbuffer, (lua_State *L, const char *buff, int32_t sz, const char *name));

//51
DEF_LUA_API_E(int, lua_setfenv, (lua_State* L, int idx));
DEF_LUA_API_E(lua_Integer, lua_tointeger, (lua_State* L, int idx));
DEF_LUA_API_E(lua_Number, lua_tonumber, (lua_State *L, int idx));
DEF_LUA_API_E(int, lua_call, (lua_State* L, int nargs, int nresults));
DEF_LUA_API_E(int, lua_pcall, (lua_State* L, int nargs, int nresults, int errfunc));
DEF_LUA_API_E(void, lua_insert, (lua_State* L, int idx));
DEF_LUA_API_E(int, luaL_loadbuffer, (lua_State *L, const char *buff, size_t sz, const char *name));
DEF_LUA_API_E(int , lua_load, (lua_State *L, lua_Reader reader, void *data, const char *chunkname));

//51 & 52
DEF_LUA_API_E(void, lua_remove, (lua_State *L, int idx));
//52 & 53 & 54
DEF_LUA_API_E(int, luaL_loadbufferx, (lua_State *L, const char *buff, size_t sz, const char *name, const char *mode));
DEF_LUA_API_E(lua_Integer, lua_tointegerx, (lua_State* L, int idx, int* isnum));
DEF_LUA_API_E(lua_Number, lua_tonumberx, (lua_State *L, int idx, int *isnum));
DEF_LUA_API_E(int, lua_getglobal, (lua_State* L, const char* name));
DEF_LUA_API_E(void, lua_setglobal, (lua_State* L, const char* name));
DEF_LUA_API_E(void, lua_callk, (lua_State* L, int nargs, int nresults, lua_KContext ctx, lua_KFunction k));
DEF_LUA_API_E(int, lua_pcallk, (lua_State* L, int nargs, int nresults, int errfunc, lua_KContext ctx, lua_KFunction k));
DEF_LUA_API_E(void, luaL_setfuncs, (lua_State* L, const luaL_Reg* l, int nup));
DEF_LUA_API_E(int, lua_absindex, (lua_State *L, int idx));
DEF_LUA_API_E(int, lua_rawgetp, (lua_State *L, int idx, const void* p));
DEF_LUA_API_E(void, lua_rawsetp, (lua_State* L, int idx, const void* p));
DEF_LUA_API_E(int, lua_pushthread, (lua_State* L));

// 51 & 52 & 53
DEF_LUA_API_E(void*, lua_newuserdata, (lua_State* L, int size));
//53
DEF_LUA_API_E(void, lua_rotate, (lua_State *L, int idx, int n));

//54
DEF_LUA_API_E(void*, lua_newuserdatauv, (lua_State* L, int size, int nuvalue));

//jit
DEF_LUA_API_E(int, luaopen_jit, (lua_State* L));

lua_Integer lua_tointeger(lua_State* L, int idx);
lua_Number lua_tonumber(lua_State* L, int idx);
int lua_setfenv(lua_State* L, int idx);
int lua_getglobal(lua_State* L, const char* name);
void lua_setglobal(lua_State* L, const char* name);
int lua_pcall(lua_State* L, int nargs, int nresults, int errfunc);
int lua_upvalueindex(int i);
int lua_absindex(lua_State *L, int idx);
void lua_call(lua_State* L, int nargs, int nresults);
void luaL_setfuncs(lua_State* L, const luaL_Reg* l, int nup);
void lua_remove(lua_State *L, int idx);
void* lua_newuserdata(lua_State* L, int size);
void lua_pushglobaltable(lua_State* L);
int lua_rawgetp(lua_State* L, int idx, const void* p);
void lua_rawsetp(lua_State* L, int idx, const void* p);
void lua_insert(lua_State* L, int idx);
int luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name);

bool SetupLuaAPI(void *handle);
#endif