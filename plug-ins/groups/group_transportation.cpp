
/* $Id: group_transportation.cpp,v 1.1.2.17.6.14 2010-08-24 20:23:09 rufina Exp $
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
#include "affecthandlertemplate.h"
#include "transportspell.h"
#include "recallmovement.h"
#include "profflags.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"


#include "clanreference.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

CLAN(none);
PROF(samurai);


/*-----------------------------------------------------------------------
 * WordOfRecallMovement
 *-----------------------------------------------------------------------*/
class WordOfRecallMovement : public RecallMovement {
public:
    WordOfRecallMovement( Character *ch )
                     : RecallMovement( ch )
    {
    }
    WordOfRecallMovement( Character *ch, Character *actor, Room *to_room )
                     : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual bool findTargetRoom( )
    {
        int point;
        
        if (to_room)
            return true;

        if (ch->is_npc( ))
            return false;

        if (( point = ch->getClan( )->getRecallVnum( ) ) <= 0)
            point = ch->getPC( )->getHometown( )->getRecall( );

        if (point <= 0 || !( to_room = get_room_instance( point ) )) {
            msgSelf( ch, "You are completely lost." );
            return false;
        }

        return true;
    }
    virtual bool canMove( Character *wch )
    {
        if (ch != actor)
            return checkForsaken( wch );
        else
            return checkMount( )
                   && checkBloody( wch )
                   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
        if (ch != actor)
            return applyInvis( wch );
        else
            return applyInvis( wch )
                   && applyMovepoints( );
    }
    virtual void movePet( NPCharacter *pet )
    {
        WordOfRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
    virtual bool checkBloody( Character *wch )
    {
        if (IS_BLOODY(wch)) {
            msgSelfParty( wch, 
                          "...и никакого эффекта.", 
                          "Богам нет дела до %1$C2." );
            return false;
        }

        return true;
    }
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
        if (fLeaving) 
             msgRoomNoParty( wch,
                             "%1$^C1 исчезает.",
                             "%1$^C1 и %2$C1 исчезают." );
        else
            msgRoomNoParty( wch, 
                            "%1$^C1 появляется в комнате.",
                            "%1$^C1 и %2$C1 появляются в комнате." );
    }
};

/*
 * 'word of recall' spell
 */
SPELL_DECL(WordOfRecall);
VOID_SPELL(WordOfRecall)::run( Character *ch, Character *, int sn, int level ) 
{ 
    if (ch->getProfession( ) == prof_samurai && ch->fighting) {
        ch->pecho("Твоя честь не позволяет тебе воспользоваться словом возврата!");
        return;
    }

    WordOfRecallMovement( ch ).move( );
}

