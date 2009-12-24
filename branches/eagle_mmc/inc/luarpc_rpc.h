#include "cexcept.h"
#include <stdint.h>

#ifdef LUA_RPC
/* signed and unsigned 8, 16 and 32 bit types */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

#define LUARPC_ENABLE_SERIAL
#define BUILD_LUARPC
#endif

/****************************************************************************/
/* handle the differences between winsock and unix */

#ifdef WIN32  /*  BEGIN WIN32 SOCKET SETUP  */

#define close closesocket
#define read(fd,buf,len) recv ((fd),(buf),(len),0)
#define write(fd,buf,len) send ((fd),(buf),(len),0)
#define SOCKTYPE SOCKET
#define sock_errno (WSAGetLastError())

#else

#define SOCKTYPE int
#define net_startup() ;
#define sock_errno errno
#define transport_strerror strerror

#endif

/****************************************************************************/
/* parameters */

#define MAXCON 10 /* maximum number of waiting server connections */

/* a kind of silly way to get the maximum int, but oh well ... */
#define MAXINT ((int)((((unsigned int)(-1)) << 1) >> 1))

/****************************************************************************/
/* error handling */

/* allow special handling for GCC compiler */
#ifdef __GNUC__
#define DOGCC(x) x
#else
#define DOGCC(x) /* */
#endif

/* assertions */

#ifndef NDEBUG
#ifdef __GNUC__
#define MYASSERT(a) if (!(a)) rpcdebug ( \
  "assertion \"" #a "\" failed in %s() [%s]",__FUNCTION__,__FILE__);
#else
#define MYASSERT(a) if (!(a)) rpcdebug ( \
  "assertion \"" #a "\" failed in %s:%d",__FILE__,__LINE__);
#endif
#else
#define MYASSERT(a) ;
#endif

/****************************************************************************/
/* more error handling */

/* error numbers passed around are normal system "errno" error numbers
 * (normally generated by transport operations), except when they have the
 * following values:
 */

enum {
  ERR_EOF       = MAXINT - 100,  /* reached end of file on transport */
  ERR_CLOSED    = MAXINT - 101,  /* attempted operation on closed transport */
  ERR_PROTOCOL  = MAXINT - 102,  /* some error in the received protocol */
  ERR_NODATA    = MAXINT - 103,
  ERR_COMMAND   = MAXINT - 106,
  ERR_HEADER    = MAXINT - 107,
  ERR_LONGFNAME = MAXINT - 108
};

enum exception_type { done, nonfatal, fatal };

struct exception {
  enum exception_type type;
	int errnum;
};

define_exception_type(struct exception);

extern struct exception_context the_exception_context[ 1 ];

#define NUM_FUNCNAME_CHARS 20

/* Transport Connection Structure */

/* FIXME: should be cleaner */
typedef struct _Transport Transport;
struct _Transport 
{
  int fd;      /* INVALID_TRANSPORT if socket is closed */
  unsigned tmr_id;
  u8     loc_little: 1, // Local is little endian?
         loc_armflt: 1, // local float representation is arm float?
         loc_intnum: 1, // Local is integer only?
         net_little: 1, // Network is little endian?
         net_intnum: 1; // Network is integer only?
  u8     lnum_bytes;
};

#define LUARPC_MODE "elua"

typedef struct _Handle Handle;
struct _Handle 
{
  Transport tpt;      /* the handle socket */
  int error_handler;    /* function reference */
  int async;      /* nonzero if async mode being used */
  int read_reply_count;   /* number of async call return values to read */  
};


typedef struct _Helper Helper;
struct _Helper {
  Handle *handle;     /* pointer to handle object */
	Helper *parent; /* parent helper */
	u8 nparents; /* number of parents */
  char funcname[NUM_FUNCNAME_CHARS];  /* name of the function */
};


typedef struct _ServerHandle ServerHandle;
struct _ServerHandle {
  Transport ltpt;   /* listening socket, always valid if no error */
  Transport atpt;   /* accepting socket, valid if connection established */
	int link_errs;
};

/* Maximum number of framing errors before connection reset */
#define MAX_LINK_ERRS ( 2 )

#define INVALID_TRANSPORT (-1)

#define TRANSPORT_VERIFY_OPEN \
	if (tpt->fd == INVALID_TRANSPORT) \
	{ \
		e.errnum = ERR_CLOSED; \
		e.type = fatal; \
		Throw( e ); \
	}

/* Arg & Error Checking Provided to Transport Mechanisms */
int check_num_args (lua_State *L, int desired_n);
void deal_with_error (lua_State *L, Handle *h, const char *error_string);
void my_lua_error( lua_State *L, const char *errmsg );

/* TRANSPORT API */

/* Setup Transport */
void transport_init (Transport *tpt);

/* Open Listener / Server */
void transport_open_listener(lua_State *L, ServerHandle *handle);

/* Open Connection / Client */
int transport_open_connection(lua_State *L, Handle *handle);

/* Accept Connection */
void transport_accept (Transport *tpt, Transport *atpt);

/* Read & Write to Transport */
void transport_read_buffer (Transport *tpt, u8 *buffer, int length);
void transport_write_buffer (Transport *tpt, const u8 *buffer, int length);

/* Check if data is available on connection without reading:
 		- 1 = data available, 0 = no data available */
int transport_readable (Transport *tpt);

/* Check if transport is open:
		- 1 = connection open, 0 = connection closed */
int transport_is_open (Transport *tpt);

/* Shut down connection */
void transport_close (Transport *tpt);
