
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
#include "act_move.h"
#include "interp.h"
#include "clanreference.h"


#include "stats_apply.h"
#include "merc.h"

#include "handler.h"
#include "effects.h"
#include "act.h"
#include "vnum.h"
#include "def.h"
#include "skill_utils.h"

CLAN(battlerager);

GSN(escape);
GSN(hide);
GSN(recall);
GSN(sneak);


/*
 * 'sneak' skill command
 */

SKILL_RUNP( sneak )
{
    Affect af;

    if (MOUNTED(ch))
    {
        ch->pecho("Ты не можешь двигаться бесшумно, когда ты в седле.");
        return;
    }

    affect_strip( ch, gsn_sneak );

    if( IS_AFFECTED(ch,AFF_SNEAK)) {
        if(IS_CHARMED(ch))
        ch->master->pecho("%^$#C1 и так двигается бесшумно.\n\r", ch);
        ch->pecho("Ты и так двигаешься бесшумно.");
        return;
    }

    if ( number_percent( ) < gsn_sneak->getEffective( ch ) + skill_level_bonus(*gsn_sneak, ch))
    {
        int slevel = skill_level(*gsn_sneak, ch);
            
        gsn_sneak->improve( ch, true );
        af.bitvector.setTable(&affect_flags);
        af.type      = gsn_sneak;
        af.level     = slevel;
        af.duration  = slevel;
        
        af.bitvector.setValue(AFF_SNEAK);
        affect_to_char( ch, &af );
        if(IS_CHARMED(ch))
        ch->master->pecho("%1$#^C1 начинает скрытно передвигаться.\n\r", ch);
        ch->pecho("Ты начинаешь скрытно передвигаться.");
    } else {
      gsn_sneak->improve( ch, false );
        if(IS_CHARMED(ch))
        ch->master->pecho("%1$#^C1 не удается скрытно передвигаться.\n\r", ch);
        ch->pecho("Тебе не удается скрытно передвигаться.");
    }

}


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
                            "%1$^C1 растворил%1$Gось|ся|ась в воздухе.",
                            "%1$^C1 и %2$C1 растворяются в воздухе." );
        else
            msgRoomNoParty( wch, 
                            "%1$^C1 появил%1$Gось|ся|ась в комнате.",
                            "%1$^C1 и %2$C1 появляются в комнате." );
    }
    virtual void msgOnStart( )
    {
        msgRoomNoParty( ch, 
                        "%1$^C1 просит о возвращении!",
                        "%1$^C1 и %2$C1 просят о возвращении!" );
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
            ch->pecho( "Тебе некуда возвращаться." );
            return false;
        }

        if (ch->getPC( ))
            point = ch->getPC( )->getHometown( )->getRecall( );
        else
            point = ch->leader->getPC( )->getHometown( )->getRecall( );

        if (!( to_room = get_room_instance( point ) )) {
            ch->pecho( "Ты окончательно заблудил%1$Gось|ся|ась.", ch );
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

/*
 * EscapeMovement
 */
class EscapeMovement : public FleeMovement {
public:
    EscapeMovement( Character *ch, const char *arg )
               : FleeMovement( ch )
    {
        this->arg = arg;
    }

protected:
    virtual bool findTargetRoom( )
    {
        peexit = from_room->extra_exits.find(arg);
        door = find_exit( ch, arg, FEX_NO_EMPTY|FEX_NO_INVIS );

        if ((!peexit || !ch->can_see( peexit )) && door < 0) {
            ch->pecho( "И куда это мы намылились?" );
            return false;
        }

        if (peexit) {
            door = DIR_SOMEWHERE;
            exit_info = peexit->exit_info;
            to_room = peexit->u1.to_room;
        }
        else {
            pexit = from_room->exit[door];
            exit_info = pexit->exit_info;
            to_room = pexit->u1.to_room;
        }

        return true;
    }
    virtual bool canMove( Character *wch )
    {
        if (!checkMovepoints( wch ))
            return false;

        if (!canFlee( wch )) {
            ch->pecho( "Что-то не дает тебе сбежать в этом направлении." );
            return false;
        }
        else
            return true;
    }
    virtual bool tryMove( Character *wch )
    {
        if (!FleeMovement::tryMove( wch ))
            return false;

        if (wch != ch)
            return true;
        
        return applySkill( gsn_escape );
    }
    virtual int getMoveCost( Character *wch )
    {
        return 1;
    }
    virtual bool checkCyclicRooms( Character *wch ) 
    {
        if (from_room == to_room) {
            ch->pecho( "Ты не можешь сбежать туда, попробуй другое место." );
            return false;
        }

        return true;
    }
    virtual bool checkPositionHorse( )
    {
        ch->pecho( "Сначала слезь, а потом уже убегай." );
        return false;
    }
    virtual bool checkPositionRider( )
    {
        ch->pecho( "На тебе сверху кто-то сидит и не дает сбежать." );
        return false;
    }
    virtual bool checkPositionWalkman( )
    {
        if (ch->fighting == 0) {
            if (ch->position == POS_FIGHTING)
                ch->position = POS_STANDING;

            ch->pecho( "Ты сейчас ни с кем не дерешься." );
            return false;
        }

        return true;
    }

    const char *arg;
};

/*
 * 'escape' skill command
 */

SKILL_RUNP( escape )
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if (arg[0] == '\0') {
        ch->pecho( "Укажи направление." );
        return;
    }

    EscapeMovement( ch, arg ).move( );
}

