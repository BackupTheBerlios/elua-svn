// Newlib stubs implementation

#include <reent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include "devman.h"
#include "ioctl.h"
#include "platform.h"

// Utility function: look in the device manager table and find the index
// for the given name. Returns an index into the device structure, -1 if error.
// Also returns a pointer to the actual file name (without the device part)
static int find_dm_entry( const char* name, char **pactname )
{
  int i;
  const DM_DEVICE* pdev;
  const char* preal;
  char tempname[ DM_MAX_DEV_NAME + 1 ];
  
  // Sanity check for name
  if( name == NULL || *name == '\0' || *name != '/' )
    return -1;
    
  // Find device name
  preal = strchr( name + 1, '/' );
  if( preal == NULL )
  {
    // This shortcut allows to register the "/" filesystem and use it like "/file.ext"
    strcpy( tempname, "/" );
    preal = name;
  }
  else
  {
    if( ( preal - name > DM_MAX_DEV_NAME ) || ( preal - name == 1 ) ) // name too short/too long
      return -1;
    memcpy( tempname, name, preal - name );
    tempname[ preal - name ] = '\0';
  }
    
  // Find device
  for( i = 0; i < dm_get_num_devices(); i ++ )
  {
    pdev = dm_get_device_at( i );
    if( !strcasecmp( tempname, pdev->name ) )
      break;
  }
  if( i == dm_get_num_devices() )
    return -1;
  // Find the actual first char of the name
  preal ++;
  if( *preal == '\0' )
    return -1;
  *pactname = ( char * )preal;
  return i;  
}

// *****************************************************************************
// _open_r
int _open_r( struct _reent *r, const char *name, int flags, int mode )
{
  char* actname;
  int res, devid;
  const DM_DEVICE* pdev;
 
  // Look for device, return error if not found or if function not implemented
  if( ( devid = find_dm_entry( name, &actname ) ) == -1 )
  {
    r->_errno = ENODEV;
    return -1; 
  }
  pdev = dm_get_device_at( devid );
  if( pdev->p_open_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1;   
  }
  
  // Device found, call its function
  if( ( res = pdev->p_open_r( r, actname, flags, mode ) ) < 0 )
    return res;
  return DM_MAKE_DESC( devid, res );
}

// *****************************************************************************
// _close_r
int _close_r( struct _reent *r, int file )
{
  const DM_DEVICE* pdev;
  
  // Find device, check close function
  pdev = dm_get_device_at( DM_GET_DEVID( file ) );
  if( pdev->p_close_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the close function
  return pdev->p_close_r( r, DM_GET_FD( file ) );
}

// *****************************************************************************
// _fstat_r (not implemented)
int _fstat_r( struct _reent *r, int file, struct stat *st )
{
  if( ( file >= DM_STDIN_NUM ) && ( file <= DM_STDERR_NUM ) )
  {
    st->st_mode = S_IFCHR;
    return 0;
  }
  r->_errno = ENOSYS;
  return -1;
}

// *****************************************************************************
// ioctl (actually our call, not newlib's)
static int _ioctl_r( struct _reent *r, int file, unsigned long request, void *ptr )
{
  const DM_DEVICE* pdev;
  
  // Find device, check ioctl function
  pdev = dm_get_device_at( DM_GET_DEVID( file ) );
  if( pdev->p_ioctl_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the ioctl function
  return pdev->p_ioctl_r( r, DM_GET_FD( file ), request, ptr );  
}

int ioctl( int file, unsigned long request, void *ptr )
{
  return _ioctl_r( _REENT, file, request, ptr );
}

// *****************************************************************************
// _read_r 
_ssize_t _read_r( struct _reent *r, int file, void *ptr, size_t len )
{
  const DM_DEVICE* pdev;
  
  // Find device, check read function
  pdev = dm_get_device_at( DM_GET_DEVID( file ) );
  if( pdev->p_read_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the read function
  return pdev->p_read_r( r, DM_GET_FD( file ), ptr, len );  
}

// *****************************************************************************
// _write_r 
_ssize_t _write_r( struct _reent *r, int file, const void *ptr, size_t len )
{
  const DM_DEVICE* pdev;
  
  // Find device, check write function
  pdev = dm_get_device_at( DM_GET_DEVID( file ) );
  if( pdev->p_write_r == NULL )
  {
    r->_errno = ENOSYS;
    return -1; 
  }
  
  // And call the write function
  return pdev->p_write_r( r, DM_GET_FD( file ), ptr, len );  
}

// *****************************************************************************
// _sbrk_r
static char *heap_ptr; 

void* _sbrk_r( struct _reent* r, ptrdiff_t incr )
{
  char* ptr;
  
  if( incr == 0 )
    ptr = heap_ptr;
  else
  {
    if( heap_ptr == NULL )
      heap_ptr = ( char* )platform_get_first_free_ram();   
    if( heap_ptr + incr >= ( char* )platform_get_last_free_ram() ) 
      ptr = ( void* )-1;
    else
    {
      ptr = heap_ptr;
      heap_ptr += incr; 
    }
  }
  return ptr;
}

// *****************************************************************************
// _lseek_r
_off_t _lseek_r( struct _reent *r, int file, _off_t ptr, int dir )
{
  struct fd_seek seek;
    
  seek.off = ptr;
  seek.dir = dir;
  
  if( ioctl( file, FDSEEK, &seek ) < 0 ) 
    return (_off_t)-1;
  else
    return seek.off;
}

int _isatty_r( struct _reent* r, int fd )
{
  return 1;
}

#ifndef WIN32

int isatty( int fd )
{
  return 1;
}
#include <sys/types.h>
#include <unistd.h>

pid_t _getpid()
{
  return 0;
}

// For some very strange reason, the next function is required by the i386 platform...
pid_t getpid()
{
  return 0;
}

#include <sys/times.h>
clock_t _times_r( struct _reent* r, struct tms *buf )
{
  return 0;
}

int _unlink_r( struct _reent *r, const char *name )
{
  r->_errno = ENOSYS;
  return -1;
}

int _link_r( struct _reent *r, const char *c1, const char *c2 )
{
  r->_errno = ENOSYS;
  return -1;
}

#include <sys/time.h>
int _gettimeofday_r( struct _reent *r, struct timeval *tv, void *tz )
{
  r->_errno = ENOSYS;
  return -1;  
}

#include <stdlib.h>
void exit( int status )
{
  while( 1 );
}

#endif

// If LUA_INTONLY is defined, "redirect" printf/scanf calls to their integer counterparts
#ifdef LUA_INTONLY
int _vfprintf_r( struct _reent *r, FILE *stream, const char *format, va_list ap )
{
  return _vfiprintf_r( r, stream, format, ap );
}

extern int __svfiscanf_r(struct _reent *,FILE *, _CONST char *,va_list);
int __svfscanf_r( struct _reent *r, FILE *stream, const char *format, va_list ap )
{
  return __svfiscanf_r( r, stream, format, ap );
}

#endif
