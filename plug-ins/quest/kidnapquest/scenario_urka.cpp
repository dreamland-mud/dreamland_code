/* $Id: scenario_urka.cpp,v 1.1.2.9.6.2 2007/09/29 19:34:04 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario_urka.h"
#include "kidnapquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "act.h"
#include "mercdb.h"
#include "interp.h"
#include "merc.h"
#include "def.h"

#define KS KidnapUrkaScenario

/*
 * Scenarios by Ragnar
 */
/*------------------------------------------------------------------------------
 * Urka scenario, basic version (rude)
 *----------------------------------------------------------------------------*/

bool KS::applicable( PCharacter *hero ) const
{
    if (hero->ethos == ETHOS_LAWFUL)
        return false;
        
    if (IS_GOOD(hero) && hero->ethos == ETHOS_NEUTRAL)
        return false;

    return true;
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const 
{
    act("%2$^C1 поднимается навстречу %1$C3.", kid, 0, king, TO_ROOM);
    act("%2$^C1 внимательно смотрит на %1$C4.", kid, 0, king, TO_ROOM);
    oldact("$C1 произносит '{gНу, здравствуй, хорошо отдохну$gло|л|ла? Пора и за работу приниматься...{x'", kid, 0, king, TO_ROOM);
    hero->printf( "%s и %s уже встретились.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    act("Приди к %2$C3 за благодарностью!", hero, 0, king, TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) const 
{
    if(hero == killer) {
        oldact("{YТы уби$gло|л|ла того, кто просил тебя о помощи. Привет тебе от преступного мира...{x", killer, 0, 0, TO_CHAR);
        hero->pecho("{YЗадание отменяется.{x");
    } else {
        oldact("{Y$c1 подло убил того, кто нуждался в твоей помощи. Мафия не забудет $s.{x", killer, 0, hero, TO_VICT);
        hero->pecho("{YЗадание отменяется.{x");
    }
}
void KS::msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) const 
{
    if(hero == killer) {
        oldact("{YТы оказа$gло|л|ла неоценимую услугу всему миру, убив его!!!{x", killer, 0, 0, TO_CHAR);
        act("{YТем не менее, от тебя ожидали не этого...{x", killer, 0, 0, TO_CHAR);
        hero->pecho("{YЗадание отменяется.{x");
    } else {
        oldact("{Y$c1 подло уби$gло|л|ла того, кого тебе было поручено спасти.{x", killer, 0, hero, TO_VICT);
        oldact("{YБольшое спасибо $m за это, однако...{x", killer, 0, hero, TO_VICT);
        hero->pecho("{YЗадание отменяется.{x");
    }
}

/*
 * bandit actions
 */
void KS::actAttackHero( NPCharacter *bandit, PCharacter *hero ) const 
{
    if (!hero->fighting) {
        act("%^C1 произносит '{gА вас я попрошу задержаться для опознания вашего трупа...{x'.", bandit, 0, hero, TO_ROOM);
        act("%^C1 злобно ухмыляется.", bandit, 0, hero, TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) const 
{
    act("%1$^C1 защелкивает наручники на запястьях %2$C2 и тащит его прочь.", bandit, 0, kid, TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) const 
{
    if(number_percent() < 10)
        act("%^C1 достает из сапога огромное увеличительное стекло и вглядывается в следы.", bandit, 0, 0, TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) const 
{
    if(number_percent() < 10) {
        act("%^C1 рявкает: '{gНе оборачиваться!!!{x'", bandit, 0, 0, TO_ROOM);
        act("%1$^C1 пинком придает %2$C3 нужное направление.", bandit, 0, kid, TO_ROOM);
    }
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) const 
{
    if(number_percent() < 10) {
        act("%^C1 озабоченно чешет репу", bandit, 0, 0, TO_ROOM);
        act("%^C1 бормочет: '{gЗамуровали изверги...{x'", bandit, 0, 0, TO_ROOM);

        if (kid->in_room == bandit->in_room)
            act("%2$^C1 злобно ухмыляется.", bandit, 0, kid, TO_ROOM);
    }
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) const 
{
    act("%^C1 говорит тебе '{GХм, пожалуй, тебе можно доверить ответственное дельце. Слухай сюды:{x", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{Gнедавно мусора поганые ни за что, ни про что схапали моего братишку{x", king, 0, hero, TO_VICT);
    oldact("$c1 говорит тебе '{Gи держат, насколько мне известно, в одной из тюрем в местности {W{hh$t{hx{G. Не курорт однако...{x", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) const 
{
    char buf[MAX_STRING_LENGTH];

    if(number_percent() < 50) {
        act("%^C1 говорит тебе '{GНо это еще ладно, они хотят его повесить. Понимаешь, невиновного человека повесить!!!{x'", king, 0, hero, TO_VICT);
    } else {
        act("%^C1 говорит тебе '{GНо это еще ладно, представляешь, они говорят, что таких хороших людей{x'", king, 0, hero, TO_VICT);
        act("%^C1 говорит тебе '{Gдолжно быть много и хотят его четвертовать...{x'", king, 0, hero, TO_VICT);
    }

    act("%^C1 говорит тебе '{GНадо бы его выручить! Вот, у меня тут есть его паспорт,{x'", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{Gтак как в наше время без документа никуда, а заодно братишка поймет, что тебе можно доверять...{x'", king, 0, hero, TO_VICT);
    act("%1$^C1 вручает тебе %3$C4.", king, mark, hero, TO_VICT);
    oldact("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);
    sprintf( buf, "$c1 говорит тебе '{GПо моим подсчетам у тебя есть {Y%d{G минут%s, пока идут приготовления к казни. "
                  "Приведи его сюда.{x'", time, GET_COUNT(time, "а", "ы", "") );
    oldact(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) const 
{
    oldact("$c1 говорит тебе '{GЧто, потеря$Gло|л|ла документ?{x'", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{GТы думаешь у меня тут художественная мастеркая? Аккуратнее надо!!!{x'", king, 0, hero, TO_VICT);
    act("%1$^C1 дает тебе новый %3$C4.", king, mark, hero, TO_VICT);
    oldact("$c1 дает $C3 новый $o4.", king, mark, hero, TO_NOTVICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) const 
{
    oldact("$c1 хлопает тебя по плечу так, что ты теряешь равновесие и падаешь мордой прямо $m на сапог.", king, 0, hero, TO_VICT);
    act("Причем тебе кажется, что сапог движется навстречу.", king, 0, hero, TO_VICT);
    oldact("$c1 так хлопает $C4 по плечу, что $E теряет равновесие и падает мордой $m на сапог.", king, 0, hero, TO_NOTVICT);
    act("Причем сапог явно движется навстречу.", king, 0, hero, TO_NOTVICT);
    act("%^C1 говорит тебе '{GМолодец, вернись к давшему тебе задание!{x'.", king, 0, hero, TO_VICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) const 
{
    if(number_percent( ) < 10)
        act("%^C1 скучающим взором окидывает камеру.", kid, 0, 0, TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) const 
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
        act("%1$^C1 чешет репу в попытке сообразить, куда же делся этот %2$C1.", kid, 0, hero, TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) const 
{
    interpret_fmt( kid, "yell ЭЙ, дюже хорошо ты ластами работаешь, грыбы обратно, отстал%s я!!!", GET_SEX(kid, "", "о", "а") );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    act("%^C1 говорит тебе '{gЭээ нет, мой братуха конечно не очень разборчив в персонале,{x'", kid, 0, victim, TO_VICT);
    act("%^C1 говорит тебе '{gно такого хмыря как ТЫ, даже он не послал бы на выручку.{x'", kid, 0, victim, TO_VICT);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    act("%^C1 говорит тебе '{gТы чего мне принес? Совсем мозги в боях повышибали?{x'", kid, 0, victim, TO_VICT);
    act("%^C1 говорит тебе '{gТы на меня посмотри и на эту батву в ксиве... Нормальный докУмент неси.{x'", kid, 0, victim, TO_VICT);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    act("%^C1 недовольно бурчит.", kid, 0, 0, TO_ROOM);
    act("%^C1 произносит '{gДаа, узнаю почерк... Такие картины только мой братан может рисовать.{x'", kid, 0, 0, TO_ROOM);
    act("%^C1 произносит '{gНу да ладно, пойдем, глядишь прорвемся.{x'", kid, 0, 0, TO_ROOM);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const 
{
    act("%2$^C1 поднимается навстречу %1$C3.", kid, 0, king, TO_ROOM);
    act("%2$^C1 внимательно смотрит на %1$C4.", kid, 0, king, TO_ROOM);
    oldact("$C1 произносит '{gНу, здравствуй, хорошо отдохну$gло|л|ла? Пора и за работу приниматься...{x'", kid, 0, king, TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) const 
{
    act("{YВдруг откуда ни возьмись появляется отряд охотников на преступников!{x", kid, 0, 0, TO_ROOM);
    act("%^C1 произносит '{gСержант Петренко, трое детей, предъявите документы{x'.", bandit, 0, 0, TO_ROOM);
    act("%^C1 долго и усердно изучает паспорт.", bandit, 0, 0, TO_ROOM);
    act("%1$^C1 поднимает взгляд на %2$C4 и произносит: '{gКак хорошо, вас-то мы и ищем...{x'", bandit, 0, kid, TO_ROOM);
    act("%2$^C1 озабоченно бормочет: '{gЧей же паспорт этот идиот мне принес?{x'", bandit, 0, kid, TO_ROOM);
}

#undef KS


/*------------------------------------------------------------------------------
 * Urka scenario, more polite version
 *----------------------------------------------------------------------------*/

#define KS KidnapUrkaPoliteScenario

void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) const 
{
    act("%^C1 говорит тебе '{GХм, пожалуй тебе можно доверить ответственное дельце, слушай:'{x", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{GСовсем недавно под внеочередную облаву попал один из моих лучших головор... товарищей то есть.'{x", king, 0, hero, TO_VICT);
    act("%^C1 говорит тебе '{GУ НИХ видите ли указ вышел, чтобы камеры не пустовали.'{x", king, 0, hero, TO_VICT);
    oldact("$c1 говорит тебе '{GВот в одной из тюрем его и держат, по моим сведениям где-то в {W{hh$t{hx{G.'{x", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) const 
{
    char buf[MAX_STRING_LENGTH];

    if(number_percent() < 50) {
        act("%^C1 говорит тебе '{GВсе бы ничего, но какой-то тип, очень похожий на моего товарища, видать так сильно насолил этой Фемиде, что она аж окаменела от ужаса.{x'", king, 0, hero, TO_VICT);
        act("%^C1 говорит тебе '{GТак и стоит в своем храме, а ее доблестные служители прямо взбесились и без всякого следствия хотят вздернуть моего лучшего голов... товарища.'{x", king, 0, hero, TO_VICT);
        act("%^C1 говорит тебе '{GНадо бы его выручить! Вот, у меня тут есть его паспорт, так как в наше время без документа тяжело, а заодно он поймет, что тебе можно доверять.'{x'", king, 0, hero, TO_VICT);
    } else {
        oldact("$c1 говорит тебе '{GНо это только половина беды, представляешь, ОНИ говорят, что таких "
            "хороших людей должно быть много и хотят его четвертовать...{x'", king, 0, hero, TO_VICT);
        oldact("$c1 говорит тебе '{GНадо бы помочь товарищу! Вот, у меня есть его паспорт, так как в наше "
             "время без документа никуда, а заодно он поймет, что тебе можно доверять.{x'", king, 0, hero, TO_VICT);
    }

    act("%1$^C1 вручает тебе %3$C4.", king, mark, hero, TO_VICT);
    oldact("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);
    sprintf( buf, "$c1 говорит тебе '{GУ тебя есть примерно {W%d{G минут%s, пока идут приготовления к казни. "
                  "Приведи его ко мне.{x'",
             time, GET_COUNT(time, "а", "ы", "") );
    oldact(buf, king, 0, hero, TO_VICT);
}

void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const
{
    act("%^C1 говорит тебе '{gТы еще кто такой? Давай, давай, топай отсюда,{x'", kid, 0, victim, TO_VICT);
    act("%^C1 говорит тебе '{gНе хватало того, чтобы меня застукали в твоей кампании.{x'", kid, 0, victim, TO_VICT);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const
{
    act("%^C1 говорит тебе '{gЧто ты мне суешь? Все умы по дороге растерял?{x'", kid, 0, victim, TO_VICT);
    act("%^C1 говорит тебе '{gТы на меня посмотри и на то, что в паспорте нарисовано...{x'", kid, 0, victim, TO_VICT);
    act("%^C1 говорит тебе '{gНормальный документ неси.{x'", kid, 0, victim, TO_VICT);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) const 
{
    interpret_fmt( kid, "yell ЭЙ, ты где? Возвращайся, отстал%s я!!!", GET_SEX(kid, "", "о", "а") );
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    act("%^C1 ухмыляется.", kid, 0, 0, TO_ROOM);
    act("%^C1 произносит '{gДаа, узнаю почерк... Такие картины только мой шеф рисовать умеет.{x'", kid, 0, 0, TO_ROOM);
    act("%^C1 произносит '{gЧто ж, пойдем. Попробуем прорваться.{x'", kid, 0, 0, TO_ROOM);
}

#undef KS
