/* $Id: scenario_dragon.cpp,v 1.1.2.16.6.2 2009/01/18 20:11:58 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario_dragon.h"
#include "kidnapquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "act.h"
#include "clanreference.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define KS KidnapDragonScenario

CLAN(knight);

bool KS::applicable( PCharacter *hero ) const
{
    return !clan_knight->canInduct( hero );
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const
{
    oldact("$c1 бросается на шею $C3. Семейная сцена, сопли/слюни.", kid, 0, king, TO_ROOM);
    hero->printf( "%s и %s уже встретились.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    oldact("Приди к $C3 за благодарностью!", hero, 0, king, TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) const
{
    if(hero == killer) {
        oldact("{YИдио$gт|т|тка.... Ты уби$gло|л|ла того, кто нуждался в твоей помощи.{x", killer, 0, 0, TO_CHAR);
        hero->pecho("{YЗадание отменяется.{x");
    } else {
        act("{Y%C1 подло убил того, кто нуждался в твоей помощи.{x", killer, 0, hero, TO_VICT);
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
        oldact("$c1 сквозь зубы произносит: '{gДракону помогаешь? Может ты и са$Gмо|м|ма дракон?{x'.", bandit, 0, hero, TO_ROOM);
        act("%^C1 сквозь зубы произносит: '{gСейчас посмотрим, какого цвета у тебя кровь...{x'.", bandit, 0, 0, TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) const 
{
    oldact("$c1 одевает на $C2 ошейник и тащит за собой.", bandit, 0, kid, TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) const 
{
    if(number_percent() < 10)
        act("%^C1 задумчиво всматривается вдаль.", bandit, 0, 0, TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) const 
{
    if(number_percent() < 10)
        act("%^C1 злобно дергает драконенка за поводок.", bandit, 0, 0, TO_ROOM);
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) const 
{
    if(number_percent() < 10)
        act("%^C1 озадаченно оглядывается по сторонам.", bandit, 0, 0, TO_ROOM);
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) const 
{
    act("%^C1 говорит тебе '{GУ меня недавно встал на крыло дракончик.. не по годам рано.{x'", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{GОт радости он улетел так далеко, что, похоже, не может найти дороги обратно.{x'", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{GНайди его и верни, пока до него не добрались охотники за драконами.{x'", king, 0, hero, TO_VICT);
    oldact("$c1 говорит тебе '{GСкорее всего ты встретишь его в местности {W{hh$t{hx{G.{x'", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) const 
{
    char buf[MAX_STRING_LENGTH];
    
    oldact("$c1 вручает тебе $o4.", king, mark, hero, TO_VICT);
    oldact("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);

    act("%^C1 говорит тебе '{GПередай эту игрушку моему малышу, чтобы он знал, что тебе можно доверять.{x'", king, 0, hero, TO_VICT);
    sprintf( buf, "$c1 говорит тебе '{GПоторопись! У тебя будет всего {Y%d{G минут%s, чтобы вернуть его в целости и сохранности.{x'",
             time, GET_COUNT(time, "а", "ы", "") );
    oldact(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) const 
{
    oldact("$c1 дает тебе $o4.", king, mark, hero, TO_VICT);
    oldact("$c1 дает $C3 $o4.", king, mark, hero, TO_NOTVICT);
    act("%^C1 вздыхает в присутствии Еще Одного Идиота.", king, 0, 0, TO_ROOM);
    act("%^C1 говорит тебе '{GВ следующий раз будь повнимательнее.{x'", king, 0, hero, TO_VICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) const 
{
    act("%^C1 сердечно благодарит тебя.", king, 0, hero, TO_VICT);
    oldact("$c1 сердечно благодарит $C4.", king, 0, hero, TO_NOTVICT);
    oldact("$c1 говорит тебе: '{GДостой$Gное|ный|ная! Ступай за славой к тому, кто дал тебе задание!{x'.", king, 0, hero, TO_VICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) const 
{
    if(number_percent( ) < 10)
        act("%^C1 оглядывается по сторонам в поисках хоть чего-нибудь знакомого.", kid, 0, 0, TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) const 
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
        oldact("$c1 потерянно озирается в поисках $C2.", kid, 0, hero, TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) const 
{
    interpret( kid, "yell Я потерялся!!!" );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    oldact("$c1 безо всякого интереса смотрит на $o4.", kid, obj, 0, TO_ROOM);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    oldact("$c1 пытается пшикнуть себе в пасть из $o2.", kid, obj, 0, TO_ROOM);
    act("%^C1 разочарованно сопит.", kid, 0, 0, TO_ROOM);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    oldact("$c1 пшикает себе в пасть $o5. Из носа валит пар. \r\nРебенок счастлив.", kid, obj, 0, TO_ROOM);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const 
{
    oldact("$c1 бросается на шею $C3. Семейная сцена, сопли/слюни.", kid, 0, king, TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) const 
{
    oldact("{YВнезапно из засады выпрыгивает банда рыцарей-драконоборцев!{x", kid, 0, 0, TO_ROOM);
    oldact("В их глазах можно прочесть смешанные чувства: жадность, праведность, коварность и ненависть.", kid, 0, 0, TO_ROOM);
}
