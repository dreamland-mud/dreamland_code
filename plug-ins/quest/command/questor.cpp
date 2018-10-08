/* $Id: questor.cpp,v 1.1.2.2 2009/08/31 15:14:43 rufina Exp $
 *
 * ruffina, 2005
 */

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "skill.h"
#include "skillmanager.h"

#include "wiznet.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "mercdb.h"

#include "clan.h"
#include "clantypes.h"
#include "selfrate.h"
#include "occupations.h"

#include "languagemanager.h"

#include "quest.h"
#include "questregistrator.h"
#include "questexceptions.h"
#include "xmlattributequestdata.h"

#include "questor.h"
#include "def.h"

#define OBJ_VNUM_QUEST_SCROLL 28109

PROF(universal);

/*--------------------------------------------------------------------------
 * Questor
 *------------------------------------------------------------------------*/
Questor::Questor( ) 
{
}

int Questor::getOccupation( )
{
    return (1 << OCC_QUEST_MASTER);
}

bool Questor::canGiveQuest( Character *ach )
{
    return !ach->is_npc( );
}

void Questor::doRequest( PCharacter *client )  
{
    XMLAttributeQuestData::Pointer attr;
    DLString descr;
    int cha;
    
    act("$c1 просит $C4 дать $m задание.",client,0,ch,TO_ROOM);
    act("Ты просишь $C4 дать тебе задание.",client,0,ch,TO_CHAR);

    if (client->getAttributes( ).isAvailable( "quest" )) {
        tell_raw( client,ch,"Но у тебя уже есть задание!" );
        return;
    }

    attr = client->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" );
    
    if (attr->getTime( ) > 0) {
        tell_fmt( "Ты очень отваж%1$Gно|ен|на, %1$C1, но дай шанс кому-нибудь еще.", client, ch );
        tell_raw( client, ch, "Приходи позже." );
        return;
    }
    
    if (client->getDescription( )) {
        descr = client->getDescription( );
        descr.stripWhiteSpace( );
    }

    if (!IS_SET( client->act, PLR_CONFIRMED )) {
        tell_raw( client, ch, "Сначала попроси у Богов подтверждения своему персонажу.");
        tell_raw( client, ch, "Если не знаешь, как это делается, прочитай help confirm." );
        return;
    } else if (descr.empty( )) {
        tell_raw( client, ch, "Я не хочу давать задание такой непримечательной личности, как ты!");
        wiznet( WIZ_CONFIRM, 0, 0, "%C1 is confirmed but has no description!", client );
        return;
    } 
    
    cha = client->getCurrStat( STAT_CHA );
    
    if (cha < 20 && number_percent( ) < (20 - cha) * 5) {
        tell_raw( client, ch, "Знаешь, что-то душа не лежит давать тебе задание." );
        tell_raw( client, ch, "Приходи позже.");
        
        if (rated_as_guru( client ))
            attr->setTime( 1 );
        else
            attr->setTime( number_range(3, 6) );

        return;
    }
    
    tell_fmt( "Спасибо тебе, %1$C1!", client, ch );

    try {
        QuestManager::getThis( )->generate( client, ch );
        
        PCharacterManager::save( client );
        
        tell_raw( client, ch,  "Пусть удача сопутствует тебе!");

    } catch (const QuestCannotStartException &e) {
        tell_raw( client, ch, "Извини, но у меня сейчас нет для тебя задания.");
        tell_raw( client, ch, "Приходи позже.");
        
        if (rated_as_guru( client ))
            attr->setTime( 1 );
        else
            attr->setTime( number_range(3, 6) );
    }
}

void Questor::doComplete( PCharacter *client, DLString &args ) 
{
    ostringstream msg;
    int time;
    Quest::Reward::Pointer r;
    Quest::Pointer quest;
    XMLAttributes *attributes;
    XMLAttributeQuestData::Pointer qdata;
    bool fExpReward, fScrollGiven;
    DLString arg = args.getOneArgument( );

    act("$c1 информирует $C4 о выполнении задания.",client,0,ch,TO_ROOM);
    act("Ты информируешь $C4 о выполнении задания.",client,0,ch,TO_CHAR);

    attributes = &client->getAttributes( );
    quest = attributes->findAttr<Quest>( "quest" );

    if (!quest) {
        if (client->getAttributes( ).isAvailable( "quest" )) 
            tell_raw( client, ch, "Твое задание невозможно выполнить." );
        else
            tell_fmt( "Тебе нужно сначала получить (request) задание, %1$C1.", client, ch );
            
        return;
    }
   
    if (!quest->isComplete( )) {
        tell_raw( client, ch,  "Задание не выполнено! Но у тебя еще осталось немного времени!");
        return;
    }

    tell_raw( client, ch,  "Поздравляю с выполнением задания!" );

    if (quest->hint.getValue( ) > 0) {
        tell_raw( client, ch,  "Я припоминаю, что мне пришлось подсказать тебе путь.");
        msg << "Но за настойчивость я даю тебе";
    }
    else {
        msg << "В награду я даю тебе";
    }
    
    fExpReward = (!arg.empty( ) && (arg.strPrefix( "experience" ) || arg.strPrefix("опыт")));
    msg << " {Y%3$d{G "
        << (fExpReward ? "очк%3$Iо|а|ов опыта" : "квестов%3$Iую|ые|ых едини%3$Iцу|цы|ц")
        << " и {Y%4$d{G золот%4$Iую|ые|ых моне%4$Iту|ты|т.";
    
    r = quest->reward( client, ch );
    tell_fmt( msg.str( ).c_str( ), client, ch, 
              fExpReward ? r->exp : r->points,
              r->gold );

    client->gold += r->gold;

    if (fExpReward) {
        client->gainExp( r->exp );
    }
    else {
        client->questpoints += r->points;

        if (r->clanpoints > 0) {
            ClanData *cd = client->getClan( )->getData( );
            
            if (cd && cd->getBank( )) {
                tell_fmt( "Еще {Y%3$d{G квестов%3$Iая|ые|ых едини%3$Iца|цы|ц уходит на счет твоего клана.",
                          client, ch, r->clanpoints );
                
                cd->getBank( )->questpoints += r->clanpoints;
                cd->save( );
            }
        }
    }

    if (r->prac > 0) {
        tell_fmt( "Тебе повезло! Ты получаешь {Y%3$d{G сесси%3$Iю|и|й практики!",
                  client, ch, r->prac );
        client->practice += r->prac;
    }
    
    if (chance( r->wordChance ))
        rewardWord( client );
   
    if (client->getProfession( ) == prof_universal) 
        r->scrollChance *= 2;
    fScrollGiven = chance( r->scrollChance );
    if (fScrollGiven)
        rewardScroll( client );

    quest->wiznet( "complete", "%s = %d Gold = %d Prac = %d WordChance = %d ScrollChance = %d %s",
                   (fExpReward ? "Exp" : "Qp"), (fExpReward ? r->exp : r->points), 
                   r->gold, r->prac, r->wordChance, r->scrollChance, (fScrollGiven ? "" : "*") );
    
    time = quest->getNextTime( client );
    qdata = attributes->getAttr<XMLAttributeQuestData>( "questdata" );
    qdata->setTime( time );
    qdata->rememberVictory( quest->getName( ) );
    
    attributes->eraseAttribute( "quest" );
    PCharacterManager::save( client );
}


void Questor::doCancel( PCharacter *client )  
{
    int time;
    XMLAttributes *attributes;
    Quest::Pointer quest;
    
    act_p("$c1 просит $C4 отменить $S задание.",client,0,ch,TO_ROOM,POS_RESTING);
    act_p("Ты просишь $C4 отменить $S задание.",client,0,ch,TO_CHAR,POS_RESTING);
    
    attributes = &client->getAttributes( );
    quest = attributes->findAttr<Quest>( "quest" );
    
    if (!quest) {
        if (attributes->isAvailable( "quest" ))
            tell_raw( client, ch, "Твое задание невозможно отменить." );
        else
            tell_raw( client, ch,  "Но у тебя нет задания!");

        return;
    }
    
    if (rated_as_guru( client )) {
        tell_raw( client, ch, "Извини, %s, но это было бы слишком простым выходом для тебя.", client->getNameP( ) );
        return;
    }
    
    if ( client->questpoints < 3 )  {
        tell_raw( client, ch,  "У тебя недостаточно квестовых единиц для отмены задания.");
        return;
    }
    
    quest->wiznet( "cancel" );

    time = quest->getCancelTime( client );
    quest->setTime( client, time );
    attributes->eraseAttribute( "quest" );
    client->questpoints -= 3;
    PCharacterManager::save( client );

    tell_raw( client, ch,  "Ты теряешь {Y3{G квестовые единицы.");
    tell_fmt( "Через {Y%3$d{G мину%3$Iту|ты|т ты сможешь получить новое задание.",
              client, ch, time );
}
            
void Questor::doFind( PCharacter *client ) 
{
    ostringstream buf;
    Quest::Pointer quest;
    
    act_p("$c1 просит помощи у $C2.",client,0,ch,TO_ROOM,POS_RESTING);
    act_p("Ты просишь помощи у $C2.",client,0,ch,TO_CHAR,POS_RESTING);

    quest = client->getAttributes( ).findAttr<Quest>( "quest" );

    if (!quest) {
        if (client->getAttributes( ).isAvailable( "quest" ))
            tell_raw( client, ch, "Для невыполнимых заданий помощь не нужна." );
        else
            tell_raw( client, ch,  "Но у тебя нет задания.");

        return;
    }
     
    if (quest->help( client, ch )) 
        return;

    if (rated_as_guru( client )) {
        tell_fmt( "Извини, но тебе придется искать путь само%1$Gму|му|й.", client, ch);
        quest->wiznet( "find", "failure, guru mode" );
        return;
    }
    
    quest->helpMessage( buf );
    
    if (!makeSpeedwalk( ch->in_room, quest->helpLocation( ), buf )) 
    {
        tell_fmt( "Извини, %1$C1, но я ничем не могу тебе помочь.", client, ch );
        quest->wiznet( "find", "failure, broken path" );
        return;
    }

    tell_raw( client, ch, "Я помогу тебе, но награда будет не так велика.");
    tell_raw( client, ch, buf.str( ).c_str( ) );
    tell_raw( client, ch,  "Но помни! Все дороги в этом мире изменчивы и опасны.");
    tell_raw( client, ch,  "И не забывай открывать двери на своем пути.");
    
    quest->hint++;
    quest->wiznet( "find", "success, attempt #%d", quest->hint.getValue( ) );
}

bool Questor::canWander( Room *const room, EXIT_DATA *exit )
{
    return exit->u1.to_room->isCommon( );
}

bool Questor::canWander( Room *const room, EXTRA_EXIT_DATA *eexit )
{
    return true;
}

bool Questor::canWander( Room *const room, Object *portal )
{
    return true;
}

void Questor::rewardWord( PCharacter *client )
{
    Word word;
    
    languageManager->getRandomWord( word, client );

    if (!word.empty( )) {
        tell_raw( client, ch, 
                  "В награду я делюсь с тобой частицей древней мудрости "
                  "и сообщаю тебе слово {1{Y%s{2.", word.toStr( ) );
        ::wiznet( WIZ_LANGUAGE, 0, 0, "%^C1 узнает слово '%s' (%s).", client, word.toStr( ), word.effect.getValue( ).c_str( ) );
    }        
}

void Questor::rewardScroll( PCharacter *client )
{
    int sn, count;
    int learned, maximum;
    vector<int> skills;
    Object *scroll;
    QuestScrollBehavior::Pointer bhv;

    for (sn = 0; sn < skillManager->size( ); sn++) {
        Skill *skill = skillManager->find( sn );

        if (!skill->usable( client, false ))
            continue;
        
        learned = skill->getLearned( client );
        maximum = skill->getMaximum( client );
        
        if (learned >= maximum)
            continue;
        
        if (number_percent( ) > learned * 100 / maximum) 
            continue;

        skills.push_back( sn );
    }
    
    if (skills.empty( ))
        return;

    bhv.construct( );
    count = number_range( 1, 2 );

    for (sn = 0; sn < count && !skills.empty( ); sn++) {
        sn = number_range( 0, skills.size( ) - 1 );
        bhv->addSkill( skills[sn], number_range( 1, 3 ) );        
        skills.erase( skills.begin( ) + sn );
    }
    
    scroll = create_object( get_obj_index( OBJ_VNUM_QUEST_SCROLL ), 0 );
    scroll->behavior.setPointer( *bhv );
    bhv->setObj( scroll );
    bhv->setOwner( client );
    bhv->createDescription( client );

    obj_to_char( scroll, client );
    tell_raw( client, ch, "Кроме того, я вручаю тебе свиток, внимательно изучив который, "
                          "ты сможешь усовершенствовать свои умения." );
    act( "$C1 дает тебе $o4.", client, scroll, ch, TO_CHAR );
    act( "$C1 дает $c3 $o4.", client, scroll, ch, TO_ROOM );
}

/*--------------------------------------------------------------------------
 * QuestScrollBehavior 
 *------------------------------------------------------------------------*/
void QuestScrollBehavior::createDescription( PCharacter *ch )
{
    ostringstream bufInfo, bufEmpty, bufSkill;
    XMLMapBase<XMLInteger>::iterator s;
    
    bufEmpty << "Ты держишь в руках свиток из желтого пергамента, все надписи на котором размыты так, " << endl
             << "что невозможно что-либо разобрать." << endl;

    bufInfo << "Ты держишь в руках свиток из желтого пергамента, испрещенный загадочными значками." << endl
            << "Значки выведены аккуратным почерком, и, по-видимому, для их написания использовались особые чернила." << endl
            << "Из пометок рядом со значками ты понимаешь, что они содержат ";
            
    for (s = skills.begin( ); s != skills.end( ); s++) {
        Skill * skill = skillManager->findExisting( s->first );

        if (!skill) 
            continue;

        if (!bufSkill.str( ).empty( ))
            bufSkill << " и ";
        
        switch (number_range( 1, 3 )) {
        case 1: bufSkill << "секрет мастерства '" << skill->getNameFor( ch ) << "'"; break;
        case 2: bufSkill << "неизвестный тебе ранее трюк в искусстве '" << skill->getNameFor( ch ) << "'"; break;
        case 3: bufSkill << "кое-что новое о '" << skill->getNameFor( ch ) << "'"; break;
        }
    }

    if (bufSkill.str( ).empty( ))
        obj->addExtraDescr( obj->getName( ), bufEmpty.str( ) );
    else {
        bufInfo << bufSkill.str( ) << "." << endl;
        obj->addExtraDescr( obj->getName( ), bufInfo.str( ) );
    }
}

void QuestScrollBehavior::addSkill( int sn, int count )
{
    skills[skillManager->find( sn )->getName( )] = count;
}

void QuestScrollBehavior::setOwner( PCharacter *pch )
{
    ownerName = pch->getName( );
    ownerID = pch->getID( );
}

bool QuestScrollBehavior::isOwner( Character *ch ) const
{
    return !(ownerName.getValue( ) != ch->getName( ) 
            || ownerID.getValue( ) != ch->getID( )
            || ch->is_npc( ));
}

bool QuestScrollBehavior::hasTrigger( const DLString &t )
{
    return (t == "examine");
}

bool QuestScrollBehavior::examine( Character *ch )
{
    ostringstream buf, tmpbuf;
    Skill *skill;
    XMLMapBase<XMLInteger>::iterator s;
    
    if (!isOwner( ch )) {
        act("Знания, заключенные в $o6, недоступны тебе.", ch, obj, 0, TO_CHAR);
        return true;
    }
    
    act("Ты внимательно изучаешь знаки на $o6.", ch, obj, 0, TO_CHAR);
    
    for (s = skills.begin( ); s != skills.end( ); s++) {
        if (s->second <= 0)
            continue;

        skill = skillManager->findExisting( s->first );

        if (!skill) 
            continue;

        if (!skill->canPractice( ch->getPC( ), tmpbuf )) {
            buf << "Ты не можешь сейчас улучшить свои познания в '" << skill->getNameFor( ch ) << "'." << endl;
            continue;
        }

        PCSkillData &data = ch->getPC( )->getSkillData( skill->getIndex( ) );
        
        if (data.learned >= skill->getMaximum( ch )) {
            buf << "Искусство '" << skill->getNameFor( ch ) << "' уже изучено тобой в совершенстве." << endl;
        }
        else {
            buf << "Ты узнаешь кое-что новое об искусстве '" << skill->getNameFor( ch ) << "'!" << endl;
            data.learned = URANGE( data.learned.getValue( ), 
                                   data.learned + s->second,
                                   skill->getMaximum( ch ));
            s->second = 0;
        }
    }
    
    if (buf.str( ).empty( ))
        buf << "Похоже, знаки на этом свитке потеряли силу." << endl;

    ch->send_to( buf );
    return true;
}

