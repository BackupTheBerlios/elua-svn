// Auxiliary Lua modules. All of them are declared here, then each platform
// decides what module(s) to register in the "platform_libs.h" file

#ifndef __AUXMODS_H__
#define __AUXMODS_H__

#include "lua.h"

#define AUXLIB_PIO      "pio"
LUALIB_API int ( luaopen_pio )( lua_State *L );

#define AUXLIB_SPI      "spi"
LUALIB_API int ( luaopen_spi )( lua_State *L );

#define AUXLIB_TMR      "tmr"
LUALIB_API int ( luaopen_tmr )( lua_State *L );

#define AUXLIB_PD       "pd"
LUALIB_API int ( luaopen_pd )( lua_State *L );

#define AUXLIB_UART     "uart"
LUALIB_API int ( luaopen_uart )( lua_State *L );

#define AUXLIB_TERM     "term"
LUALIB_API int ( luaopen_term )( lua_State *L );

#define AUXLIB_PWM      "pwm"
LUALIB_API int ( luaopen_pwm )( lua_State *L );

#define AUXLIB_PACK     "pack"
LUALIB_API int ( luaopen_pack )( lua_State *L );

#define AUXLIB_BIT      "bit"
LUALIB_API int ( luaopen_bit )( lua_State *L );

#define AUXLIB_NET      "net"
LUALIB_API int ( luaopen_net )( lua_State *L );

#define AUXLIB_CPU      "cpu"
LUALIB_API int ( luaopen_cpu )( lua_State* L );

#define AUXLIB_ADC      "adc"
LUALIB_API int ( luaopen_adc )( lua_State *L );

#define AUXLIB_LUARPC   "rpc"
LUALIB_API int ( luaopen_luarpc )( lua_State *L );

// Helper macros

#define MOD_CHECK_ID( mod, id )\
  if( !platform_ ## mod ## _exists( id ) )\
    return luaL_error( L, #mod" %d does not exist", ( unsigned )id )

#define MOD_CHECK_RES_ID( mod, id, resmod, resid )\
  if( !platform_ ## mod ## _check_ ## resmod ## _id( id, resid ) )\
    return luaL_error( L, #resmod" %d not valid with " #mod " %d", ( unsigned )resid, ( unsigned )id )
    
#endif
