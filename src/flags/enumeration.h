/* $Id: enumeration.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */

#ifndef __ENUMERATION_H__
#define __ENUMERATION_H__

#include <vector>
#include "integer.h"
#include "flagtablewrapper.h"

/*
 * Enumeration
 */
struct Enumeration : public FlagTableWrapper, public Integer {
    inline Enumeration( );
    inline Enumeration( int, const FlagTable * );

    inline DLString name( ) const;
    inline DLString message( char gcase = '1' ) const;

    static const Enumeration emptyEnumeration;
};

/*
 * EnumerationArray
 */
struct EnumerationArray : public FlagTableWrapper, public vector<int> { 
    inline EnumerationArray( );
    inline EnumerationArray( const FlagTable * );
    
    inline void clear( );
    inline void fill( int );

    inline int & operator [] (size_type);
    inline const int & operator [] (size_type) const;

    static const EnumerationArray emptyArray;
};

#endif

#include "enumeration-impl.h"
