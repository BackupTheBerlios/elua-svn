// General purpose function/macros

#ifndef __UTILS_H__
#define __UTILS_H__

#define ABSDIFF( x, y )   ( ( x ) >= ( y ) ? ( x ) - ( y ) : ( y ) - ( x ) )
#define UMIN( x, y )      ( ( x ) <= ( y ) ? ( x ) : ( y ) )
#define UABS( x )         ( ( x ) >= 0 ? ( x ) : -( x ) )
    
#endif
