/* $Id: scenario_cyclop.cpp,v 1.1.2.5.6.2 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario_cyclop.h"
#include "kidnapquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "act.h"
#include "interp.h"
#include "mercdb.h"
#include "handler.h"
#include "merc.h"
#include "def.h"

#define KS KidnapCyclopScenario

/*
 * Scenario by Ragnar
 */

/*------------------------------------------------------------------------------
 * Cyclop scenario, for evil cruel bastards 
 *----------------------------------------------------------------------------*/
bool KS::applicable( PCharacter *hero ) const
{
    return IS_EVIL(hero);
}

void KS::onQuestStart( PCharacter *hero, NPCharacter *questman, NPCharacter *king ) const
{
    tell_raw( hero, questman, 
              "{W%s{G зачем-то понадобилась твоя помощь.",
                   king->getNameP( '3' ).c_str() );
    tell_raw( hero, questman, 
             "Ищи %s в местности под названием {W%s{G ({W{hh%s{hx{G).",
                   GET_SEX(king, "его", "его", "ее"), king->in_room->getName(), king->in_room->areaName() );
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const
{
    interpret_raw( king, "grin" );
    act("%1$^C1 глядя на %2$C4 произносит: '{gО, вот и обед подоспел{x'.", king, 0, kid, TO_ROOM);
    act("%^C1 хватает дите за руку.", king, 0, 0, TO_ROOM);
    hero->printf( "%s и %s уже встретились.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    act("Сходи, проведай %2$C4.", hero, 0, king, TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) const
{
    if(hero == killer) {
        oldact("{YМолодец, убив злыдня ты соверши$gло|л|ла достойный поступок, он тебе зачтется... со временем.{x", killer, 0, 0, TO_CHAR);
        act("{YЗадание отменяется.{x", killer, 0, 0, TO_CHAR);
    } else {
        oldact("{Y$c1 жестоко убил страждующего злыдня, слава и почет $c3.{x", killer, 0, hero, TO_VICT);
        act("{YДля тебя же это означает, что: задание отменяется.{x", killer, 0, hero, TO_VICT);
    }
}
void KS::msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) const 
{
    if(hero == killer) 
        act("{YНет ребенка - нет обеда.{x\r\n{YЗадание отменяется.{x", killer, 0, 0, TO_CHAR);
    else 
        oldact("{Y$c1 с особым цинизмом уничтожи$gло|л|ла обед для упыря.{x\r\n{YЗадание отменяется.{x", killer, 0, hero, TO_VICT);
}

/*
 * bandit actions
 */
void KS::actAttackHero( NPCharacter *bandit, PCharacter *hero ) const 
{
    if (hero->fighting) 
        return;

    if (chance( 10 )) {
        act("%^C1 достает из-за пояса огромную ребристую скалку.", bandit, 0, 0, TO_ROOM);
        act("Раскатывающее движение скалкой %C2 {R<*) (*>= ! ПРЕВРАЩАЕТ В КРОВАВОЕ МЕСИВО ! =<*) (*>{x твое лицо", bandit, 0, hero, TO_VICT);
        act("Ты в {RУЖАСНОМ СОСТОЯНИИ.{x", bandit, 0, hero, TO_VICT);
        act("Раскатывающее движение скалкой %1$C2 {R<*) (*>= ! ПРЕВРАЩАЕТ В КРОВАВОЕ МЕСИВО ! =<*) (*>{x лицо %2$C2", bandit, 0, hero, TO_NOTVICT);
        act("%2$^C1 в {RУЖАСНОМ СОСТОЯНИИ.{x", bandit, 0, hero, TO_NOTVICT);
    }
    else {
        oldact("$c1 произносит '{gЗдравствуй мил$Gое|ок|ая, куда же это ты направил$Gось|ся|ась с чужим дитенком?{x'.", bandit, 0, hero, TO_ROOM);
        oldact("$c1 произносит '{gВидишь, что неразумное еще, так и ра$Gдо|д|да стараться?{x'.", bandit, 0, hero, TO_ROOM);
        oldact("$c1 произносит '{gНебось упырю какому-нибудь спихнуть захоте$Gло|л|ла? Ох как не хорошо...{x'.", bandit, 0, hero, TO_ROOM);
        act("%^C1 качает головой.", bandit, 0, hero, TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) const 
{
    oldact("$c1 берет за руку $C4 и уводит $S прочь.", bandit, 0, kid, TO_ROOM);
    act("%1$^C1 говорит %2$C3: '{gПойдем, драгоценный мой, а злыдня этого мы проучим.{x", bandit, 0, kid, TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) const 
{
    if(number_percent() < 10)
        act("%^C1 пристально осматривается по сторонам.", bandit, 0, 0, TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) const 
{
    if(number_percent() < 10)
        act("%2$^C1, держа %1$C4 за палец, тащит коня-качалку за собой.", bandit, 0, kid, TO_ROOM);
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) const 
{
    if(number_percent() < 10) {
        act("%^C1 горько вздыхает.", bandit, 0, 0, TO_ROOM);
        act("%^C1 произносит: '{gВот и все, пришли,... не туда{x'", bandit, 0, 0, TO_ROOM);
        
        if (kid->in_room == bandit->in_room)
            act("%2$^C1 готов расплакаться.", bandit, 0, kid, TO_ROOM);
    }
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) const 
{
    act("%^C1 оценивающе смотрит на тебя.", king, 0, hero, TO_VICT);
    act("%1$^C1 оценивающе смотрит на %2$C4.", king, 0, hero, TO_NOTVICT);
    interpret_raw(king, "sigh");
    oldact("$c1 говорит тебе '{GПрослыша$gло|л|ла я, что в {W{hh$t{hx{G есть один ребенок.{x", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
    act("%^C1 говорит тебе '{GЗамечательный экземплярчик...{x'", king, 0, hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) const 
{
    char buf[MAX_STRING_LENGTH];

    if(number_percent() < 50) {
        oldact("$c1 говорит тебе '{GЯ бы са$gмо|м|ма навести$gло|л|ла его, но голод ослабил меня.{x'", king, 0, hero, TO_VICT);
        act("%^C1 говорит тебе '{GВот, возьми эту деревяшку, думаю как приманка сработает.{x'", king, 0, hero, TO_VICT);
        act("%1$^C1 дает тебе %3$C4.", king, mark, hero, TO_VICT);
        oldact("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);
        act("%^C1 говорит тебе '{GПриведи его сюда.{x'", king, 0, hero, TO_VICT);
        sprintf( buf, "$c1 говорит тебе '{GИ поспеши! Чувствую, что через {Y%d{G минут%s он мне уже не понадобится.{x",
                 time, GET_COUNT(time, "у", "ы", "") );
    } else {
        oldact("$c1 говорит тебе '{GЯ бы и са$gмо|м|ма навести$gло|л|ла его, но жду гостей, надо многое приготовить, а времени нет.{x'", king, 0, hero, TO_VICT);
        act("%^C1 говорит тебе '{GВот, возьми эту деревяшку, думаю как приманка сработает.{x'", king, 0, hero, TO_VICT);
        act("%1$^C1 дает тебе %3$C4.", king, mark, hero, TO_VICT);
        oldact("$c1 вручает $C3 $o4.", king, mark, hero, TO_NOTVICT);
        sprintf( buf, "$c1 говорит тебе '{GПоспеши! Через {Y%d{G минут%s он должен быть здесь!{x",
                 time, GET_COUNT(time, "у", "ы", "") );
    }

    oldact(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) const 
{
    act("%^C1 говорит тебе '{GЛошадка приглянулась? Ладно, можешь оставить ее себе.{x'", king, 0, hero, TO_VICT);
    act("%1$^C1 дает тебе %3$C4.", king, mark, hero, TO_VICT);
    oldact("$c1 дает $C3 $o4.", king, mark, hero, TO_NOTVICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) const 
{
    oldact("$c1 говорит тебе '{GТы свое дело сдела$Gло|л|ла, свобод$Gно|ен|на.{x'.", king, 0, hero, TO_VICT);
    act("%^C1 выжидающе смотрит на тебя.", king, 0, hero, TO_VICT);
    act("%1$^C1 выжидающе смотрит на %2$C4.", king, 0, hero, TO_NOTVICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) const 
{
    if(number_percent( ) < 10)
        act("%^C1 вертит головой, не понимая, куда же он прискакал.", kid, 0, 0, TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) const 
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
        act("%1$^C1 вот-вот расплачется, потеряв %2$C4.", kid, 0, hero, TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) const 
{
    interpret_raw( kid, "yell", "ууууУУУААААА. Я К МАМЕ ХОЧУУУУ!!!!" );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object * ) const 
{
    act("%^C1 насупив нос отворачивается от тебя", kid, 0, victim, TO_VICT);
    act("%1$^C1 насупив нос отворачивается от %2$C4", kid, 0, victim, TO_NOTVICT);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object * ) const 
{
    act("%^C1 говорит тебе '{GЭто не моя лошадка.{x'", kid, 0, victim, TO_VICT);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const 
{
    interpret_raw(kid, "flip");
    act("%^C1, обняв лошадку, доверчиво смотрит тебе в глаза.", kid, 0, victim, TO_VICT);
    act("%1$^C1, обняв лошадку, доверчиво смотрит в глаза %2$C3.", kid, 0, victim, TO_NOTVICT);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const 
{
    interpret(king, "grin");
    act("%1$^C1 глядя на %2$C4 произносит: '{gО, вот и обед подоспел{x'.", king, 0, kid, TO_ROOM);
    act("%^C1 хватает дите за руку.", king, 0, 0, TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) const 
{
    act("{YТебя останавливают несколько добродушных старушек.{x", hero, 0, 0, TO_CHAR);
}
