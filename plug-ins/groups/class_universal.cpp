/* $Id: class_universal.cpp,v 1.1.2.3 2008/05/27 21:30:03 rufina Exp $
 *
 * ruffina, 2004
 */

#include "class_universal.h"
#include "regexp.h"
#include "logstream.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "dreamland.h"
#include "act.h"
#include "wiznet.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

PROF(none);
PROF(universal);
Profession * find_prof_unstrict( const DLString &className);

/*
 * paladin [7900]  the grand knight of paladins [7900]
 * samurai [9800]  самурай-мастер оружия [9594]
 * warrior [1221]  heimdall [1200]
 * cleric  [9604]  первосвященник [9581]
 * witch   [9623]  Эльвира [9579]
 * warlock [18103] Волшебник (Магазин Магических Товаров) [18206]
 * ninja   [16412] The guildmaster (Assassin's Meeting Chamber) [16099]
 * thief   [16385] The head thief [16088]
 * ranger  [234]   Лорд-Рейнджер [213]
 * anti-paladin [18116] Анти-паладин [18036]
 * necromancer [16135] The necromancer [16010]
 */
UniclassAdept::UniclassAdept( ) 
{
}

bool UniclassAdept::parseSpeech( Character *vict, const char *speech, DLString &className )
{
    PCharacter *victim;

    RegExp::MatchVector matches;
    static RegExp pattern1( "I (wish|want) .*become a?|я хочу стать"); 
    static RegExp pattern2( "([A-Za-zЮ-Ъю-ъ-]+)[.!]*$"); 
    
    if (vict->is_npc( ))
        return false;
    
    victim = vict->getPC( );

    if (victim->getProfession( ) != prof_universal)
        return false;

    if (!pattern1.match( speech ))
        return false;

    matches = pattern2.subexpr( speech );

    if (matches.size( ) < 1) {
        return true;
    }
    
    className = matches.front( );
    return true;
}

void UniclassAdept::tell( Character *vict, const char * msg ) 
{
    speech( vict, msg );
}

void UniclassAdept::speech( Character *vict, const char * speech ) 
{
    PCharacter *victim;
    DLString className;
    int cost, timeout;
    Profession *prof;
    XMLAttributeUniclass::Pointer attr;

    cost = 50;
    timeout = 24 * 60;

    if (!parseSpeech( vict, speech, className ))
        return;

    victim = vict->getPC( );
    if (className.empty( )) {
        act( "$c1 произносит '{gКем же ты хочешь стать, $C1?{x'", ch, 0, victim, TO_ALL );
        return;
    }
    
    prof = find_prof_unstrict( className );

    if (!prof) {
        act( "$c1 произносит '{gНикогда не слыша$gло|л|ла о такой странной профессии - '$t', $C1.{x'", ch, className.c_str( ), victim, TO_ALL );
        return;
    }
    
    if (prof_universal == prof) {
        interpret_fmt( ch, "bonk %s", victim->getNameP( ) );
        return;
    }
    
    className = prof->getName( );
    DLString rusName = prof->getRusName( );
    
    if (myclass.getValue( ) != className) {
        switch (number_range( 1, 3 )) {
        case 1: act( "$c1 произносит '{gЯ не могу сделать с тобой то, чего ты просишь, $C1.{x'", ch, className.c_str( ), victim, TO_ALL ); break;
        case 2: act( "$c1 произносит '{gТы ошиб$Gлось|ся|лась адресом. Уйди, не тревожь мой покой.{x'", ch, className.c_str( ), victim, TO_ALL ); break;
        case 3: act( "$c1 произносит '{g$C1! А ну-ка марш в гильдию и внимательно читай, что на стенах пишут!{x'", ch, 0, victim, TO_ALL ); break;
        }
        return;
    }
    
    if (!prof->getSex( ).isSetBitNumber( victim->getSex( ) )) {
        if (victim->getSex( ) == SEX_MALE) {
            act( "$c1 произносит '{gДля этого придется у тебя кое-что отрезать, $C1.{x'", ch, 0, victim, TO_ALL );
            interpret_fmt( ch, "giggle %s", victim->getNameP( ) );
        }
        else {
            act( "$c1 произносит '{gДля этого тебе не хватает одной маленькой штучки, $C1.{x'", ch, 0, victim, TO_ALL );
            interpret_fmt( ch, "smirk" );
        }
        
        return;
    }
    
    if (!prof->getAlign( ).isSetBitNumber( ALIGNMENT(victim) )) {
        if (IS_EVIL( victim ))
            act( "$c1 произносит '{gУ тебя нечистая душа, $C1. Тебе не место среди нас.{x'", ch, className.c_str( ), victim, TO_ALL );
        else if (IS_GOOD( victim )) {
            act( "$c1 презрительно кривится.", ch, 0, 0, TO_ALL );
            act( "$c1 произносит '{gУж больно ты хорош$Gое|ий|ая, $C1. Ступай-ка отсюда подобру-поздорову.{x'", ch, 0, victim, TO_ALL );
        }
        else
            act( "$c1 произносит '{gТы служишь силам равновесия. Тебе не место среди нас, $C1.{x'", ch, 0, victim, TO_ALL );
        
        return;
    }
    
    if (!prof->getEthos( ).isSetBitNumber( victim->ethos )) {
        act( "$c1 произносит '{gТвой этос не позволяет стать одн$Gим|им|ой из нас, $C1.{x'", ch, 0, victim, TO_ALL );
        return;
    }
    
    if (victim->getSubProfession( ) == prof) {
        act( "$c1 произносит '{gНо ты и так пытаешься изображать из себя $n4, $C1!{x'", ch, rusName.c_str( ), victim, TO_ALL );
        return;
    }
    
    if (victim->getSubProfession( ) == prof_none)
        cost = 0;

    if (victim->questpoints < cost) {
        act( "$c1 произносит '{gИзвини, $C1, но у тебя не хватает квестовых единиц.{x'", ch, 0, victim, TO_ALL );
        return;
    }
    
    attr = victim->getAttributes( ).getAttr<XMLAttributeUniclass>( "uniclass" );

    if (dreamland->getCurrentTime( ) - attr->lastTime < timeout) {
        switch (number_range( 1, 2 )) {
        case 1: act( "$c1 произносит '{g$C1, будешь так часто скакать между профессиями, голова закружится.{x'", ch, 0, victim, TO_ALL ); break;
        case 2: act( "$c1 произносит '{g$C1, твое время еще не пришло. Подожди немного.{x'", ch, 0, victim, TO_ALL ); break;
        }
        return;        
    }
    
    wiznet( WIZ_LEVELS, 0, 0 , "%^C1 changes uniclass to %s", victim, className.c_str( ) );

    attr->lastTime = dreamland->getCurrentTime( );
    attr->history[className]++;
    victim->questpoints -= cost;
    victim->setSubProfession( prof->getName( ) );
    victim->updateSkills( );
    victim->save( );
    
    act( "$c1 произносит '{gТы хочешь стать $n5 и я благословляю тебя на этот путь, $C1.{x'", ch, rusName.c_str( ), victim, TO_ALL ); 
    act( "$c1 произносит '{gОтныне ты сможешь овладевать знаниями нашей профессии.{x'", ch, 0, victim, TO_ALL ); 
    act( "$c1 произносит '{gНо помни, что истинного мастерства тебе будет достичь очень трудно.{x'", ch, 0, victim, TO_ALL ); 
}

/*
 * UniclassAdeptAndShopTrader
 */
UniclassAdeptAndShopTrader::~UniclassAdeptAndShopTrader( )
{
}

void UniclassAdeptAndShopTrader::speech( Character *vict, const char * speech ) 
{
    DLString className;
    if (parseSpeech( vict, speech, className ))
        UniclassAdept::speech( vict, speech );
    else
        ShopTrader::speech( vict, speech );
}

void UniclassAdeptAndShopTrader::tell( Character *vict, const char * speech ) 
{
    DLString className;
    if (parseSpeech( vict, speech, className ))
        UniclassAdept::tell( vict, speech );
    else
        ShopTrader::tell( vict, speech );
}

/* 
 * XMLAttributeUniclass 
 */

XMLAttributeUniclass::XMLAttributeUniclass( ) 
{
}

/*
 * DwarkinAdept
 */
DwarkinAdept::DwarkinAdept( ) 
{
}

void DwarkinAdept::tell( Character *victim, const char * msg ) 
{
    speech( victim, msg );
}

void DwarkinAdept::speech( Character *victim, const char * speech ) 
{
    PCharacter *pch;
    int cost, age;
    XMLAttributeEnlight::Pointer attr;
    static RegExp pattern( "^Я хочу быстре.* учить*ся|^I wish to learn quicker"); 

    if (victim->is_npc( ) || victim->getProfession( ) != prof_universal)
        return;
        
    if (!pattern.match( speech ))
        return;
    
    pch = victim->getPC( );
    cost = 1000;
    age = pch->age.getTrueYears( ) + 3;

    if (pch->getAttributes( ).isAvailable( "enlight" )) {
        act( "$c1 произносит '{gТы уже неплохо учишься, $C1.{x'", ch, 0, victim, TO_ALL );
        return;
    }

    if (pch->questpoints < cost) {
        act( "$c1 произносит '{gИзвини, $C1, но у тебя не хватает квестовых единиц.{x'", ch, 0, victim, TO_ALL );
        return;
    }
    
    attr = pch->getAttributes( ).getAttr<XMLAttributeEnlight>( "enlight" );
    attr->age = age;
    pch->questpoints -= cost;
    pch->save( );

    act( "$c1 дотрагивается до твоего лба.", ch, 0, victim, TO_VICT );
    act( "$c1 прикасается к голове $C2.", ch, 0, victim, TO_NOTVICT );
    interpret_raw( ch, "say", "Я даю тебе способность к быстрому обучению. Но она исчезнет, когда тебе исполнится {G%d{g %s.",
                       age, GET_COUNT(age, "год", "года", "лет") );
}

/* 
 * XMLAttributeEnlight 
 */

XMLAttributeEnlight::XMLAttributeEnlight( ) 
{
}

bool XMLAttributeEnlight::pull( PCharacter *pch ) 
{
    if (pch->age.getTrueYears( ) >= age) { 
        pch->send_to( "{GТы чувствуешь, что способность к быстрому обучению покидает тебя.{x\r\n" );
        return true;                
    }

    return false;
}

