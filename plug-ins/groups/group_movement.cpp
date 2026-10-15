
/* $Id: group_movement.cpp,v 1.1.2.11 2009/03/16 20:24:06 rufina Exp $
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

#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "recallmovement.h"
#include "fleemovement.h"

#include "player_utils.h"

#include "so.h"

#include "pcharacter.h"
#include "mobilebehavior.h"
#include "room.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "playerattributes.h"
#include "object.h"
#include "affect.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "interp.h"
#include "clanreference.h"


#include "stats_apply.h"
#include "merc.h"

#include "loadsave.h"
#include "act.h"
#include "vnum.h"
#include "def.h"
#include "skill_utils.h"
#include "l10n.h"

CLAN(battlerager);

GSN(escape);
GSN(hide);
GSN(recall);
GSN(sneak);


/*
 * TempleRecallMovement 
 */
class TempleRecallMovement : public RecallMovement {
public:
    TempleRecallMovement( Character *ch )
                   : RecallMovement( ch )
    {
    }
    TempleRecallMovement( Character *ch, Character *actor, Room *to_room )
                   : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
        if (fLeaving)
            msgRoomNoParty( wch,
                            "%1$^C1 vanishes into thin air.",
                            "%1$^C1 растворил%1$Gось|ся|ась в воздухе.",
                            "%1$^C1 розчини%1$Gлось|вся|лась у повітрі.",
                            "%1$^C1 and %2$C1 vanish into thin air.",
                            "%1$^C1 и %2$C1 растворяются в воздухе.",
                            "%1$^C1 і %2$C1 розчиняються в повітрі." );
        else
            msgRoomNoParty( wch,
                            "%1$^C1 appears in the room.",
                            "%1$^C1 появил%1$Gось|ся|ась в комнате.",
                            "%1$^C1 з'яви%1$Gлось|вся|лась у кімнаті.",
                            "%1$^C1 and %2$C1 appear in the room.",
                            "%1$^C1 и %2$C1 появляются в комнате.",
                            "%1$^C1 і %2$C1 з'являються в кімнаті." );
    }
    virtual void msgOnStart( )
    {
        msgRoomNoParty( ch,
                        "%1$^C1 prays for transportation!",
                        "%1$^C1 просит о возвращении!",
                        "%1$^C1 просить про повернення!",
                        "%1$^C1 and %2$C1 pray for transportation!",
                        "%1$^C1 и %2$C1 просят о возвращении!",
                        "%1$^C1 і %2$C1 просять про повернення!" );
    }
    virtual void movePet( NPCharacter *pet )
    {
        TempleRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
    virtual bool findTargetRoom( )
    {
        int point;
        
        if (to_room)
            return true;

        if (!ch->getPC( )
            && (!ch->leader || ch->leader->is_npc( ) || ch->leader->getPC( )->pet != ch))
        {
            ch->pecho( _("Тебе некуда возвращаться.") );
            return false;
        }

        if (ch->getPC( ))
            point = ch->getPC( )->getHometown( )->getRecall( );
        else
            point = ch->leader->getPC( )->getHometown( )->getRecall( );

        if (!( to_room = get_room_instance( point ) )) {
            ch->pecho( _("Команда 'возврат' сейчас недоступна тебе."), ch );
            return false;
        }
        
        return true;                             
    }
    bool checkNewbie( )
    {
        if (ch->is_npc( ))
            return true;

        if (!ch->desc)
            return true;

        if (Player::isNewbie(ch->getPC()))
            return true;

        return checkPumped( );
    }
    virtual bool canMove( Character *wch )
    {
        if (ch != actor)
            return checkForsaken( wch );
        else
            return checkMount( )
                   && checkShadow( )
                   && checkBloody( wch )
                   && checkNewbie( )
                   && checkSameRoom( )
                   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
        if (ch != actor)
            return applyInvis( wch );
        else
            return applyInvis( wch )
                   && applyFightingSkill( wch, gsn_recall )
                   && applyMovepoints( );
    }
};

/*
 * 'recall' skill command
 */

SKILL_RUNP( recall )
{
    TempleRecallMovement( ch ).move( );
}


