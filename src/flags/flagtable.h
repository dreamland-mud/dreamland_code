/* $Id: flagtable.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __FLAGTABLE_H__
#define __FLAGTABLE_H__

#include "dlstring.h"
#include "bitstring.h"

const int NO_FLAG   = -99;

/*
 * FlagTable
 */
struct FlagTable {
    struct Field {
        bitnumber_t value;
        const char *name;
        const char *message;
    };

    const Field * fields;
    const int * reverse;
    const int size;
    const int max;
    const bool enumerated;
    
    int index( const DLString &arg, bool strict = true ) const;
    bitstring_t bitstring( const DLString &arg, bool strict = true ) const;
    bitnumber_t value( const DLString &arg, bool strict = true ) const;

    DLString names( bitstring_t ) const;
    DLString messages( bitstring_t, bool comma = false, char gcase = '1' ) const;

    DLString name( bitnumber_t value ) const;
    DLString message( bitnumber_t value, char gcase = '1' ) const;
};

#endif
