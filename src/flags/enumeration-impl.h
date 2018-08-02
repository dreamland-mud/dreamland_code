/* $Id: enumeration-impl.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __ENUMERATIONS_IMPL_H__
#define __ENUMERATIONS_IMPL_H__

#include "enumeration.h"

/*----------------------------------------------------------------------
 * Enumeration 
 *---------------------------------------------------------------------*/
inline Enumeration::Enumeration( )
                        : FlagTableWrapper( NULL ), Integer( 0 )
{
}

inline Enumeration::Enumeration( int v, const FlagTable *t )
                             : FlagTableWrapper( t ), Integer( v )
{
}
inline DLString Enumeration::name( ) const
{
    return (table ? table->name( getValue( ) ) : DLString::emptyString);
}

inline DLString Enumeration::message( char gcase ) const
{
    return (table ? table->message( getValue( ) , gcase ) : DLString::emptyString);
}

/*----------------------------------------------------------------------
 * EnumerationArray
 *---------------------------------------------------------------------*/
inline EnumerationArray::EnumerationArray( )
                               : FlagTableWrapper( NULL )
{
}

inline EnumerationArray::EnumerationArray( const FlagTable * t)
                               : FlagTableWrapper( t )
{
    resize( table->max + 1, 0 );
}

inline void EnumerationArray::clear( )
{
    vector<int>::clear( );
    resize( table->max + 1, 0 );
}

inline void EnumerationArray::fill( int value )
{
    for (iterator i = begin( ); i != end( ); i++)
	*i = value;
}

inline int & EnumerationArray::operator [] (size_type ndx)
{
    static int zeroValue;
    if (!table)
	return zeroValue;
    else
	return vector<int>::operator [](ndx);
}

inline const int & EnumerationArray::operator [] (size_type ndx) const
{
    static int zeroValue;
    if (!table)
	return zeroValue;
    else
	return vector<int>::operator [](ndx);
}

#endif
