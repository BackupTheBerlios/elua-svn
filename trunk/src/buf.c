// eLua "char device" buffering system

#include "platform_conf.h"

#if defined( BUF_ENABLE_UART ) || defined( BUF_ENABLE_ADC )
#define BUF_ENABLE
#endif

#ifdef BUF_ENABLE

#include "buf.h"
#include "type.h"
#include "platform.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

// [TODO]? Following code might need a C99 compiler (for 0-sized arrays)
#ifdef BUF_ENABLE_UART
  static buf_desc buf_desc_uart[ NUM_UART ];
#else
  static buf_desc buf_desc_uart[ 0 ];
#endif

#ifdef BUF_ENABLE_ADC
  static buf_desc buf_desc_adc [ NUM_ADC ];
#else
  static buf_desc buf_desc_adc [ 0 ];
#endif

// NOTE: the order of descriptors here MUST match the order of the BUF_ID_xx
// enum in inc/buf.h
static const buf_desc* buf_desc_array[ BUF_ID_TOTAL ] = 
{
  buf_desc_uart,
  buf_desc_adc,
};

// Helper macros
#define BUF_MOD_INCR( p, m ) p->m = ( p->m + ( ( u16 )1 << p->logdsize ) ) & ( ( ( u16 )1 << p->logsize ) - 1 )
#define BUF_REALSIZE( p ) ( ( u16 )1 << ( p->logsize - p->logdsize ) )
#define BUF_BYTESIZE( p ) ( ( u16 )1 << p->logsize )
#define BUF_REALDSIZE( p ) ( ( u16 )1 << p->logdsize )
#define BUF_GETPTR( resid, resnum ) buf_desc *pbuf = ( buf_desc* )buf_desc_array[ resid ] + resnum

// READ16 and WRITE16 macros are here to ensure _atomic_ reads and writes of 
// 16-bits data. Might have to be changed for an 8-bit architecture.
#define READ16( p )     p
#define WRITE16( p, x ) p = x

// Initialize the buffer of the specified resource
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
// bufsize - new size of the buffer (one of the BUF_SIZE_xxx constants from
// dsize - number of bytes held by each element (must be a power of 2)
//   buf.h, or BUF_SIZE_NONE to disable buffering
// Returns 1 on success, 0 on failure
int buf_set( unsigned resid, unsigned resnum, u8 logsize, size_t dsize )
{
  BUF_GETPTR( resid, resnum );
  u8 prevlogsize = pbuf->logsize;
  
  // Make sure dsize is a power of 2
  if ( dsize & ( dsize - 1 ) )
    return PLATFORM_ERR;
   
  pbuf->logdsize = intlog2( dsize );
  pbuf->logsize = logsize + ( pbuf->logdsize );
  
  if( ( pbuf->buf = ( t_buf_data* )realloc( pbuf->buf, BUF_BYTESIZE( pbuf ) ) ) == NULL )
  {
    pbuf->logsize = BUF_SIZE_NONE;
    pbuf->rptr = pbuf->wptr = pbuf->count = 0;
    if( logsize != BUF_SIZE_NONE )
      return PLATFORM_ERR;
  }
  
  return PLATFORM_OK;
}

// Marks buffer as empty
void buf_flush( unsigned resid, unsigned resnum )
{
  BUF_GETPTR( resid, resnum );
  
  pbuf->rptr = pbuf->wptr = pbuf->count = 0;
}

// Write to buffer
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
// data - pointer for where data will come from
// dsize - length of data to get
// Returns PLATFORM_OK on success, PLATFORM_ERR on failure
// [TODO] maybe add a buffer overflow flag
int buf_write( unsigned resid, unsigned resnum, t_buf_data *data )
{
  BUF_GETPTR( resid, resnum );
    
  memcpy( &pbuf->buf[ pbuf->wptr ], data, BUF_REALDSIZE( pbuf ) );
  
  BUF_MOD_INCR( pbuf, wptr );
  
  if( pbuf->count == BUF_REALSIZE( pbuf ) )
    BUF_MOD_INCR( pbuf, rptr );
  else
    pbuf->count ++;
    
  return PLATFORM_OK;
}

// Returns 1 if the specified device is buffered, 0 otherwise
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
int buf_is_enabled( unsigned resid, unsigned resnum )
{
  BUF_GETPTR( resid, resnum );
    
  return pbuf->logsize != BUF_SIZE_NONE;
}

// Return the size of the buffer in number
unsigned buf_get_size( unsigned resid, unsigned resnum )
{
  BUF_GETPTR( resid, resnum );
    
  return BUF_REALSIZE( pbuf );
}

// Return the size of the data in the buffer
unsigned buf_get_count( unsigned resid, unsigned resnum )
{
  BUF_GETPTR( resid, resnum );
  
  return READ16( pbuf->count );  
}

// Get data from buffer of size dsize
// resid - resource ID (BUF_ID_UART ...)
// resnum - resource number (0, 1, 2...)
// data - pointer for where data should go
// dsize - length of data to get
// Returns PLATFORM_OK on success, PLATFORM_ERR on failure, 
//   PLATFORM_UNDERFLOW on buffer empty
int buf_read( unsigned resid, unsigned resnum, t_buf_data *data )
{
  BUF_GETPTR( resid, resnum );
  
  if( READ16( pbuf->count ) == 0 )
    return PLATFORM_UNDERFLOW;

  memcpy( data, &pbuf->buf[ pbuf->rptr ], BUF_REALDSIZE( pbuf ) );

  platform_cpu_disable_interrupts();
  pbuf->count --;
  BUF_MOD_INCR( pbuf, rptr );
  platform_cpu_enable_interrupts();
  
  return PLATFORM_OK;  
}

#endif // #ifdef BUF_ENABLE

