/* $Id: questor.cpp,v 1.1.2.2 2009/08/31 15:14:43 rufina Exp $
 *
 * ruffina, 2005
 */
#include "profiler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "skill.h"
#include "skillmanager.h"
#include "bonus.h"
#include "wiznet.h"
#include "skill_utils.h"
#include "merc.h"
#include "../../anatolia/handler.h"
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
#include "objquestbehavior.h"

#include "questor.h"
#include "def.h"

#define OBJ_VNUM_QUEST_SCROLL 28109

RELIG(fili);
BONUS(experience);

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

void Questor::doComplete( PCharacter *client, DLString &args ) 
{
    int time;
    QuestReward::Pointer reward;
    Quest::Pointer quest;
    XMLAttributes *attributes;
    XMLAttributeQuestData::Pointer qdata;
    DLString arg = args.getOneArgument( );

    oldact("$c1 информирует $C4 о выполнении задания.",client,0,ch,TO_ROOM);
    oldact("Ты информируешь $C4 о выполнении задания.",client,0,ch,TO_CHAR);

    attributes = &client->getAttributes( );
    quest = attributes->findAttr<Quest>( "quest" );

    if (!quest) {
        if (client->getAttributes( ).isAvailable( "quest" )) 
            tell_raw( client, ch, "Твое задание невозможно выполнить." );
        else
            tell_fmt( "Тебе нужно сначала {yпопросить{x{le (quest request){x задание, %1$C1.", client, ch );
            
        return;
    }
   
    if (!quest->isComplete( ) && !quest->hasPartialRewards()) {
        tell_raw( client, ch,  "Задание не выполнено! Но у тебя еще осталось немного времени!");
        return;
    }

    tell_raw( client, ch,  "Поздравляю с выполнением задания!" );

    reward = quest->reward( client, ch );

    // Issue reward only for quests that allow players to gain quest points.
    if (reward->points > 0) {
        ostringstream buf;

        // Handle 'quest complete exp' syntax.
        bool fExpReward = (!arg.empty( ) && (arg.strPrefix( "experience" ) || arg.strPrefix("опыт")));
        if (fExpReward)
            reward->points = 0;
        else
            reward->exp = 0;

        // Apply altar and calendar exp bonuses.
        if (reward->exp > 0 && bonus_experience->isActive(client, time_info)) {
            int xp = reward->exp;
            reward->exp = number_range(xp + xp / 2, xp * 2);
            bonus_experience->reportAction(client, buf);
        }

        giveReward(client, quest, reward);
        client->send_to(buf);

        quest->wiznet( "complete", "%s = %d Gold = %d Prac = %d WordChance = %d ScrollChance = %d",
                    (fExpReward ? "Exp" : "Qp"), (fExpReward ? reward->exp : reward->points), 
                    reward->gold, reward->prac, reward->wordChance, reward->scrollChance);
    } else {
        quest->wiznet("complete", "");
    }
    
    time = quest->getNextTime( client );
    qdata = attributes->getAttr<XMLAttributeQuestData>( "questdata" );
    qdata->setTime( time );
    qdata->rememberVictory( quest->getName( ) );
    qdata->rememberLastQuest(quest->getName());
    
    attributes->eraseAttribute( "quest" );
    PCharacterManager::save( client );
}

void Questor::giveReward(PCharacter *client, Quest::Pointer &quest, QuestReward::Pointer &r)
{
    ostringstream msg;

    if (quest->hint.getValue( ) > 0 && !IS_TOTAL_NEWBIE(client)) {
        tell_raw( client, ch,  "Я припоминаю, что мне пришлось подсказать тебе путь.");
        msg << "Но за настойчивость я даю тебе";
    }
    else {
        msg << "В награду я даю тебе";
    }
    
    msg << " {Y%3$d{G "
        << (r->exp > 0 ? "очк%3$Iо|а|ов опыта" : "квестов%3$Iую|ые|ых едини%3$Iцу|цы|ц")
        << " и {Y%4$d{G золот%4$Iую|ые|ых моне%4$Iту|ты|т.";
    
    tell_fmt( msg.str( ).c_str( ), client, ch, 
              r->exp > 0 ? r->exp : r->points,
              r->gold );

    if (client->getReligion() == god_fili && get_eq_char(client, wear_tattoo)) {
        int bonus = r->gold;
        tell_fmt("{YФили{G просил передать тебе еще {Y%3$d{G золот%3$Iую|ые|ых моне%3$Iту|ты|т.", client, ch, bonus);
        client->gold += bonus;
    }

    client->gold += r->gold;

    if (r->exp > 0) {
        client->gainExp( r->exp );
    }
    else {
        client->addQuestPoints(r->points);

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
   
    if (chance( r->scrollChance ))
        rewardScroll( client );
}

void Questor::doCancel( PCharacter *client )  
{
    int time;
    XMLAttributes *attributes;
    Quest::Pointer quest;
    
    oldact("$c1 просит $C4 отменить $S задание.",client,0,ch,TO_ROOM);
    oldact("Ты просишь $C4 отменить $S задание.",client,0,ch,TO_CHAR);
    
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
        tell_raw( client, ch, "Извини, %s, но это было бы слишком простым выходом для тебя.", client->getNameC() );
        return;
    }
    
    if ( client->getQuestPoints() < 3 )  {
        tell_raw( client, ch,  "У тебя недостаточно квестовых единиц для отмены задания.");
        return;
    }
    
    quest->wiznet( "cancel" );

    time = quest->getCancelTime( client );
    quest->setTime( client, time );
    attributes->eraseAttribute( "quest" );
    client->addQuestPoints(-3);
    PCharacterManager::save( client );

    tell_raw( client, ch,  "Ты теряешь {Y3{G квестовые единицы.");
    tell_fmt( "Через {Y%3$d{G мину%3$Iту|ты|т ты сможешь получить новое задание.",
              client, ch, time );
}
            
void Questor::doFind( PCharacter *client ) 
{
    ostringstream buf;
    Quest::Pointer quest;
    
    oldact("$c1 просит помощи у $C2.",client,0,ch,TO_ROOM);
    oldact("Ты просишь помощи у $C2.",client,0,ch,TO_CHAR);

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

    if (quest->hint >= 3 && !IS_TOTAL_NEWBIE(client)) {
        tell_fmt( "Извини, %1$C1, но теперь тебе придется искать путь самостоятельно.", client, ch );
        quest->wiznet( "find", "failure, too many hints" );
        return;
    }

    quest->helpMessage( buf );
    
    if (!makeSpeedwalk( ch->in_room, quest->helpLocation( ), buf )) 
    {
        tell_fmt( "Извини, %1$C1, но я ничем не могу тебе помочь.", client, ch );
        quest->wiznet( "find", "failure, broken path" );
        return;
    }

    if(!IS_TOTAL_NEWBIE(client))
    tell_raw( client, ch, "Я помогу тебе, но награда будет не так велика.");
    tell_raw( client, ch, buf.str( ).c_str( ) );
    tell_raw( client, ch,  "Но помни! Все дороги в этом мире изменчивы и опасны.");
    tell_raw( client, ch,  "И не забывай открывать двери на своем пути.");
    
    quest->hint++;
    quest->wiznet( "find", "success, attempt #%d", quest->hint.getValue( ) );
}

bool Questor::canWander( Room *const room, EXIT_DATA *exit )
{
    if (!exit->u1.to_room->isCommon())
        return false;
        
    if (IS_SET(exit->u1.to_room->room_flags, ROOM_NO_QUEST))
        return false;

    return true;
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
    ostringstream report;
    int sn, i, count;
    int learned, maximum;
    vector<int> skills;
    Object *scroll;
    QuestScrollBehavior::Pointer bhv;

    for (sn = 0; sn < skillManager->size( ); sn++) {
        Skill *skill = skillManager->find( sn );

        if (!skill->usable( client, false ))
            continue;

        if (temporary_skill_active(skill, client))
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

    for (i = 0; i < count && !skills.empty( ); i++) {
        sn = number_range( 0, skills.size( ) - 1 );
        int gain = number_range(2, 4);
        bhv->addSkill(skills[sn], gain);
        report << skillManager->find(skills[sn])->getName() << " (" << gain << "%) ";
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
    oldact("$C1 дает тебе $o4.", client, scroll, ch, TO_CHAR );
    oldact("$C1 дает $c3 $o4.", client, scroll, ch, TO_ROOM );
    ::wiznet( WIZ_QUEST, 0, 0, "%^C1 получает свиток познания для умения %s", client, report.str().c_str());
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

    bufInfo << "Ты держишь в руках свиток из желтого пергамента, испещренный загадочными значками." << endl
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
    bool extract = true;
    
    if (!isOwner( ch )) {
        oldact("Знания, заключенные в $o6, недоступны тебе.", ch, obj, 0, TO_CHAR);
        return true;
    }
    
    oldact("Ты внимательно изучаешь знаки на $o6.", ch, obj, 0, TO_CHAR);
    
    for (s = skills.begin( ); s != skills.end( ); s++) {
        if (s->second <= 0)
            continue;

        skill = skillManager->findExisting( s->first );

        if (!skill) 
            continue;

        if (!skill->canPractice( ch->getPC( ), tmpbuf )) {
            buf << "Ты не можешь сейчас улучшить свои познания в '" << skill->getNameFor( ch ) << "'." << endl;
            extract = false;
            continue;
        }

        PCSkillData &data = ch->getPC( )->getSkillData( skill->getIndex( ) );
        
        if (data.learned >= skill->getMaximum( ch )) {
            buf << "Искусство '" << skill->getNameFor( ch ) << "' уже изучено тобой в совершенстве." << endl;
            extract = false;
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
    if(extract) {
        oldact("Чернила меркнут, и $o1 рассыпается трухой.", ch, obj, 0, TO_CHAR);
        extract_obj( obj );
    }
    return true;
}

static void delay_noquest(XMLAttributeQuestData::Pointer &attr, PCharacter *client)
{
    if (rated_as_guru( client ))
        attr->setTime( 1 );
    else
        attr->setTime( number_range(3, 6) );
}

void Questor::doRequest(PCharacter *client, const DLString &arg)  
{
    ProfilerBlock pb("quest request");
    XMLAttributeQuestData::Pointer attr;
    DLString descr;
    
    if (arg.empty() || arg_is_list(arg)) {
        oldact("$c1 просит $C4 показать список заданий.",client, 0, ch, TO_ROOM);
        oldact("Ты просишь $C4 показать список заданий.",client, 0, ch, TO_CHAR);
    } else {
        oldact("$c1 просит $C4 дать $m задание.", client, 0, ch, TO_ROOM);
        oldact("Ты просишь $C4 дать тебе задание.", client, 0, ch, TO_CHAR);
    }

    if (client->getAttributes( ).isAvailable( "quest" )) {
        tell_raw(client, ch, "Но у тебя уже есть задание!");
        return;
    }

    attr = client->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" );
    
    if (attr->getTime( ) > 0) {
        tell_fmt( "Ты очень отваж%1$Gно|ен|на, %1$C1, но дай шанс кому-нибудь еще.", client, ch );
        tell_fmt( "Приходи через {Y%3$d{G мину%3$Iту|ты|т, когда истечет твое {y{hcквест время{x.", client, ch, attr->getTime() );
        return;
    }
    
    if (client->getDescription( )) {
        descr = client->getDescription( );
        descr.stripWhiteSpace( );
    }

    if (!IS_SET( client->act, PLR_CONFIRMED )) {
        if (attr->getAllVictoriesCount() > 20) {
            tell_raw( client, ch, "Попроси у богов подтверждения своему персонажу, чтобы продолжить выполнять задания.");
            tell_raw( client, ch, "Если не знаешь, как это делается, прочитай {y{hc{lRсправка подтверждение{lEhelp confirm{x." );
            return;
        }
    } else if (descr.empty( )) {
        tell_raw( client, ch, "Я не хочу давать задание такой непримечательной личности, как ты!");
        wiznet( WIZ_CONFIRM, 0, 0, "%C1 is confirmed but has no description!", client );
        return;
    } 
   
    if (client->getQuestPoints() > 0) { 
        int cha = client->getCurrStat( STAT_CHA );
        
        if (cha < 20 && number_percent( ) < (20 - cha) * 5) {
            tell_raw( client, ch, "Знаешь, что-то душа не лежит давать тебе задание." );
            delay_noquest(attr, client);        
            tell_fmt( "Приходи через {Y%3$d{G мину%3$Iту|ты|т.", client, ch, attr->getTime() );
            hint_fmt(client, "Квестор может отказаться выдавать тебе задание, если у тебя слишком низкая {hhхаризма{x.");
            hint_fmt(client, "Харизму, как и другие параметры, можно поднять вещами или {hh1353тренировками{x.");
            return;
        }
    }
  
    if (arg.empty() || arg_is_list(arg)) {
        QuestList quests = QuestManager::getThis()->list(client);
        if (quests.empty()) {
            tell_raw(client, ch, "Извини, но у меня не нашлось ни одного подходящего для тебя задания.");
            delay_noquest(attr, client);        
            tell_fmt( "Приходи через {Y%3$d{G мину%3$Iту|ты|т.", client, ch, attr->getTime() );            
            return;
        }

        tell_fmt("Спасибо тебе, %1$C1!", client, ch);
        tell_raw(client, ch, "Вот какие поручения я могу тебе сегодня дать:");
        
        QuestList::const_iterator q;
        int index;
        ostringstream buf;
        buf << endl;
        for (index = 1, q = quests.begin(); q != quests.end(); index++, q++) {
            DLString d = (*q)->getDifficulty();
            buf << dlprintf("     {W%2d{x. %-25s {D(%s){x \r\n",
                            index, (*q)->getShortDescr().c_str(), d.c_str());
        }
        buf << endl;
        client->send_to(buf);

        tell_raw(client, ch, "Подробнее о каждом из них ты можешь прочитать в справке по теме '{W{hhвиды заданий{hx{G'.");
        tell_raw(client, ch, "Выбери поручение и укажи его номер, например {y{lRзадание просить 3{lEquest request 3{x.");
        tell_raw(client, ch, "Или же попроси у меня задание на мое усмотрение: {y{lRзадание просить любое{lEquest request any{x.");
        return;
    }

    if (arg_oneof_strict(arg, "any", "любое", "любой")) {
        try {
            QuestManager::getThis( )->generate( client, ch );
            attr->setStartTime();
            PCharacterManager::save( client );

            if(!rated_as_guru(client)){
               tell_raw(client, ch, "Если не сможешь справиться - попроси у меня подсказку командой {y{hc{lRзадание найти{lEquest find{x.");
            if(IS_TOTAL_NEWBIE(client)){
               tell_raw(client, ch, "Для новичков {x({y{hc{lRсправка яесть{lEhelp selfrate{x){G, живущих первую жизнь {x({y{hc{lRсправка перерождение{lEhelp remort{x){G, это бесплатно!");
            }
            }
            
            tell_raw(client, ch,  "Пусть удача сопутствует тебе!");

        } catch (const QuestCannotStartException &e) {
            tell_raw(client, ch, "Извини, но я не могу сейчас выбрать для тебя задание.");
            delay_noquest(attr, client);        
            tell_fmt( "Приходи через {Y%3$d{G мину%3$Iту|ты|т.", client, ch, attr->getTime() );            
        }
        return;
    }

    QuestRegistratorBase::Pointer q = parseQuestArgument(client, arg);
    if (!q)
        return;

    if (attr->getLastQuestCount((*q)->getName()) >= 5) {
        tell_fmt("Я уже заметил%2$Gо||а, что тебе очень нравятся такие задания, %1$C1.", client, ch);
        tell_raw(client, ch, "Но попробуй добиться успеха в чем-то еще.");
        return;
    }

    for (int i = 0; i < 3; i++) {
        try {
            client->getAttributes().addAttribute(
                (*q)->createQuest(client, ch), "quest");
            attr->setStartTime();
            PCharacterManager::save(client);

            if (!rated_as_guru(client)) {
                tell_raw(client, ch, "Если не сможешь справиться - попроси у меня подсказку командой {y{hc{lRзадание найти{lEquest find{x.");
                if (IS_TOTAL_NEWBIE(client)) {
                    tell_raw(client, ch, "Для новичков {x({y{hc{lRсправка яесть{lEhelp selfrate{x){G, живущих первую жизнь {x({y{hc{lRсправка Перерождение{lEhelp remort{x){G, это бесплатно!");
                }
            }

            tell_raw(client, ch, "Пусть удача сопутствует тебе!");            
            return;

        } catch (const QuestCannotStartException &e) {
        }
    }

    tell_fmt("Извини, оказывается у меня нет подходящих для тебя заданий на '%3$s'.", client, ch, (*q)->getShortDescr().c_str());
    tell_raw(client, ch, "Приходи позже или выбери что-то другое.");
}

/** Return a quest type that player has requested with "q request <arg>" command */
QuestRegistratorBase::Pointer Questor::parseQuestArgument(PCharacter *client, const DLString &arg)
{
    QuestRegistratorBase::Pointer result, null;
    QuestList quests = QuestManager::getThis()->list(client);

    // Player chose quest type by its number.
    Integer choice;
    if (Integer::tryParse(choice, arg)) {
        if (choice <= 0 || choice > (int)quests.size()) {
            tell_fmt("Я не предлагал%2$Gо||а тебе задание под таким номером, будь внимательнее!", client, ch);
            return null;
        }

        int index;
        QuestList::const_iterator q;
        for (index = 1, q = quests.begin(); index != choice; index++, q++)
            ;

        result = *q;
        return result;
    }
    
    // Player is probably calling quest type by its name.    
    for (auto &q: quests) {
        
        if (is_name(arg.c_str(), q->getName().c_str()) || is_name(arg.c_str(), q->getShortDescr().c_str())) {
            
            // No ambiguity allowed.
            if (result) {
                tell_fmt("У меня есть несколько заданий с таким названием, %1$C1. Скажи конкретнее, что ты хочешь?", client, ch);
                tell_fmt("Например, {1{y{hcзадание попросить %3$s{hx{2 или {1{y{hcзадание попросить %4$s{hx{2.",
                        client, ch, result->getShortDescr().c_str(), q->getShortDescr().c_str());
                return null;
            }

            result = *q;
        }
    }

    if (!result)
        tell_fmt("Я не умею читать мысли, %1$C1. Укажи номер задания, название или {1{Y{lRлюбое{lEany{lx{2.", client, ch);

    return result;
}

void Questor::give( Character *victim, Object *obj )
{
    obj_from_char(obj);
    obj_to_char(obj, victim);

    if (!victim->is_npc() && obj->behavior && obj->behavior.getDynamicPointer<ObjQuestBehavior>()) {
        tell_fmt("Нет нужды передавать %3$O4 мне в руки.", victim, ch, obj);
        tell_fmt("Если ты закончил%1$Gо||а мое задание, просто набери {y{hc{lRзадание сдать{lEquest complete{x.", victim, ch);
    }

    victim->pecho("%^C1 возвращает тебе %O4.", ch, obj);
    victim->recho(ch, "%^C1 возвращает %O4 %C3.", ch, obj, victim);
}
