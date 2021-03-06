// eLua dynamic loader test module

#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "type.h"
#include "lrotable.h"

#define UDL_MOD_NAME          "testmod"
#define UDL_MOD_VER           0x0001

typedef u32 ( *p_udl_get_offset )( int );
extern int udl_get_id();
#define UDL_MOD_FUNC          __attribute__ ((section(".modtext")))
#define UDL_MOD_LTRMAP        __attribute__ ((section(".modltrmap")))
#define UDL_PTR( x )          ( ( x ) + udl_get_offset( udl_id ) )
#define UDL_FUNC_PTR( x )     ( ( u32 )( x ) + udl_get_offset( udl_id ) + 1 )
#define UDL_DECL_ID           int udl_id = udl_get_id()
#define UDL_DECL_GET_OFFSET   p_udl_get_offset udl_get_offset = ( p_udl_get_offset )*( u32* )0x20000008
#define UDL_FUNC_DECL\
  int udl_id = udl_get_id();\
  p_udl_get_offset udl_get_offset = ( p_udl_get_offset )*( u32* )0x20000008

UDL_MOD_FUNC static int test_func( lua_State *L )
{
  UDL_FUNC_DECL;
  const char *parg = luaL_checkstring( L, 1 );

  printf( UDL_PTR( "Called with argument '%s'\n" ), parg );
  lua_pushinteger( L, 666 );
  return 1;
}

UDL_MOD_FUNC static int test_meta( lua_State *L )
{
  lua_pushinteger( L, 777 );
  return 1;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
UDL_MOD_LTRMAP const LUA_REG_TYPE testmod_map[] = 
{
  { LSTRKEY( "testfunc" ), LFUNCVAL( test_func ) },
  { LSTRKEY( "__index" ), LFUNCVAL( test_meta ) },
  { LSTRKEY( "__metatable" ), LROVAL( testmod_map ) },
  { LNILKEY, LNILVAL }
};

UDL_MOD_FUNC int luaopen_testmod( lua_State *L )
{
  return 1;
}

