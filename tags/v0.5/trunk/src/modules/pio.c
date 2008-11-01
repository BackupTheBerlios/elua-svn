// Module for interfacing wit5~h PIO

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// Local operation masks for all the ports
static pio_type pio_masks[ PLATFORM_IO_PORTS ];

// Helper function: clear all masks
static void pioh_clear_masks()
{
  int i;
  
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    pio_masks[ i ] = 0;
}

// Helper function: pin operations
// Gets the stack index of the first pin and the operation
static int pioh_set_pins( lua_State* L, int stackidx, int op )
{
  int total = lua_gettop( L );
  int i, v, port, pin;
  
  pioh_clear_masks();
  
  // Get all masks
  for( i = stackidx; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );
    port = PLATFORM_IO_GET_PORT( v );
    pin = PLATFORM_IO_GET_PIN( v );
    if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) || !platform_pio_has_pin( port, pin ) )
      return luaL_error( L, "invalid pin" );
    pio_masks[ port ] |= 1 << pin;
  }
  
  // Ask platform to execute the given operation
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    if( pio_masks[ i ] )
      if( !platform_pio_op( i, pio_masks[ i ], op ) )
        return luaL_error( L, "invalid PIO operation" );
  return 0;
}

// Helper function: port operations
// Gets the stack index of the first port and the operation (also the mask)
static int pioh_set_ports( lua_State* L, int stackidx, int op, pio_type mask )
{
  int total = lua_gettop( L );
  int i, v, port;
  
  pioh_clear_masks();
  
  // Get all masks
  for( i = stackidx; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );
    port = PLATFORM_IO_GET_PORT( v );
    if( !PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) )
      return luaL_error( L, "invalid port" );
    pio_masks[ port ] = mask;
  }
  
  // Ask platform to execute the given operation
  for( i = 0; i < PLATFORM_IO_PORTS; i ++ )
    if( pio_masks[ i ] )
      if( !platform_pio_op( i, pio_masks[ i ], op ) )
        return luaL_error( L, "invalid PIO operation" );
  return 0;
}

// Lua: setpin( val, Pin1, Pin2, Pin3 ... )
static int pio_set_pin_state( lua_State* L )
{
  int value;
  
  value = luaL_checkinteger( L, 1 );  
  return pioh_set_pins( L, 2, value ? PLATFORM_IO_PIN_SET : PLATFORM_IO_PIN_CLEAR );
}

// Lua: set( Pin1, Pin2, Pin3 ... )
static int pio_set_pin( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_SET );
}

// Lua: clear( Pin1, Pin2, Pin3 ... )
static int pio_clear_pin( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_CLEAR );
}

// Lua: input( Pin1, Pin2, Pin3 ... )
static int pio_pin_input( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_DIR_INPUT );
}

// Lua: output( Pin1, Pin2, Pin3 ... )
static int pio_pin_output( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_DIR_OUTPUT );
}

// Lua: setport( val, Port1, Port2, ... )
static int pio_set_port( lua_State* L )
{
  int value;
  
  value = luaL_checkinteger( L, 1 );  
  return pioh_set_ports( L, 2, PLATFORM_IO_PORT_SET_VALUE, value );
}

// Lua: value1, value2, ... = getport( Port1, Port2, ... )
static int pio_get_port( lua_State* L )
{
  pio_type value;
  int v, i, port;
  int total = lua_gettop( L );
  
  for( i = 1; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );  
    port = PLATFORM_IO_GET_PORT( v );
    if( !PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) )
      return luaL_error( L, "invalid port" );
    else
    {
      value = platform_pio_op( port, PLATFORM_IO_ALL_PINS, PLATFORM_IO_PORT_GET_VALUE );
      lua_pushinteger( L, value );
    }
  }
  return total;
}

// Lua: port_input( Port1, Port2, ... )
static int pio_port_input( lua_State* L )
{
  return pioh_set_ports( L, 1, PLATFORM_IO_PORT_DIR_INPUT, PLATFORM_IO_ALL_PINS );
}

// Lua: port_output( Port1, Port2, ... )
static int pio_port_output( lua_State* L )
{
  return pioh_set_ports( L, 1, PLATFORM_IO_PORT_DIR_OUTPUT, PLATFORM_IO_ALL_PINS );
}

// Lua: value1, value2, ... = get( Pin1, Pin2 ... )
static int pio_get_pin( lua_State* L )
{
  pio_type value;
  int v, i, port, pin;
  int total = lua_gettop( L );
  
  for( i = 1; i <= total; i ++ )
  {
    v = luaL_checkinteger( L, i );  
    port = PLATFORM_IO_GET_PORT( v );
    pin = PLATFORM_IO_GET_PIN( v );
    if( PLATFORM_IO_IS_PORT( v ) || !platform_pio_has_port( port ) || !platform_pio_has_pin( port, pin ) )
      return luaL_error( L, "invalid pin" );
    else
    {
      value = platform_pio_op( port, 1 << pin, PLATFORM_IO_PIN_GET );
      lua_pushinteger( L, value );
    }
  }
  return total;
}

// Lua: port = port( value )
static int pio_port( lua_State* L )
{
  pio_type value = ( pio_type )luaL_checkinteger( L, 1 );
  
  lua_pushinteger( L, PLATFORM_IO_GET_PORT( value ) );
  return 1;
}

// Lua: pin = pin( value )
static int pio_pin( lua_State* L )
{
  pio_type value = ( pio_type )luaL_checkinteger( L, 1 );
  
  lua_pushinteger( L, PLATFORM_IO_GET_PIN( value ) );
  return 1;
}

// Lua: pullup( pin1, pin2, ... )
static int pio_pin_pullup( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_PULLUP );
}

// Lua: pulldown( pin1, pin2, ... )
static int pio_pin_pulldown( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_PULLDOWN );
}

// Lua: nopull( pin1, pin2, ... )
static int pio_pin_nopull( lua_State* L )
{
  return pioh_set_pins( L, 1, PLATFORM_IO_PIN_NOPULL );
}

// Module function map
static const luaL_reg pio_map[] = 
{
  { "setpin",  pio_set_pin_state },
  { "set", pio_set_pin },
  { "get", pio_get_pin },
  { "clear", pio_clear_pin },
  { "input", pio_pin_input },
  { "output", pio_pin_output },
  { "setport", pio_set_port },
  { "getport", pio_get_port },
  { "port_input", pio_port_input },
  { "port_output", pio_port_output },
  { "pullup", pio_pin_pullup },
  { "pulldown", pio_pin_pulldown },
  { "nopull", pio_pin_nopull },
  { "port", pio_port },
  { "pin", pio_pin },
  { NULL, NULL }
};

// __index metafunction for PIO
// Look for all Px or Px_y keys and return their correct value
static int pio_mt_index( lua_State* L )
{
  const char *key = luaL_checkstring( L ,2 );
  int port = 0xFFFF, pin = 0xFFFF, isport = 0, sz;
  
  if( !key || *key != 'P' )
    return 0;
  if( isupper( key[ 1 ] ) ) // PA, PB, ...
  {
    port = key[ 1 ] - 'A';
    if( key[ 2 ] == '\0' )
      isport = 1;
    else if( key[ 2 ] == '_' )      
      if( sscanf( key + 3, "%d%n", &pin, &sz ) != 1 || sz != strlen( key ) - 3 )
        return 0;      
  }
  else // P0, P1, ...
  {
    // P0, P1, ...
    if( !strchr( key, '_' ) )   // parse port
    {
      if( sscanf( key + 1, "%d%n", &port, &sz ) != 1  || sz != strlen( key ) - 1 )
        return 0;
      isport = 1;
    }
    else    // parse port_pin
      if( sscanf( key + 1, "%d_%d%n", &port, &pin, &sz ) != 2 || sz != strlen( key ) - 1 )
        return 0;
  }
  sz = -1;
  if( isport )
  {
    if( platform_pio_has_port( port ) )
      sz = PLATFORM_IO_ENCODE( port, 0, 1 );
  }
  else
  {
    if( platform_pio_has_port( port ) && platform_pio_has_pin( port, pin ) )
      sz = PLATFORM_IO_ENCODE( port, pin, 0 );
  }
  if( sz == -1 )
    return 0;
  else
  {
    lua_pushinteger( L, sz );
    return 1;
  }
}

// Metatable data
static const luaL_reg pio_mt_map[] =
{
  { "__index", pio_mt_index },
  { NULL, NULL }
};

LUALIB_API int luaopen_pio( lua_State *L )
{
  luaL_register( L, AUXLIB_PIO, pio_map );
  
  // Create and set metatable
  lua_newtable( L );
  luaL_register( L, NULL, pio_mt_map );  
  lua_setmetatable( L, -2 );

  return 1;
}
