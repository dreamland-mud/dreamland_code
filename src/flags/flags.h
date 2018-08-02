/* $Id: flags.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __FLAGS_H__
#define __FLAGS_H__

#include <vector>
#include "flagtablewrapper.h"

struct Flags : public FlagTableWrapper, public Bitstring {
    inline Flags( );
    inline Flags( bitstring_t, const FlagTable * );

    inline DLString names( ) const;
    inline DLString messages( bool comma = false, char gcase = '1' ) const;

    static const Flags emptyFlags;
};

struct FlagsArray : public FlagTableWrapper, vector<int> {
    inline FlagsArray( const FlagTable * );
    /* todo */
};

#endif

#include "flags-impl.h"

