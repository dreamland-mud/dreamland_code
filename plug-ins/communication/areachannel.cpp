/* $Id$
 *
 * ruffina, 2004
 */
#include "areachannel.h"
#include "replay.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "descriptor.h"
#include "loadsave.h"
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*-----------------------------------------------------------------------
 * AreaChannel
 *-----------------------------------------------------------------------*/
AreaChannel::AreaChannel( )
{
}

bool AreaChannel::isGlobalListener( Character *ch, Character *victim ) const
{
    if (victim->in_room->area != ch->in_room->area)
        return false;

    return WorldChannel::isGlobalListener( ch, victim );
}

void AreaChannel::triggers( Character *ch, const DLString &msg ) const
{
    const char *str = msg.c_str( );

    DLString chanID( name );
    chanID.upperFirstCharacter( );
        
    for (Character *wch = char_list; wch; wch = wch->next) {
        if (wch->in_room->area == ch->in_room->area) {
            FENIA_VOID_CALL( wch, chanID, "Cs", ch, str );
            
            if (wch->is_npc( ))
                FENIA_NDX_VOID_CALL( wch->getNPC( ), chanID, "CCs", wch, ch, str );

            for (Object *obj = wch->carrying; obj; obj = obj->next_content) {
                FENIA_VOID_CALL( obj, chanID, "Cs", ch, str );
                FENIA_NDX_VOID_CALL( obj, chanID, "OCs", obj, ch, str );
            }
        }
    }

    if (!ch->is_npc( ) || IS_CHARMED(ch))
        WorldChannel::triggers( ch, msg );
}

void AreaChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    if (!outputTo->is_npc())
        remember_history_near( outputTo->getPC( ), message );
}

