/* $Id: class_ranger.cpp,v 1.1.2.25.4.25 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "logstream.h"
#include "summoncreaturespell.h"
#include "objthrow.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "spelltarget.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "pcharactermanager.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "string_utils.h"
#include "dreamland.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "magic.h"
#include "fight.h"
#include "weapongenerator.h"
#include "stats_apply.h"
#include "directions.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "tohit.h"
#include "merc.h"
#include "loadsave.h"
#include "save.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"
#include "l10n.h"

PROF(ranger);
GSN(track);


/*
 * 'track' skill command
 */
static bool oprog_track( Object *obj, Character *ch, const char *target, int door )
{
    FENIA_CALL( obj, "Track", "Csi", ch, target, door );
    FENIA_NDX_CALL( obj, "Track", "OCsi", obj, ch, target, door );
    return false;
}

SKILL_RUNP( track )
{
    DLString arg( argument );
    EXIT_DATA *pexit;
    int d;

    if (arg.empty( )) {
        ch->pecho( _("Кого ты хочешь выследить?") );
        return;
    }

    oldact(_("$c1 всматривается в землю в поисках следов."),ch,0,0,TO_ROOM);

    int slevel;
    slevel = gsn_track->getEffective( ch ) + skill_level_bonus(*gsn_track, ch);  
    
    const RoomHistoryEntry *h = 0;

    if (number_percent() < slevel)
        h = ch->in_room->history.find( arg, false );

    if (h && h->went < DIR_SOMEWHERE) {
        d = h->went;

        if (( pexit = ch->in_room->exit[d] )) {

            gsn_track->improve( ch, true );

            for (Object *obj = ch->carrying; obj; obj = obj->next_content)
                if (oprog_track(obj, ch, arg.c_str(), d))
                    return;

            ch->pecho(_("Следы %N2 ведут %s."), arg.c_str(), dirs[d].leave);
            
            if (IS_SET(pexit->exit_info, EX_CLOSED)) 
                open_door_extra( ch, d, pexit );
            
            move_char(ch, d );
            return;
        }
    }

    // The trail ends at a portal or a hidden passage: follow it in.
    Object *portal = (h && h->portal ? trail_portal( ch->in_room, h->portal ) : 0);

    if (portal && ch->can_see( portal )) {
        gsn_track->improve( ch, true );

        for (Object *obj = ch->carrying; obj; obj = obj->next_content)
            if (oprog_track(obj, ch, arg.c_str(), DIR_SOMEWHERE))
                return;

        ch->pecho(_("Следы %N2 ведут в %O4."), arg.c_str(), portal);

        if (!IS_SET(portal->value1( ), EX_CLOSED) || open_portal( ch, portal ))
            move_char( ch, portal );
        return;
    }

    EXTRA_EXIT_DATA *peexit = (h && h->eexit ? trail_eexit( ch->in_room, h->eexit ) : 0);

    if (peexit && ch->can_see( peexit )) {
        gsn_track->improve( ch, true );

        for (Object *obj = ch->carrying; obj; obj = obj->next_content)
            if (oprog_track(obj, ch, arg.c_str(), DIR_SOMEWHERE))
                return;

        ch->pecho(_("Следы %N2 ведут в потайной проход."), arg.c_str());

        if (IS_SET(peexit->exit_info, EX_CLOSED))
            open_door_extra( ch, DIR_SOMEWHERE, peexit );

        move_char( ch, peexit );
        return;
    }
    
    ch->pecho(_("Ты не видишь здесь следов."));
    gsn_track->improve( ch, false );
}
