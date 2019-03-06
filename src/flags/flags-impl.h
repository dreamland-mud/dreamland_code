/* $Id: flags-impl.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __FLAGS_IMPL_H__
#define __FLAGS_IMPL_H__

#include "flags.h"

/*----------------------------------------------------------------------
 * Flags
 *---------------------------------------------------------------------*/
inline Flags::Flags( )
{
}

inline Flags::Flags( bitstring_t v, const FlagTable *t )
                          : FlagTableWrapper( t ), Bitstring( v )
{
}

inline DLString Flags::names( ) const
{
    if (table)
        return table->names( getValue( ) );
    else
        return DLString::emptyString;
}

inline DLString Flags::messages( bool comma, char gcase ) const
{
    if (table)
        return table->messages( getValue( ), comma, gcase );
    else
        return DLString::emptyString;
}

/*----------------------------------------------------------------------
 * FlagsArray
 *---------------------------------------------------------------------*/
inline FlagsArray::FlagsArray( const FlagTable * t)
                               : FlagTableWrapper( t )
{
    resize( table->max, 0 );
}


#endif
