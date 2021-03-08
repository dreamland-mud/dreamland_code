/* $Id: rats.cpp,v 1.1.2.11.6.3 2008/05/10 01:22:36 rufina Exp $
 *
 * ruffina, cradya, 2004
 */

#include "rats.h"

#include "class.h"
#include "regexp.h"

#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"


#include "save.h"
#include "act.h"
#include "merc.h"
#include "handler.h"
#include "move_utils.h"
#include "vnum.h"
#include "mercdb.h"
#include "def.h"


static Character * get_actor( Character *victim ) 
{
    if (victim->is_mirror( ) && victim->doppel)
        return get_actor( victim->doppel );
    
    if (!victim->is_npc( ) && !IS_CHARMED(victim))
        return victim;
    
    if (victim->leader && victim->leader != victim)
        return get_actor( victim->leader );
    
    if (victim->master && victim->master != victim)
        return get_actor( victim->master );
    
    return victim;
}

/*----------------------------------------------------------------------------
 * Rat behavior (disapper after stopfol and curse the killer)
 *--------------------------------------------------------------------------*/
Rat::Rat( ) 
{
    timer.setValue( -1 );
}

bool Rat::death( Character *killer ) {
   
    if (!killer || killer == ch)
        return false;

    killer = get_actor( killer );

    if (killer->is_npc( ))
        return false;

    killer->getPC( )->getAttributes( ).getAttr<XMLAttributeRats>( "rats" )->nongrata.setValue( true );
    return false;
}

bool Rat::area( ) 
{
    int time = timer.getValue( );
    Character *master = ch->master;
    
    if (LevelAdaptivePet::area( ))
        return true;

    if (master && !master->is_npc( )) {
        XMLAttributeRats::Pointer attr = master->getPC( )->getAttributes( ).findAttr<XMLAttributeRats>( "rats" );
        
        if (attr && attr->nongrata == true) {
            act_p( "$c1 говорит тебе '{GЯ не желаю служить тебе..{x'", ch, 0, master, TO_VICT, POS_DEAD );
            extract_char( ch );
            return true;
        }
    } else if (ch->in_room && ch->in_room->vnum != 778) { /* not in paradise */
        stopfol( NULL );
    }
    
    if (time == -1)
        return false;

    if (time > 1) {
        timer.setValue( time - 1);
        return false;
    }

    act("$c1 отправляется в крысиный рай.", ch, 0, 0, TO_ROOM);
    extract_char( ch );
    return true;
}

void Rat::stopfol( Character *master ) {
    LevelAdaptivePet::stopfol( master );

    if (timer.getValue( ) == -1) {
        timer.setValue( 5 ); 
        save_mobs( ch->in_room );
    }
}

/*----------------------------------------------------------------------------
 * Rat God (prevent rat killer from entering the Temple)
 *--------------------------------------------------------------------------*/
void RatGod::greet( Character *mob ) {
    XMLAttributeRats::Pointer attr;
    DLString race;

    race = mob->getRace( )->getName( );
    
    if (race == "felar" || race == "lion" || race == "cat") {
        exorcism( mob, "{DС котами нельзя!{x" );
        return;
    }
    
    mob = get_actor( mob );
    if (mob->is_npc( ))
        return;

    attr = mob->getPC( )->getAttributes( ).findAttr<XMLAttributeRats>( "rats" );

    if (attr) {
        if (attr->nongrata.getValue( ) == true) {
            exorcism( mob, "{rКрысоубийце{D не место здесь.{x" );
            return;
        }
        if (attr->desecrator) {
            exorcism( mob, "Тебя здесь не желают видеть." );
            return;
        }
    }
}

void RatGod::exorcism( Character *victim, const char *msg ) {
    Room *to_room;
    EXIT_DATA *pexit;
    
    pexit = ch->in_room->exit[DIR_UP];

    if (pexit && pexit->u1.to_room)
        to_room = pexit->u1.to_room;
    else
        to_room = get_room_instance( ROOM_VNUM_TEMPLE );
    
    transfer_char( victim, ch, to_room,
                   "%1$^C1 улетучивается.",
                   msg,
                   "%1$^C1 появил%1$Gось|ся|ась в комнате." );
}


/*----------------------------------------------------------------------------
 * Attribute for rat killers 
 *--------------------------------------------------------------------------*/

XMLAttributeRats::XMLAttributeRats( ) : nongrata( false ), desecrator( false )
{
}

