// eLua platform configuration

#ifndef __EVK1100_CONF_H__
#define __EVK1100_CONF_H__

#include "sdramc.h"
#include "sermux.h"
#include "buf.h"

// *****************************************************************************
// Define here what components you want for this platform

#define BUILD_MMCFS
#define BUILD_XMODEM
#define BUILD_SHELL
#define BUILD_ROMFS
#define BUILD_TERM
#define BUILD_CON_GENERIC
//#define BUILD_RPC
#define BUILD_C_INT_HANDLERS
#define BUILA_LUA_INT_HANDLERS
#define BUILD_RFS
#define BUILD_SERMUX

// *****************************************************************************
// UART/Timer IDs configuration data (used in main.c)

#define CON_UART_ID         ( SERMUX_SERVICE_ID_FIRST + 1 )
//#define CON_UART_ID         0
#define CON_UART_SPEED      115200
#define CON_TIMER_ID        0
#define TERM_LINES          25
#define TERM_COLS           80

// *****************************************************************************
// SPI pins configuration data

#define BOARD_SPI0_SCK_PIN                  AVR32_PIN_PA13
#define BOARD_SPI0_SCK_PIN_FUNCTION         0
#define BOARD_SPI0_MISO_PIN                 AVR32_PIN_PA11
#define BOARD_SPI0_MISO_PIN_FUNCTION        0
#define BOARD_SPI0_MOSI_PIN                 AVR32_PIN_PA12
#define BOARD_SPI0_MOSI_PIN_FUNCTION        0
#define BOARD_SPI0_CS_PIN                   AVR32_PIN_PA10
#define BOARD_SPI0_CS_PIN_FUNCTION          0

#define BOARD_SPI1_SCK_PIN                  AVR32_PIN_PA15
#define BOARD_SPI1_SCK_PIN_FUNCTION         1
#define BOARD_SPI1_MISO_PIN                 AVR32_PIN_PA17
#define BOARD_SPI1_MISO_PIN_FUNCTION        1
#define BOARD_SPI1_MOSI_PIN                 AVR32_PIN_PA16
#define BOARD_SPI1_MOSI_PIN_FUNCTION        1
#define BOARD_SPI1_CS_PIN                   AVR32_PIN_PA18
#define BOARD_SPI1_CS_PIN_FUNCTION          1

// *****************************************************************************
// Auxiliary libraries that will be compiled for this platform

#if defined( ELUA_BOOT_RPC ) && !defined( BUILD_RPC )
#define BUILD_RPC
#endif

#if defined( BUILD_RPC ) 
#define RPCLINE _ROM( AUXLIB_RPC, luaopen_rpc, rpc_map )
#else
#define RPCLINE
#endif

#define LUA_PLATFORM_LIBS_ROM\
  _ROM( AUXLIB_PD, luaopen_pd, pd_map )\
  _ROM( AUXLIB_UART, luaopen_uart, uart_map )\
  _ROM( AUXLIB_PIO, luaopen_pio, pio_map )\
  _ROM( AUXLIB_SPI, luaopen_spi, spi_map )\
  _ROM( AUXLIB_TMR, luaopen_tmr, tmr_map )\
  _ROM( AUXLIB_TERM, luaopen_term, term_map )\
  _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map )\
  _ROM( AUXLIB_ELUA, luaopen_elua, elua_map )\
  RPCLINE\
  _ROM( AUXLIB_BIT, luaopen_bit, bit_map )\
  _ROM( AUXLIB_PACK, luaopen_pack, pack_map )\
  _ROM( LUA_MATHLIBNAME, luaopen_math, math_map )

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       4
#define VTMR_FREQ_HZ          4

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               5
#define NUM_SPI               8
#define NUM_UART              4
#if VTMR_NUM_TIMERS > 0
#define NUM_TIMER             2
#else
#define NUM_TIMER             3
#endif
#define NUM_PWM               0
#define NUM_ADC               0
#define NUM_CAN               0

// RPC boot options
#define RPC_UART_ID           CON_UART_ID
#define RPC_TIMER_ID          CON_TIMER_ID
#define RPC_UART_SPEED        CON_UART_SPEED

// Enable RX buffering on UART
#define BUF_ENABLE_UART
#define CON_BUF_SIZE          BUF_SIZE_128

// SD/MMC Filesystem Setup
#define MMCFS_TICK_HZ     10
#define MMCFS_TICK_MS     ( 1000 / MMCFS_TICK_HZ )
#define MMCFS_SPI_NUM     5
#define MMCFS_CS_PORT     0
#define MMCFS_CS_PIN      SD_MMC_SPI_NPCS_PIN

// CPU frequency (needed by the CPU module, 0 if not used)
#define CPU_FREQUENCY         REQ_CPU_FREQ

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            'A'
// Pins per port configuration:
// #define PIO_PINS_PER_PORT (n) if each port has the same number of pins, or
// #define PIO_PIN_ARRAY { n1, n2, ... } to define pins per port in an array
// Use #define PIO_PINS_PER_PORT 0 if this isn't needed
#define PIO_PIN_ARRAY         { 31, 32, 6, 32, 8 }

// Allocator data: define your free memory zones here in two arrays
// (start address and end address)
#define MEM_START_ADDRESS     { ( void* )end, ( void* )SDRAM }
#define MEM_END_ADDRESS       { ( void* )( 0x10000 - STACK_SIZE_TOTAL - 1 ), ( void* )( SDRAM + SDRAM_SIZE - 1 ) }

#define RFS_BUFFER_SIZE       BUF_SIZE_512
#define RFS_UART_ID           ( SERMUX_SERVICE_ID_FIRST )
#define RFS_TIMER_ID          0
#define RFS_TIMEOUT           100000
#define RFS_UART_SPEED        115200

#define SERMUX_PHYS_ID        0
#define SERMUX_PHYS_SPEED     115200
#define SERMUX_NUM_VUART      2
#define SERMUX_BUFFER_SIZES   { RFS_BUFFER_SIZE, CON_BUF_SIZE }

// Interrupt list
#define INT_UART_RX           ELUA_INT_FIRST_ID
#define INT_ELUA_LAST         INT_UART_RX

#define PLATFORM_CPU_CONSTANTS\
 _C( INT_UART_RX )

// *****************************************************************************
// CPU constants that should be exposed to the eLua "cpu" module

#endif // #ifndef __EVK1100_CONF_H__
