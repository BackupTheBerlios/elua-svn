// eLua platform configuration

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "stacks.h"
#include "sysctl.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
#define BUILD_UIP
#define BUILD_DHCPC
#define BUILD_DNS
#define BUILD_CON_GENERIC
#define BUILD_ADC
//#define BUILD_CON_TCP

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID           0
#define CON_UART_SPEED        115200
#define CON_TIMER_ID          0
#define TERM_LINES            25
#define TERM_COLS             80

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

// The name of the platform specific libs table
#ifdef ENABLE_DISP
#define PS_LIB_TABLE_NAME   "lm3s"
#endif

#ifdef FORLM3S6918
#define PWMLINE
#else
#define PWMLINE  _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map )
#endif

#ifdef BUILD_UIP
#define NETLINE  _ROM( AUXLIB_NET, luaopen_net, net_map )
#else
#define NETLINE
#endif

#ifdef PS_LIB_TABLE_NAME
#define PLATLINE _ROM( PS_LIB_TABLE_NAME, luaopen_platform, platform_map )
#else
#define PLATLINE
#endif

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_SPI, luaopen_spi, spi_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  PWMLINE\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_BITARRAY, luaopen_bitarray, bitarray_map )\
  NETLINE\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  _ROM( AUXLIB_ADC, luaopen_adc, adc_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )\
  PLATLINE

// *****************************************************************************
// Configuration data

// Static TCP/IP configuration
#define ELUA_CONF_IPADDR0     192
#define ELUA_CONF_IPADDR1     168
#define ELUA_CONF_IPADDR2     1
#define ELUA_CONF_IPADDR3     218

#define ELUA_CONF_NETMASK0    255
#define ELUA_CONF_NETMASK1    255
#define ELUA_CONF_NETMASK2    255
#define ELUA_CONF_NETMASK3    0

#define ELUA_CONF_DEFGW0      192
#define ELUA_CONF_DEFGW1      168
#define ELUA_CONF_DEFGW2      1
#define ELUA_CONF_DEFGW3      1

#define ELUA_CONF_DNS0        192
#define ELUA_CONF_DNS1        168
#define ELUA_CONF_DNS2        1
#define ELUA_CONF_DNS3        1

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               7
#define NUM_SPI               1
#ifdef FORLM3S6965
  #define NUM_UART            3
#else
  #define NUM_UART            2
#endif
#define NUM_TIMER             4
#ifndef FORLM3S6918
  #define NUM_PWM             6
#else
  #define NUM_PWM             0
#endif  
#define NUM_ADC               4

// Enable RX buffering on UART
//#define BUF_ENABLE_UART
//#define CON_BUF_SIZE          BUF_SIZE_128

// ADC Configuration Params
#define ADC_BIT_RESOLUTION    10
#define BUF_ENABLE_ADC
#define ADC_BUF_SIZE          BUF_SIZE_2

// These should be adjusted to support multiple ADC devices
#define ADC_TIMER_FIRST_ID    0
#define ADC_NUM_TIMERS        NUM_TIMER  


// CPU frequency (needed by the CPU module, 0 if not used)
#define CPU_FREQUENCY         SysCtlClockGet()

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array 
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 8, 8, 8, 8, 4, 4, 2 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define MEM_START_ADDRESS     { ( void* )end }
#define MEM_END_ADDRESS       { ( void* )( SRAM_BASE + 0x10000 - STACK_SIZE_TOTAL - 1 ) }

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#include "hw_ints.h"

#define PLATFORM_CPU_CONSTANTS\
  _C( INT_GPIOA ),\
  _C( INT_GPIOB ),\
  _C( INT_GPIOC ),\
  _C( INT_GPIOD ),\
  _C( INT_GPIOE ),\
  _C( INT_UART0 ),\
  _C( INT_UART1 ),\
  _C( INT_SSI0 ),\
  _C( INT_I2C0 ),\
  _C( INT_PWM_FAULT ),\
  _C( INT_PWM0 ),\
  _C( INT_PWM1 ),\
  _C( INT_PWM2 ),\
  _C( INT_QEI0 ),\
  _C( INT_ADC0 ),\
  _C( INT_ADC1 ),\
  _C( INT_ADC2 ),\
  _C( INT_ADC3 ),\
  _C( INT_WATCHDOG ),\
  _C( INT_TIMER0A ),\
  _C( INT_TIMER0B ),\
  _C( INT_TIMER1A ),\
  _C( INT_TIMER1B ),\
  _C( INT_TIMER2A ),\
  _C( INT_TIMER2B ),\
  _C( INT_COMP0 ),\
  _C( INT_COMP1 ),\
  _C( INT_COMP2 ),\
  _C( INT_SYSCTL ),\
  _C( INT_FLASH ),\
  _C( INT_GPIOF ),\
  _C( INT_GPIOG ),\
  _C( INT_GPIOH ),\
  _C( INT_UART2 ),\
  _C( INT_SSI1 ),\
  _C( INT_TIMER3A ),\
  _C( INT_TIMER3B ),\
  _C( INT_I2C1 ),\
  _C( INT_QEI1 ),\
  _C( INT_CAN0 ),\
  _C( INT_CAN1 ),\
  _C( INT_CAN2 ),\
  _C( INT_ETH ),\
  _C( INT_HIBERNATE ),\
  _C( INT_USB0 ),\
  _C( INT_PWM3 ),\
  _C( INT_UDMA ),\
  _C( INT_UDMAERR )
  
#endif // #ifndef __PLATFORM_CONF_H__
