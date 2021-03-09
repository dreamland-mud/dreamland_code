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

bool KS::applicable( PCharacter *hero ) const
{
    return !IS_EVIL(hero);
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const
{
    act("%1$^C1 с громким ревом бросается на шею %2$C3. Как гадко...", kid, king, 0,TO_ROOM);
    hero->printf( "%s и %s уже встретились.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    act("Приди к %2$C3 за благодарностью!", hero, king, 0,TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) const
{
    if(hero == killer) {
        oldact("{YИдио$gт|т|тка.... Ты уби$gло|л|ла того, кто нуждался в твоей помощи.{x", killer, 0, 0, TO_CHAR);
        hero->pecho("{YЗадание отменяется.{x");
    } else {
        act("{Y%C1 подло убил того, кто нуждался в твоей помощи.{x", killer, hero, 0,TO_VICT);
        hero->pecho("{YЗадание отменяется.{x");
    }
}
void KS::msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) const
{
    if(hero == killer) {
        oldact("{YИдио$gт|т|тка.... Ты уби$gло|л|ла того, кого долж$gно|ен|на бы$gло|л|ла спасти.{x", killer, 0, 0, TO_CHAR);
        hero->pecho("{YЗадание отменяется.{x");
    } else {
        oldact("{Y$c1 подло уби$gло|л|ла того, кого тебе было поручено спасти.{x", killer, 0, hero, TO_VICT);
        hero->pecho("{YЗадание отменяется.{x");
    }
}

/*
 * bandit actions
 */
void KS::actAttackHero( NPCharacter *bandit, PCharacter *hero ) const
{
    if (!hero->fighting) {
        act("%^C1 сквозь зубы произносит: '{gА вас я попрошу остаться...{x'.", bandit, hero, 0,TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) const
{
    act("%1$^C1 достает конфетку и подманивает %2$C4.", bandit, kid, 0,TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) const
{
    if(number_percent() < 10)
        act("%^C1 глубоко затягивается огромной сигарой.", bandit, 0, 0,TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) const
{
    if(number_percent() < 10)
        act("%^C1 показывает девочке куклу.", bandit, 0, 0,TO_ROOM);
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) const
{
    if(number_percent() < 10)
        act("%^C1 озадаченно оглядывается по сторонам.", bandit, 0, 0,TO_ROOM);
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) const
{
    act("%^C1 говорит тебе '{GМоя дочурка вчера утром ушла за молоком, взяла деньги, но забыла бидончик.{x'", king, hero, 0,TO_VICT);
    act("%^C1 говорит тебе '{GВот уже прошли сутки, а ее все нет. Я очень волнуюсь.{x'", king, hero, 0,TO_VICT);
    oldact("$c1 говорит тебе '{GСкорее всего, она заблудилась где-то в районе {W{hh$t{x'", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) const
{
    char buf[MAX_STRING_LENGTH];
    
    act("%1$^C1 вручает тебе %3$C4.", king, hero, mark,TO_VICT);
    oldact("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);

    act("%^C1 говорит тебе '{GВозьми этот бидончик и передай ей...{x'", king, hero, 0,TO_VICT);
    if(number_percent() < 10) {
        act("%^C1 пронзительно кричит '{YЧТОБ БЕЗ МОЛОКА НЕ ВОЗВРАЩАЛАСЬ!...{x'", king, hero, 0,TO_VICT);
        sprintf( buf, "$c1 говорит тебе '{GИ учти, что через {Y%d{G минут%s мое терпение лопнет!{x'",
                 time, GET_COUNT(time, "у", "ы", "") );
    } else {
        sprintf( buf, "$c1 говорит тебе '{GПоторопись! Материнское сердце подсказывает мне, "
                      "что если ты не приведешь ее ко мне через {Y%d{G минут%s, "
                      "с ней случится что-то непоправимое.{x'",
                 time, GET_COUNT(time, "у", "ы", "") );
    }

    oldact(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) const
{
    oldact("$c1 говорит тебе '{GА предыдущий ты слома$Gло|л|ла?{x'", king, 0, hero, TO_VICT);
    act("%1$^C1 дает тебе новый %3$C4.", king, hero, mark,TO_VICT);
    oldact("$c1 дает $C3 новый $o4.", king, mark, hero, TO_NOTVICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) const
{
    act("%^C1 чмокает тебя в обе щеки.", king, hero, 0,TO_VICT);
    act("%1$^C1 чмокает %2$C4 в обе щеки.", king, hero, 0,TO_NOTVICT);
    act("%^C1 говорит тебе '{GИди скорее за наградой к тому, кто дал тебе задание!{x'.", king, hero, 0,TO_VICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) const
{
    if(number_percent( ) < 10)
        act("%^C1 ждет прекрасного принца.", kid, 0, 0,TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) const
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
        act("%1$^C1 потерянно озирается в поисках %2$C2.", kid, hero, 0,TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) const
{
    interpret( kid, "yell Я потерялась!!!" );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const
{
    act("%1$^C1 пытается положить деньги в %3$C4, но ничего не получается.", kid, 0, obj,TO_ROOM);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const
{
    act("%1$^C1 безо всякого интереса смотрит на %3$C4.", kid, 0, obj,TO_ROOM);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const
{
    interpret(kid, "hop");
    act("%^C1 радостно кричит '{YЭто бидончик моей мамочки!{x'", kid, 0, 0,TO_ROOM);
    act("%^C1 радостно кричит '{YОтведите меня к ней, пожалуйста!{x'", kid, 0, 0,TO_ROOM);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const
{
    act("%1$^C1 с громким ревом бросается на шею %2$C3. Как гадко...", kid, king, 0,TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) const
{
    act("{YВнезапно из засады выпрыгивает банда похитителей маленьких детей!{x", kid, 0, 0,TO_ROOM);
}

