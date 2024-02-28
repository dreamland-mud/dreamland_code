/* $Id$
 *
 * ruffina, 2004
 */
#ifndef SPELLTARGET_H
#define SPELLTARGET_H

#include "dlobject.h"
#include "pointer.h"
#include "dlstring.h"

class Character;
class Object;
class Room;

struct SpellTarget : public virtual DLObject {
    typedef ::Pointer<SpellTarget> Pointer;
    
    SpellTarget( ); 
    SpellTarget( Character *victim ); 
    SpellTarget( Object *obj );
    SpellTarget( Room *room ); 
    SpellTarget( const DLString &arg);

    virtual ~SpellTarget( );
    
    void init( );
    inline bool isValid( ) const;

    DLString arg;
    Character *victim;
    Object *obj;
    Room *room;
    DLString doorOrExtraExit;
    DLString extraExit;
    int argdoor;

    enum {
        UNDEF,
        NONE,
        CHAR,
        OBJECT,
        ROOM,
        EXIT
    } type;

    bool castFar;
    int door, range;
    int error;
};

inline bool SpellTarget::isValid( ) const
{
    return type != UNDEF;
}

#endif
