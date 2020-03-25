/* $Id: steakcustomer.cpp,v 1.1.2.12.6.4 2008/03/06 17:48:29 rufina Exp $
 *
 * ruffina, 2003
 */

#include "steakcustomer.h"
#include "butcherquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "vnum.h"
#include "handler.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

void SteakCustomer::greet( Character *victim ) 
{
    if (ourHero( victim ))
        act( "$c1 выжидающе смотрит на тебя.", ch, 0, victim, TO_VICT );
}

bool SteakCustomer::givenCheck( PCharacter *hero, Object *obj )
{
    MOB_INDEX_DATA *orig;
    
    if (!getQuest( ))
        return false;

    if (obj->pIndexData->vnum != OBJ_VNUM_STEAK) {
        tell_fmt( "Что ты мне приволок%1$Gло|л|ла! Это даже не мясо!", hero, ch );
        return false;
    }

    if (!( orig = get_mob_index( obj->value2() ) )) {
        tell_fmt( "Ужас, с кого ты это среза%1$Gло|л|ла?!", hero, ch );
        return false;
    }

    if (quest->raceName != orig->race) {
        tell_fmt( "Хороший кусок, но я заказыва%2$Gло|л|ла мясо %3$s.",
                  hero, ch, quest->raceRusName.c_str( ) );
        return false;        
        
    } 
    
    if (quest->areaName != orig->area->name) {
        tell_raw( hero, ch, 
                "Эти звери водятся в %s, а не в %s.",
                orig->area->name, quest->areaName.c_str( ) );
        return false;
    }

    return true;
}

void SteakCustomer::givenBad( PCharacter *hero, Object *obj )
{
    act("$c1 возвращает тебе $o4.", ch, obj, hero, TO_VICT);
    act("$c1 возвращает $C5 $o4.", ch, obj, hero, TO_NOTVICT);
}

void SteakCustomer::givenGood( PCharacter *hero, Object *obj )
{
    quest->delivered++;
    
    if (quest->delivered == quest->ordered) 
        tell_raw(hero, ch, "Спасибо за помощь! Вернись к квестору за наградой.");
    else if (quest->delivered > quest->ordered) 
        tell_fmt("Хватит, родн%1$Gое|ой|ая.", hero, ch);
    else 
        tell_raw(hero, ch, "Маловато будет...");
    
    act("$c1 куда-то прячет $o4.", ch, obj, 0, TO_ROOM);
    extract_obj( obj );
}

void SteakCustomer::deadAction( Quest::Pointer quest, PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
        pcm->getPlayer( )->println( "{YЗаказ отменен в связи с кончиной заказчика.{x" );

    ProtectedClient::deadAction( quest, pcm, killer );
}

