// Module for interfacing with platform data

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"

#define MACRO_NAME( x ) MACRO_AGAIN( x )
#define MACRO_AGAIN( x ) #x

// Lua: platform = platform()
static int pd_platform( lua_State* L )
{
  lua_pushstring( L, MACRO_NAME( ELUA_PLATFORM ) );
  return 1;
}

// Lua: cpuname = cpu()
static int pd_cpu( lua_State* L )
{
  lua_pushstring( L, MACRO_NAME( ELUA_CPU ) ); 
  return 1;
}

// Lua: boardname = board()
static int pd_board( lua_State* L )
{
  lua_pushstring( L, MACRO_NAME( ELUA_BOARD ) );
  return 1;
}

// Module function map
static const luaL_reg pd_map[] = 
{
  { "platform",  pd_platform }, 
  { "cpu", pd_cpu },
  { "board", pd_board },
  { NULL, NULL }
};

LUALIB_API int luaopen_pd( lua_State* L )
{
  luaL_register( L, AUXLIB_PD, pd_map );
  return 1;
}
