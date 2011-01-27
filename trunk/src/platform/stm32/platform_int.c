// STM32 interrupt support

// Generic headers
#include "platform.h"
#include "platform_conf.h"
#include "elua_int.h"
#include "common.h"

// Platform-specific headers
#include "stm32f10x.h"

// ****************************************************************************
// Interrupt handlers

extern USART_TypeDef *const stm32_usart[];

static void all_usart_irqhandler( int resnum )
{
  int temp;

  temp = USART_GetFlagStatus( stm32_usart[ resnum ], USART_FLAG_ORE );
  cmn_int_handler( INT_UART_RX, resnum );
  if( temp == SET )
    for( temp = 0; temp < 10; temp ++ )
      platform_s_uart_send( resnum, '@' );
}

void USART1_IRQHandler()
{
  all_usart_irqhandler( 0 );
}

void USART2_IRQHandler()
{
  all_usart_irqhandler( 1 );
}

void USART3_IRQHandler()
{
  all_usart_irqhandler( 2 );
}

void UART4_IRQHandler()
{
  all_usart_irqhandler( 3 );
}

void UART5_IRQHandler()
{
  all_usart_irqhandler( 4 );
}

// ****************************************************************************
// External interrupt handlers

static const u32 exti_line[] = { EXTI_Line0, EXTI_Line1, EXTI_Line2, EXTI_Line3, EXTI_Line4, EXTI_Line5, EXTI_Line6, EXTI_Line7, EXTI_Line8, EXTI_Line9, EXTI_Line10, EXTI_Line12, EXTI_Line13, EXTI_Line14, EXTI_Line15 };

static u16 exti_line_to_gpio( u32 line )
{
  return PLATFORM_IO_ENCODE( ( AFIO->EXTICR[line >> 0x02] >> (0x04 * ( line & 0x03 ) ) ) & 0x07, line, PLATFORM_IO_ENC_PIN );
}

// Convert a GPIO ID to a EXINT number
static int exint_gpio_to_src( pio_type piodata )
{
  u16 pin = PLATFORM_IO_GET_PIN( piodata );
  return pin;
}

static void all_exti_irqhandler( int line )
{
  u16 v, port, pin;
  
  v = exti_line_to_gpio( line );	
  port = PLATFORM_IO_GET_PORT( v );
  pin = PLATFORM_IO_GET_PIN( v );

  if( EXTI->RTSR & (1 << line ) && platform_pio_op( port, 1 << pin, PLATFORM_IO_PIN_GET ) )
    cmn_int_handler( INT_GPIO_POSEDGE, v );
  if( EXTI->FTSR & (1 << line ) && ( platform_pio_op( port, 1 << pin, PLATFORM_IO_PIN_GET ) == 0 ) )
    cmn_int_handler( INT_GPIO_NEGEDGE, v );

  EXTI_ClearITPendingBit( exti_line[ line ] );
}

void EXTI0_IRQHandler()
{
  all_exti_irqhandler( 0 );
}

void EXTI1_IRQHandler()
{
  all_exti_irqhandler( 1 );
}

void EXTI2_IRQHandler()
{
  all_exti_irqhandler( 2 );
}

void EXTI3_IRQHandler()
{
  all_exti_irqhandler( 3 );
}

void EXTI4_IRQHandler()
{
  all_exti_irqhandler( 4 );
}

void EXTI9_5_IRQHandler()
{
  int i;
  for( i = 5; i < 10; i++ )
  {
    if( EXTI_GetITStatus( exti_line[ i ] ) != RESET )
      all_exti_irqhandler( i );
  }
}

void EXTI15_10_IRQHandler()
{
  int i;
  for( i = 10; i < 16; i++ )
  {
    if( EXTI_GetITStatus( exti_line[ i ] ) != RESET )
      all_exti_irqhandler( i );
  }
}

// ****************************************************************************
// GPIO helper functions

static int gpioh_get_int_status( elua_int_id id, elua_int_resnum resnum )
{
  if ( EXTI->IMR & ( 1 << exint_gpio_to_src( resnum ) ) )
    return 1;
  else
    return 0;
}

static int gpioh_set_int_status( elua_int_id id, elua_int_resnum resnum, int status )
{
  int prev = gpioh_get_int_status( id, resnum );
  EXTI_InitTypeDef exti_init_struct;
  
  GPIO_EXTILineConfig( PLATFORM_IO_GET_PORT( resnum ), PLATFORM_IO_GET_PIN( resnum ) );

  EXTI_StructInit(&exti_init_struct);
  exti_init_struct.EXTI_Line = exti_line[ exint_gpio_to_src( resnum ) ];
  exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;
  exti_init_struct.EXTI_Trigger = id == INT_GPIO_POSEDGE ? EXTI_Trigger_Rising : EXTI_Trigger_Falling;
  exti_init_struct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&exti_init_struct);

  EXTI_ClearITPendingBit( exti_line[ exint_gpio_to_src( resnum ) ] );
  return prev;
}

static int gpioh_get_int_flag( elua_int_id id, elua_int_resnum resnum, int clear )
{
  int flag = 0;
  u32 mask =  1 << exint_gpio_to_src( resnum );

  if( EXTI_GetFlagStatus( exti_line[ exint_gpio_to_src( resnum ) ] ) == SET )
  {
    if( id == INT_GPIO_POSEDGE )
      flag = ( EXTI->RTSR & mask ) != 0;
    else
      flag = ( EXTI->FTSR & mask ) != 0;
  }
  if( flag && clear )
    EXTI_ClearFlag( exti_line[ exint_gpio_to_src( resnum ) ] );
  return flag;
}

// ****************************************************************************
// Interrupt: INT_GPIO_POSEDGE

static int int_gpio_posedge_set_status( elua_int_resnum resnum, int status )
{
  return gpioh_set_int_status( INT_GPIO_POSEDGE, resnum, status );
}

static int int_gpio_posedge_get_status( elua_int_resnum resnum )
{
  return gpioh_get_int_status( INT_GPIO_POSEDGE, resnum );
}

static int int_gpio_posedge_get_flag( elua_int_resnum resnum, int clear )
{
  return gpioh_get_int_flag( INT_GPIO_POSEDGE, resnum, clear );
}

// ****************************************************************************
// Interrupt: INT_GPIO_NEGEDGE

static int int_gpio_negedge_set_status( elua_int_resnum resnum, int status )
{
  return gpioh_set_int_status( INT_GPIO_NEGEDGE, resnum, status );
}

static int int_gpio_negedge_get_status( elua_int_resnum resnum )
{
  return gpioh_get_int_status( INT_GPIO_NEGEDGE, resnum );
}

static int int_gpio_negedge_get_flag( elua_int_resnum resnum, int clear )
{
  return gpioh_get_int_flag( INT_GPIO_NEGEDGE, resnum, clear );
}

// ****************************************************************************
// Interrupt: INT_TMR_MATCH

static int int_tmr_match_set_status( elua_int_resnum resnum, int status )
{
  return PLATFORM_INT_NOT_HANDLED;
}

static int int_tmr_match_get_status( elua_int_resnum resnum )
{
  return PLATFORM_INT_NOT_HANDLED;
}

static int int_tmr_match_get_flag( elua_int_resnum resnum, int clear )
{
  return PLATFORM_INT_NOT_HANDLED;
}

// ****************************************************************************
// Interrupt: INT_UART_RX

static int int_uart_rx_get_status( elua_int_resnum resnum )
{
  return USART_GetITStatus( stm32_usart[ resnum ], USART_IT_RXNE ) == SET ? 1 : 0;
}

static int int_uart_rx_set_status( elua_int_resnum resnum, int status )
{
  int prev = int_uart_rx_get_status( resnum );
  USART_ITConfig( stm32_usart[ resnum ], USART_IT_RXNE, status == PLATFORM_CPU_ENABLE ? ENABLE : DISABLE );
  return prev;
}

static int int_uart_rx_get_flag( elua_int_resnum resnum, int clear )
{
  int status = USART_GetFlagStatus( stm32_usart[ resnum ], USART_FLAG_RXNE ) == SET ? 1 : 0;
  if( clear )
    USART_ClearFlag( stm32_usart[ resnum ], USART_FLAG_RXNE );
  return status;
}

// ****************************************************************************
// Initialize interrupt subsystem

// UART IRQ table
#if defined( STM32F10X_LD )
static const u8 uart_irq_table[] = { USART1_IRQn, USART2_IRQn };
#elseif defined( STM32F10X_MD )
static const u8 uart_irq_table[] = { USART1_IRQn, USART2_IRQn, USART3_IRQn };
#else // high density devices
static const u8 uart_irq_table[] = { USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn, UART5_IRQn };
#endif

// EXTI IRQ table
static const u8 exti_irq_table[] = { EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn, EXTI9_5_IRQn, EXTI15_10_IRQn };

void platform_int_init()
{
  NVIC_InitTypeDef nvic_init_structure;
  unsigned i;
  
  // Enable all USART interrupts in the NVIC
  nvic_init_structure.NVIC_IRQChannelPreemptionPriority = 0;
  nvic_init_structure.NVIC_IRQChannelSubPriority = 0;
  nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;  
  for( i = 0; i < sizeof( uart_irq_table ) / sizeof( u8 ); i ++ )
  {
    nvic_init_structure.NVIC_IRQChannel = uart_irq_table[ i ];
    NVIC_Init( &nvic_init_structure );
  }

  // Enable all EXTI interrupts in the NVIC
  for( i = 0; i < sizeof( exti_irq_table ) / sizeof( u8 ); i ++ )
  {
    nvic_init_structure.NVIC_IRQChannel = exti_irq_table[ i ];
    NVIC_Init( &nvic_init_structure );
  }

}

// ****************************************************************************
// Interrupt table
// Must have a 1-to-1 correspondence with the interrupt enum in platform_conf.h!

const elua_int_descriptor elua_int_table[ INT_ELUA_LAST ] = 
{
  { int_gpio_posedge_set_status, int_gpio_posedge_get_status, int_gpio_posedge_get_flag },
  { int_gpio_negedge_set_status, int_gpio_negedge_get_status, int_gpio_negedge_get_flag },
  { int_tmr_match_set_status, int_tmr_match_get_status, int_tmr_match_get_flag },
  { int_uart_rx_set_status, int_uart_rx_get_status, int_uart_rx_get_flag }  
};
