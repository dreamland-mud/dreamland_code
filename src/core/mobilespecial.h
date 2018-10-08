/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MOBILESPECIAL_H
#define MOBILESPECIAL_H

#include "dlstring.h"

class NPCharacter;

typedef bool SPEC_FUN        ( NPCharacter *ch );

struct spec_type
{
    const char *         name;                        /* special function name */
    SPEC_FUN *        function;                /* the function */
};

extern const   struct  spec_type    *spec_table;
extern const   struct  spec_type    zero_spec_table[];

bool empty_prog( ... );

template <typename PType>
struct ProgWrapper {
    ProgWrapper( ) : func( 0 )
    {
    }
    
    PType * operator * () 
    {
        if (func)
            return func;
        else
            return (PType *)&empty_prog;
    }

    PType * operator = ( PType *mp )
    {
        func = mp;
        return func;
    }

    void clear( )
    {
        func = 0;
        name.clear( );
    }

    PType *func;
    DLString name;
};

SPEC_FUN *        spec_lookup         ( const char *name );
const char *        spec_name        ( SPEC_FUN *function );

#endif
