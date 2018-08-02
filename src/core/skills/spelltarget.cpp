/* $Id$
 *
 * ruffina, 2004
 */
#include "spelltarget.h"

SpellTarget::SpellTarget( ) 
{
    init( );
}
SpellTarget::SpellTarget( Character *victim ) 
{
    init( );
    this->victim = victim;
    type = CHAR;
}
SpellTarget::SpellTarget( Object *obj )
{
    init( );
    this->obj = obj;
    type = OBJECT;
}
SpellTarget::SpellTarget( Room *room ) 
{
    init( );
    this->room = room;
    type = ROOM;
}
SpellTarget::SpellTarget( const char *arg )
{
    init( );
    this->arg = arg;
    type = NONE;
}

SpellTarget::~SpellTarget( ) 
{
}

void SpellTarget::init( )
{
    arg = 0;
    victim = 0;
    obj = 0;
    room = 0;
    type = UNDEF;
    castFar = false;
    door = -1;
    range = -1;
}

