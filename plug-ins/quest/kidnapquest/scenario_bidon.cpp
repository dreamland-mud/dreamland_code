/* $Id: scenario_bidon.cpp,v 1.1.2.13.6.4 2009/01/18 20:11:58 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario_bidon.h"
#include "kidnapquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "act.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define KS KidnapBidonScenario

bool KS::applicable( PCharacter *hero )
{
    return !IS_EVIL(hero);
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 с громким ревом бросается на шею $C3. Как гадко...", kid, 0, king, TO_ROOM);
    hero->printf( "%s и %s уже встретились.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    act("Приди к $C3 за благодарностью!", hero, 0, king, TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) 
{
    if(hero == killer) {
        act("{YИдио$gт|т|тка.... Ты уби$gло|л|ла того, кто нуждался в твоей помощи.{x", killer, 0, 0, TO_CHAR);
        hero->send_to("{YЗадание отменяется.{x\r\n");
    } else {
        act("{Y$c1 подло убил того, кто нуждался в твоей помощи.{x", killer, 0, hero, TO_VICT);
        hero->send_to("{YЗадание отменяется.{x\r\n");
    }
}
void KS::msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) 
{
    if(hero == killer) {
        act("{YИдио$gт|т|тка.... Ты уби$gло|л|ла того, кого долж$gно|ен|на бы$gло|л|ла спасти.{x", killer, 0, 0, TO_CHAR);
        hero->send_to("{YЗадание отменяется.{x\r\n");
    } else {
        act("{Y$c1 подло уби$gло|л|ла того, кого тебе было поручено спасти.{x", killer, 0, hero, TO_VICT);
        hero->send_to("{YЗадание отменяется.{x\r\n");
    }
}

/*
 * bandit actions
 */
void KS::actAttackHero( NPCharacter *bandit, PCharacter *hero ) 
{
    if (!hero->fighting) {
        act("$c1 сквозь зубы произносит: '{gА вас я попрошу остаться...{x'.", bandit, 0, hero, TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) 
{
    act("$c1 достает конфетку и подманивает $C4.", bandit, 0, kid, TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) 
{
    if(number_percent() < 10)
        act("$c1 глубоко затягивается огромной сигарой.", bandit, 0, 0, TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) 
{
    if(number_percent() < 10)
        act("$c1 показывает девочке куклу.", bandit, 0, 0, TO_ROOM);
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) 
{
    if(number_percent() < 10)
        act("$c1 озадаченно оглядывается по сторонам.", bandit, 0, 0, TO_ROOM);
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) 
{
    act("$c1 говорит тебе '{GМоя дочурка вчера утром ушла за молоком, взяла деньги, но забыла бидончик.{x'", king, 0, hero, TO_VICT);
    act("$c1 говорит тебе '{GВот уже прошли сутки, а ее все нет. Я очень волнуюсь.{x'", king, 0, hero, TO_VICT);
    act("$c1 говорит тебе '{GСкорее всего, она заблудилась где-то в районе $t{x'", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) 
{
    char buf[MAX_STRING_LENGTH];
    
    act("$c1 вручает тебе $o4.", king, mark, hero, TO_VICT);
    act("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);

    act("$c1 говорит тебе '{GВозьми этот бидончик и передай ей...{x'", king, 0, hero, TO_VICT);
    if(number_percent() < 10) {
        act("$c1 пронзительно кричит '{YЧТОБ БЕЗ МОЛОКА НЕ ВОЗВРАЩАЛАСЬ!...{x'", king, 0, hero, TO_VICT);
        sprintf( buf, "$c1 говорит тебе '{GИ учти, что через {Y%d{G минут%s мое терпение лопнет!{x'",
                 time, GET_COUNT(time, "у", "ы", "") );
    } else {
        sprintf( buf, "$c1 говорит тебе '{GПоторопись! Материнское сердце подсказывает мне, "
                      "что если ты не приведешь ее ко мне через {Y%d{G минут%s, "
                      "с ней случится что-то непоправимое.{x'",
                 time, GET_COUNT(time, "у", "ы", "") );
    }

    act(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) 
{
    act("$c1 говорит тебе '{GА предыдущий ты слома$Gло|л|ла?{x'", king, 0, hero, TO_VICT);
    act("$c1 дает тебе новый $o4.", king, mark, hero, TO_VICT);
    act("$c1 дает $C3 новый $o4.", king, mark, hero, TO_NOTVICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 чмокает тебя в обе щеки.", king, 0, hero, TO_VICT);
    act("$c1 чмокает $C4 в обе щеки.", king, 0, hero, TO_NOTVICT);
    act("$c1 говорит тебе '{GИди скорее за наградой к тому, кто дал тебе задание!{x'.", king, 0, hero, TO_VICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) 
{
    if(number_percent( ) < 10)
        act("$c1 ждет прекрасного принца.", kid, 0, 0, TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) 
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
        act("$c1 потерянно озирается в поисках $C2.", kid, 0, hero, TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) 
{
    interpret( kid, "yell Я потерялась!!!" );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) 
{
    act("$c1 пытается положить деньги в $o4, но ничего не получается.", kid, obj, 0, TO_ROOM);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) 
{
    act("$c1 безо всякого интереса смотрит на $o4.", kid, obj, 0, TO_ROOM);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) 
{
    interpret(kid, "hop");
    act("$c1 радостно кричит '{YЭто бидончик моей мамочки!{x'", kid, 0, 0, TO_ROOM);
    act("$c1 радостно кричит '{YОтведите меня к ней, пожалуйста!{x'", kid, 0, 0, TO_ROOM);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 с громким ревом бросается на шею $C3. Как гадко...", kid, 0, king, TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) 
{
    act("{YВнезапно из засады выпрыгивает банда похитителей маленьких детей!{x", kid, 0, 0, TO_ROOM);
}

