/* $Id: mobiles.cpp,v 1.1.2.12.6.6 2010/01/01 15:48:15 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobiles.h"
#include "objects.h"
#include "stealquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "wearloc_utils.h"
#include "act.h"
#include "interp.h"
#include "loadsave.h"

#include "def.h"

/* 
 * RobbedVictim 
 */
bool RobbedVictim::givenCheck( PCharacter *hero, Object *obj )
{
    return getQuest( ) && quest->check<RobbedItem>( obj );
}

void RobbedVictim::givenGood( PCharacter *hero, Object *obj )
{
    quest->state = QSTAT_FINISHED;

    oldact("$c1 восклицает '{gСпасибо тебе, $C1!{x'", ch, 0, hero, TO_ROOM );
    say_act( hero, ch, "Вернись за вознаграждением к тому, кто рассказал тебе о моем несчастье." );
    
    if (quest->itemWear != wear_none) {
        quest->itemWear->wear( obj, F_WEAR_VERBOSE );
        if (obj->wear_loc != wear_none)
            interpret( ch, "smile" );
        else /* not enough experience to use the item */
            interpret( ch, "emote смущенно улыбается." );
    }
}

void RobbedVictim::givenBad( PCharacter *hero, Object *obj )
{
    say_act( hero, ch, "Ну и зачем мне это?" );
    oldact("$c1 с равнодушным видом протягивает тебе $o4.", ch, obj, hero, TO_VICT );
    oldact("$c1 с равнодушным видом протягивает $C3 $o4.", ch, obj, hero, TO_NOTVICT );
}

void RobbedVictim::deadFromIdiot( PCMemoryInterface *pcm )
{
    oldact("{YИдио$gт|т|тка! Ты уби$gло|л|ла того, кто нуждался в твоей помощи.{x", pcm->getPlayer( ), 0, 0, TO_CHAR);
}

void RobbedVictim::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) 
        oldact_p("{Y$c1 внезапно скончал$gось|ся|ась. Задание отменяется.{x", ch, 0, pcm->getPlayer( ), TO_VICT, POS_DEAD);
}

void RobbedVictim::deadFromKill( PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
        oldact_p("{Y$c1 подло уби$gло|л|ла того, кто нуждался в твоей помощи.{x", killer, 0, pcm->getPlayer( ), TO_VICT, POS_DEAD);
}

void RobbedVictim::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ) && getQuest( ) && !quest->isComplete( ))
        buf << "{x({YХныкает{x) ";
}

void RobbedVictim::talkToHero( PCharacter *hero )
{
    if (!getQuest( ))
        return;
    
    if (ch->position == POS_SLEEPING)
        interpret_raw( ch, "wake" );

    switch (quest->state.getValue( )) {
    case QSTAT_INIT:
            quest->state = QSTAT_HUNT_ROBBER;
            quest->setTime( hero, number_range( 20, 30 ) );
            quest->wiznet( "", "%s tells story", ch->getNameP( '1' ).c_str( ) );
            
            tell_fmt( "{1{W%3$N1{2, подл%4$gое|ый|ая вор%4$gье|юга|овка, "
                      "укра%4$gло|л|ла у меня {1{W%5$N4{2.",
                      hero, ch,
                      quest->thiefName.c_str( ),
                      quest->thiefSex.getValue( ),
                      quest->itemName.c_str( ) );
            tell_fmt( "Но мне удалось кое-что выяснить о грабител%3$gе|е|ьнице.",
                      hero, ch,
                      quest->thiefSex.getValue( ) );
            tell_fmt( "%3$^p1 родом из {1{W{hh%4$s{hx{2, и чаще всего ошивается в районе {1{W%5$s{2.",
                      hero, ch,
                      quest->thiefSex.getValue( ),
                      quest->thiefArea.c_str( ), 
                      quest->thiefRoom.c_str( ) );

            if (!quest->chestRoom.empty( )) {
                tell_fmt( "А награбленное добро, по слухам, прячет около {1{W%3$s{2, точнее сказать не могу.",
                          hero, ch,
                          quest->chestRoom.c_str( ) );
                tell_fmt( "Скорее всего, именно туда %3$p1 припрята%3$gло|л|ла мои вещички, и теперь не расстается с ключом.",
                          hero, ch,
                          quest->thiefSex.getValue( ) );
            }

            tell_raw( hero, ch, "Верни мне украденное! Жду с нетерпением." );

        break;
    }
}


/* 
 * Robber 
 */
void Robber::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim )) 
        buf << "{R[ВОР] {x";
}

