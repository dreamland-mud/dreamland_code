/* $Id: cclan.cpp,v 1.1.6.15.4.15 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 * based on CClan by NoFate, 2001
 */

#include <sstream>
#include <iomanip>
#include <iostream>

#include "class.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "pcharactermemory.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "race.h"

#include "merc.h"
#include "clanreference.h"
#include "wearloc_utils.h"
#include "loadsave.h"
#include "messengers.h"
#include "act.h"

#include "screenreader.h"

#include "clantypes.h"
#include "clantitles.h"
#include "clanorg.h"
#include "cclan.h"
#include "clanrecords.h"
#include "xmlattributeinduct.h"
#include "msgformatter.h"
#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "reglist.h"
#include "def.h"
#include "l10n.h"

CLAN(none);

using namespace std;

/* Width of the fixed-width clan cell in the language being rendered. Russian
 * measures the authored padName, so its column keeps the width it has always
 * had; the other languages size themselves to their own longest short name --
 * every Russian and Ukrainian one fits 11 columns, but English needs 15 for
 * "Flower Children". */
static size_t clanColumnWidth( lang_t lang )
{
    size_t width = 0;
    ClanManager *cm = ClanManager::getThis( );

    for (int i = 0; i < cm->size( ); i++) {
        Clan *clan = cm->find( i );
        size_t len = (lang == LANG_RU
                        ? clan->getPaddedName( ).colourStrip( ).size( )
                        : clan->getShortFor( lang ).size( ));
        if (len > width)
            width = len;
    }

    return width;
}

/* The cell itself, right-aligned like the authored Russian one. Russian returns
 * that string verbatim rather than rebuilding it, so its output cannot drift. */
static DLString clanPaddedName( Clan *clan, lang_t lang )
{
    if (lang == LANG_RU)
        return clan->getPaddedName( );

    DLString name = clan->getShortFor( lang );
    size_t width = clanColumnWidth( lang );

    return DLString( string( width > name.size( ) ? width - name.size( ) : 0, ' ' ) ) + name;
}


struct clan_diplomacy_names {
  const char *eng_name;
  const char *abbr;
  const char *color;
  const char *long_name;
  const char *state_name;
  const char *state_ruscase;
};

struct clan_diplomacy_names clan_diplomacy_names_table[] =
{
  {"alliance",      "аль.", "{W", "альянс",     "В альянсе с",   "5" },
  {"peace",         "мир ", "{G", "мир",        "В мире с",      "5" },
  {"truce",         "пер.", "{Y", "перемирие",  "В перемирии с", "5" },
  {"distrust",      "нед.", "{B", "недоверие",  "Не доверяет",   "3" },
  {"aggression",    "агр.", "{r", "агрессия",   "В агрессии с",  "5" },
  {"war",           "вой.", "{R", "война",      "Враждует с",    "5" },
  {"subordination", "под.", "{Y", "подчинение", "Подчиняется",   "3" },
  {"oppression",    "угн.", "{Y", "угнетение",  "Угнетает",      "4" },
};

const int clan_diplomacy_max = 5;

// A treaty that took effect goes to the public channels; an offer stays private.
static void clan_diplomacy_announce( Clan *a, Clan *b, int dip )
{
    if (a->isHidden( ) || b->isHidden( ))
        return;

    send_discord_clan( fmt( 0, "{W%s and %s: %s.{x",
                            a->getShortFor( LANG_EN ).c_str( ), b->getShortFor( LANG_EN ).c_str( ),
                            clan_diplomacy_names_table[dip].eng_name ) );
    send_telegram( fmt( 0, "{W%s и %s: %s.{x",
                        a->getShortFor( LANG_RU ).c_str( ), b->getShortFor( LANG_RU ).c_str( ),
                        clan_diplomacy_names_table[dip].long_name ) );
}

/*
 * Reformed clans run the Fenia command .tmp.clan.command(ch, args). It returns
 * false to hand the call to the legacy code below; true, nothing, or an
 * exception all count as handled, so code that already moved money never
 * runs twice. No function there means legacy.
 */
static bool gprog_clan( PCharacter *pc, const DLString &args )
{
    static Scripting::IdRef ID_TMP( "tmp" ), ID_CLAN( "clan" ), ID_COMMAND( "command" );
    Scripting::Register tmpClan, commandFn;

    if (!FeniaManager::wrapperManager)
        return false;

    try {
        Scripting::Register tmp = *Scripting::Context::root[ID_TMP];
        tmpClan = *tmp[ID_CLAN];
        commandFn = *tmpClan[ID_COMMAND];
    }
    catch (const ::Exception &) {
        return false;
    }

    // A closure whose code source is gone would throw before running anything.
    if (commandFn.type != Scripting::Register::FUNCTION || commandFn.toFunction( )->isBroken( ))
        return false;

    try {
        Scripting::RegisterList fnArgs;
        fnArgs.push_back( FeniaManager::wrapperManager->getWrapper( pc ) );
        fnArgs.push_back( Scripting::Register( args ) );

        Scripting::Register result = commandFn.toFunction( )->invoke( tmpClan, fnArgs );
        return !(result.type == Scripting::Register::NUMBER && result.toNumber( ) == 0);
    }
    catch (const ::Exception &ex) {
        FeniaManager::getThis( )->croak( 0, Scripting::Register( DLString( ".tmp.clan.command" ) ), ex );
        pc->pecho(_("Попробуй позже."));
        return true;
    }
}

COMMAND(CClan, "clan")
{
    PCharacter *pc = ch->getPC( );

    if (!pc)
        return;
    
    if (IS_CHARMED(pc)) {
        if (pc->master)
            pc->master->pecho(_("Нельзя проникнуть в тайны чужого клана с помощью колдовства."));

        pc->pecho(_("Тебя пытаются принудить выдать тайны своего клана, но ты не поддаешься."));
        return;
    }

    if (gprog_clan( pc, constArguments ))
        return;

    if (constArguments.length( ) == 0) {
        clanList( pc );
    }
    else {
        DLString argument = constArguments;
        DLString argumentOne = argument.getOneArgument( );
        
        if (arg_is_list( argumentOne ) || arg_is(argumentOne, "info"))
            clanList( pc );
        else if (arg_is(argumentOne, "count")) 
            clanCount( pc );
        else if( arg_is(argumentOne, "remove") )
            clanRemove( pc, argument );
        else if( arg_is(argumentOne, "level") )
            clanLevel( pc, argument );
        else if( arg_is(argumentOne, "member") )
            clanMember( pc, argument );
        else if( arg_is(argumentOne, "diplomacy") )
            clanDiplomacy( pc, argument );
        else if (pc->is_immortal( ) && arg_is(argumentOne, "scan"))
            clanScan( pc );
        else
            usage( pc );
    }        
}

void CClan::usage( PCharacter *pc )
{    
    basic_ostringstream<char> buf;

    buf << "{Wклан список{x     показать список всех кланов" << endl
        << "{Wклан счет {x      показать количество игроков в кланах" << endl
        << "{Wклан выгнать себя{x выйти из клана" << endl
        << "{Wклан уровень{x    посмотреть клановый ранг или список рангов" << endl
        << "{Wклан состав{x     показывает список членов клана (см. {Wклан состав помощь{x)" << endl
        << "{Wклан дипломатия{x посмотреть/установить клановую дипломатию (см. {Wклан дипломатия помощь{x)" << endl;

    pc->send_to( buf );
}

/*
 * clan
 * clan list
 */
void CClan::clanList( PCharacter* pc )
{
    pc->pecho(_("В мире есть такие кланы:"));                              
        
    for (int i = 0; i < ClanManager::getThis( )->size( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i );
        
        if (!clan->isHidden( )) {
            basic_ostringstream<char> buf;                                          
            buf << setw( 40 ) << clan->getLongNameFor( viewerLang(pc) ) << " [{"
                << clan->getColor( ) << clanPaddedName( clan, viewerLang(pc) ) 
                << "{x]" << endl;
            pc->send_to( buf );
        }
    }

    pc->pecho(_("\n\rПодробнее смотри команду клан ?{x."));                              
}

/*
 * clan count
 */ 
void CClan::clanCount( PCharacter* pc )
{
    vector<int> counts;
    PCharacterMemoryList::const_iterator pos;
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    ClanManager *cm = ClanManager::getThis( );
    
    counts.resize( cm->size( ) );
    
    for (int i = 0; i < cm->size( ); i++)
        counts[i] = 0;

    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;
        
        if (pcm->getLevel( ) < 102 && !pcm->getClan( )->isHidden( ))
            counts[pcm->getClan( )]++;
    }
    
    pc->pecho(_("      Клан         кол."));                               

    for (int i = 0; i < cm->size( ); i++) {
        Clan *clan = cm->find( i );
        
        if (!clan->isHidden( )) {
            basic_ostringstream<char> buf;
            buf << "  [{" << clan->getColor( )
                << clanPaddedName( clan, viewerLang(pc) ) << "{x] "
                << setw( 5 ) << counts[i] << endl;
            pc->send_to( buf );
        }
    }
}

/*
 * clan remove self
 * Leaving is the only legacy path left: expelling others is Fenia's job
 * (`clan kick` for officers, `clan force remove` for immortals).
 */ 
void CClan::clanRemove( PCharacter* pc, DLString& argument )
{
    basic_ostringstream<char> buf;
    XMLAttributeInduct::Pointer attr; 
    DLString argumentOne = argument.getOneArgument( );
    
    if (!arg_is_self( argumentOne )) {
        usage( pc );
        return;
    }

    Clan &clan = *pc->getClan( );
    ClanMembership *member = clan.getMembership( );

    if (!member) {
        pc->pecho(_("А откуда еще тебе хотелось бы уйти?"));
        return;
    }
    
    if (!pc->is_immortal( )) 
        if (!member->removable) {
            pc->pecho( _("Из твоего клана невозможно уйти по собственной воле.") );
            return;
        }

    buf << "Ты решаешь покинуть [" 
        << clan.getRussianName( ).ruscase('4') << "].";
            
    clan_freeze( pc );
    pc->setClan( member->removeSelf );

    pc->pecho("Ok.");

    pc->setClanLevel( 0 );
    ClanOrgs::delAttr( pc );

    attr = pc->getAttributes( ).getAttr<XMLAttributeInduct>( "induct" );
    attr->addEntry( buf.str( ) );
    attr->run( pc );
}

/* 
 * clan level [list|<victim>]
 * Read-only: ranks are set by Fenia (`clan setrank`) or grow on their own.
 */ 
void CClan::clanLevel( PCharacter *pc, DLString& argument )
{        
    PCMemoryInterface *victim;

    DLString argumentOne = argument.getOneArgument( );

    if (arg_is_list( argumentOne )) {
        clanLevelList( pc );
        return;
    }
    else if (argumentOne.empty( ) || arg_is_self( argumentOne )) {
        victim = pc;
    }
    else {
        victim = PCharacterManager::find( argumentOne );

        if (!victim) {
            pc->pecho(_("Игрок с таким именем не найден."));
            return;
        }                
    }
    
    clanLevelShow( pc, victim );
}

/*
 * clan level list
 */
void CClan::clanLevelList( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    const ClanTitles *titles;

    titles = pc->getClan( )->getTitles( );

    if (!titles) {
        pc->pecho(_("Клановых званий в твоем клане не обнаружено."));
        return;
    }
    
    titles->toStream( buf );
    pc->send_to( buf );
}

/*
 * clan level [<victim>|self]
 */
void CClan::clanLevelShow( PCharacter *pc, PCMemoryInterface *victim )
{
    Clan *clan = &*victim->getClan( );

    if (!clan->getTitles( )) {
        if (victim == pc)
            pc->pecho(_("Клановых рангов в твоем клане не обнаружено."));
        else
            pc->pecho(_("В его/ее клане нет клановых рангов."));
    }
    else {
        if (victim == pc)
            pc->pecho( _("Твой ранг [{%s%s{x]."),
                        clan->getColor( ).c_str( ),
                        clan->getTitle( pc, viewerLang(pc) ).c_str( ) );
        else
            pc->pecho( _("%s имеет ранг [{%s%s{x]."),
                        victim->getName( ).c_str( ),
                        clan->getColor( ).c_str( ),
                        clan->getTitle( victim, viewerLang(pc) ).c_str( ) );
    }
    
}

/*
 * clan member [date|name|level|clanlevel]
 */
static bool __member_cmp_date__( PCMemoryInterface *a, PCMemoryInterface *b )
{
    return a->getLastAccessTime( ).getTime( ) > b->getLastAccessTime( ).getTime( );
}    
static bool __member_cmp_level__( PCMemoryInterface *a, PCMemoryInterface *b )
{
    return a->getLevel( ) > b->getLevel( );
}    
static bool __member_cmp_clanlevel__( PCMemoryInterface *a, PCMemoryInterface *b )
{
    return a->getClanLevel( ) > b->getClanLevel( );
}    

void CClan::clanMember( PCharacter *pc, DLString& argument )
{        
    basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator pos;
    typedef std::list<PCMemoryInterface *> MemberList;
    MemberList members;

    DLString argumentOne = argument.getOneArgument( );

    if (arg_is_help( argumentOne )) {
        clanMemberHelp( pc );
        return;
    }

    if (pc->getClan()->isDispersed()) {
        if (pc->getClan() == clan_none)
            pc->pecho(_("Сначала присоединись к одному из кланов."));
        else
            pc->pecho(_("Ты не можешь увидеть список своих соклановиков."));
        return;
    }
    
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );

    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;

        if (pcm->getClan( ) != pc->getClan( ) || pcm->getLevel( ) >= 102)
            continue;
        
        members.push_back( pcm );
    }
    
    if (!argumentOne.empty( )) {
        
        if (arg_is(argumentOne, "date"))
            members.sort( __member_cmp_date__ );
        else if (arg_is(argumentOne, "name")) 
            ;
        else if (arg_is(argumentOne, "level"))
            members.sort( __member_cmp_level__ );
        else if (arg_is(argumentOne, "clanlevel"))
            members.sort( __member_cmp_clanlevel__ );
        else {
            clanMemberHelp( pc );
            return;
        }            
    }   
    
    for (MemberList::iterator i = members.begin( ); i != members.end( ); i++) {
        PCMemoryInterface *pcm = *i;
        buf << fmt(0, "%-10s %-10s %-12s %2d %3d  %-15s %s\r\n",
                   pcm->getName().c_str(),
                   pcm->getRace()->getName().c_str(),
                   pcm->getProfession( )->getNameFor(pc).c_str(),
                   pcm->getRemorts().size(), 
                   pcm->getLevel(),
                   pcm->getClan()->getTitle(pcm, viewerLang(pc)).c_str(),
                   pcm->getLastAccessTime( ).getTimeAsString("%d/%m/%y %H:%M").c_str());
    }

    pc->pecho(_("\n\r{BИмя         раса        класс         уровень звание           last time{x"));
    pc->send_to( buf );
}

/*
 * clan member help
 */
void CClan::clanMemberHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf   << "{Wклан состав{x           - показывает список всех членов клана, в алфавитном порядке" << endl
          << "{Wклан состав дата{x      - сортирует список по дате последнего захода в мир" << endl
          << "{Wклан состав уровень{x     - сортирует список по рангу" << endl
          << "{Wклан состав клануровень{x - сортирует список по клановому рангу" << endl;

    pc->send_to( buf );
}

/*
 * clan diplomacy [prop|set <clan> <dipl#>|list]
 */ 
void CClan::clanDiplomacy( PCharacter *pc, DLString& argument )
{        
    DLString argumentOne = argument.getOneArgument( );

    if (argumentOne.empty( )) 
        clanDiplomacyShow( pc );        
    else if (arg_is(argumentOne, "proposition"))
        clanDiplomacyProp( pc );
    else if (arg_is(argumentOne, "set"))
        clanDiplomacySet( pc, argument );
    else if (arg_is_list( argumentOne ))
        clanDiplomacyList( pc );
    else
        clanDiplomacyHelp( pc );
}

/* 
 * clan diplomacy
 */ 
void CClan::clanDiplomacyShow( PCharacter *pc )
{    
    ostringstream buf;
    Clan *clan;
    ClanData *data;
    ClanManager *cm = ClanManager::getThis( );

    if (uses_screenreader(pc)) {
        clanDiplomacyForBlindShow(pc);
        return;
    }

    pc->pecho(_("Клановая дипломатия :"));
    // Same cell as the row labels below (one short, as it always has been).
    size_t gridWidth = clanColumnWidth( viewerLang(pc) );
    buf << string( gridWidth > 0 ? gridWidth - 1 : 0, '*' ) << ' ';
    
    for (int i = 0; i < cm->size( ); i++) {
        clan = cm->find( i );

        if (clan->getData( ) && clan->hasDiplomacy( )) {
            DLString abbr = clan->getShortName( );
            
            abbr.toLower( );
            abbr.upperFirstCharacter( );
                    
            buf << ' ' << setw( 5 ) << abbr.substr( 0, 3 );
        }
    }
    
    buf << endl;

    for (int i = 0; i < cm->size( ); i++) {
        clan = cm->find( i );
        data = clan->getData( );
        
        if (data && clan->hasDiplomacy( )) {
            buf << clanPaddedName( clan, viewerLang(pc) ) << ' ';
            
            for (int j = 0; j < cm->size( ); j++) {
                Clan *c = cm->find( j );

                if (c->getData( ) && c->hasDiplomacy( ))
                    buf << ' ' 
                        << clan_diplomacy_names_table[data->getDiplomacy( c )].color
                        << setw( 5 ) 
                        << l(pc, clan_diplomacy_names_table[data->getDiplomacy( c )].abbr)
                        << "{x";
            }
            
            buf << endl;
        }
    }

    buf << endl;

    for (int i = 0; i <= clan_diplomacy_max; i++) 
        buf << clan_diplomacy_names_table[i].color 
            << l(pc, clan_diplomacy_names_table[i].abbr)
            << "{x - " 
            << l(pc, clan_diplomacy_names_table[i].long_name)
            << (i < clan_diplomacy_max ? ", " : " ");

    buf << endl;
    pc->send_to( buf );
}            

/*
 * clan diplomacy for the blind
 */
void CClan::clanDiplomacyForBlindShow( PCharacter *pc )
{
    ostringstream buf;
    Clan *clan;
    ClanData *data;
    ClanManager *cm = ClanManager::getThis( );

    pc->pecho(_("Клановая дипломатия :"));
    buf << "********** " << endl;

    for (int i = 0; i < cm->size( ); i++) {
        clan = cm->find( i );
        data = clan->getData( );

        if (clan->getData( ) && clan->hasDiplomacy( )) {
            buf << clan->getNameFor( viewerLang(pc) ).ruscase('1').c_str() << " : ";
        } else {
            continue;
        }

        for (int j = clan_diplomacy_max; j >= 0; j--) {
            int itemCount = 0;

            for (int k = 0; k < cm->size( ); k++) {
                if (i == k)
                    continue;

                Clan *c = cm->find( k );

                if (c->getData( ) && c->hasDiplomacy( ) && j == data->getDiplomacy( c )) {
                    itemCount++;
                }

            }
            if (itemCount > 0) {
                 buf << clan_diplomacy_names_table[j].color
                     << l(pc, clan_diplomacy_names_table[j].state_name) << "{x ";
                char rusCase = *clan_diplomacy_names_table[j].state_ruscase;
                for (int k = 0; k < cm->size( ); k++) {
                    if (i == k)
                        continue;

                    Clan *c = cm->find( k );

                    if (c->getData( ) && c->hasDiplomacy( ) && j == data->getDiplomacy( c )) {
                        itemCount--;
                        buf << c->getNameFor( viewerLang(pc) ).ruscase( rusCase ).c_str();
                        if (itemCount > 1) {
                            buf << ", ";
                        } else if (itemCount == 1) {
                            buf << " и ";
                        } else {
                            buf << ". ";
                        }
                    }
                }
            }

        }
        buf << endl;

    }
    pc->send_to( buf );
}

/* 
 * clan diplomacy prop
 */ 
void CClan::clanDiplomacyProp( PCharacter *pc )
{    
    ostringstream buf;
    Clan *myclan = &*pc->getClan( );
    ClanData *mydata = myclan->getData( );

    if (!mydata || !myclan->hasDiplomacy( )) {
        pc->pecho(_("Для твоего клана не существует понятия дипломатии."));
        return;
    }

    buf << "Просмотр пропозиций для " << myclan->getRussianName( ).ruscase('4') << ":" << endl;
    
    for (int i = 0; i < ClanManager::getThis( )->size( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i );
        ClanData *data = clan->getData( );

        if (clan == myclan || !clan->isValid( ) || !data || !clan->hasDiplomacy( ))
            continue;
            
        // A real offer is always better (lower) than the status quo.
        if (mydata->getProposition( clan ) >= mydata->getDiplomacy( clan ))
            continue;
        
        buf << '[' << clan->getShortName( ) << "] " 
            << clan_diplomacy_names_table[mydata->getDiplomacy( clan )].long_name
            << " - " 
            << clan_diplomacy_names_table[mydata->getProposition( clan )].long_name
            << endl;
    }

    pc->send_to( buf );    
}

/*
 * clan diplomacy set <clan> <dipl#>
 */ 
void CClan::clanDiplomacySet( PCharacter *pc, DLString& argument )
{    
    ostringstream buf;
    DLString argumentOne = argument.getOneArgument( );
    Clan *clan, *myclan;
    ClanData *data, *mydata;
    int dip;
    
    myclan = &*pc->getClan( );
    mydata = myclan->getData( );

    if (!mydata || !myclan->hasDiplomacy( )) {
        pc->pecho(_("Для твоего клана не существует понятия дипломатии."));
        return;
    }
    
    if (!myclan->isRecruiter( pc ) && !pc->is_immortal( )) {
        pc->pecho(_("Только руководство кланов может менять политику."));
        return; 
    }        

    pc->pecho(_("Установка политики"));

    clan = ClanManager::getThis( )->findUnstrict( argumentOne );

    if (!clan) {
        pc->pecho(_("Такого клана не существует."));
        return;
    }
    
    data = clan->getData( );

    if (!data || !clan->hasDiplomacy( )) {
        pc->pecho(_("Для этого клана не существует понятия дипломатии."));
        return;
    }
    
    if (myclan == clan) {
        pc->pecho(_("Твой клан развалится и без твоей помощи"));
        mydata->setDiplomacy( myclan, 0 );
        mydata->save( );
        return;
    }

    argument = argument.getOneArgument( );
    
    try {
        dip = argument.toInt( );
    } catch (const ExceptionBadType &e) {
        pc->pecho(_("Неверная политика."));
        return;
    }
    
    if (dip < 0 || dip > clan_diplomacy_max) {
        pc->pecho(_("Неверная политика (см. clan diplomacy list)."));
        return;
    }
    
    if (mydata->getDiplomacy( clan ) == dip) {
        pc->pecho(_("Это ничего не меняет."));
        return;
    }
    
    if (clan->isDispersed( )) {
        mydata->setDiplomacy( clan, dip );
        mydata->save( );
        data->setDiplomacy( myclan, dip );
        data->save( );
        
        buf << "Установка политики для "
            << clan->getShortName( ) << " : "
            << clan_diplomacy_names_table[dip].long_name
            << endl;
        pc->send_to( buf );
        return;
    }
    
    if (mydata->getDiplomacy( clan ) > dip) {
        pc->pecho(_("улучшение"));
        
        if (mydata->getProposition( clan) <= dip) {
            // Не лучше предложеного
            mydata->setDiplomacy( clan, dip );
            mydata->setProposition( clan, dip );
            mydata->save( );

            data->setDiplomacy( myclan, dip );
            data->setProposition( myclan, dip );
            data->save( );
            clan_diplomacy_announce( myclan, clan, dip );

            buf << "Установка политики для "
                << clan->getRussianName( ).ruscase('2') << " : "
                << clan_diplomacy_names_table[dip].long_name
                << endl;
            pc->send_to( buf );
        }
        else
        {
            buf << "Ты предлагаешь "
                << clan->getRussianName( ).ruscase('3')
                << " отношение типа "
                << clan_diplomacy_names_table[dip].long_name
                << endl;
            pc->send_to( buf );

            data->setProposition( myclan, dip );
            data->save( );
        }
    }
    else
    {
        pc->pecho(_("УХУДШЕНИЕ"));
        
        mydata->setDiplomacy( clan, dip );
        mydata->setProposition( clan, dip );
        mydata->save( );

        data->setDiplomacy( myclan, dip );
        data->setProposition( myclan, dip );
        data->save( );
        clan_diplomacy_announce( myclan, clan, dip );

        buf << "Установка политики для "
            << clan->getRussianName( ).ruscase('2')
            << " : " << clan_diplomacy_names_table[dip].long_name
            << endl;
        pc->send_to( buf );
    }
}

/* 
 * clan diplomacy list 
 */ 
void CClan::clanDiplomacyList( PCharacter *pc )
{    
    ostringstream buf;

    buf << "Доступные дипломатии:" << endl;

    for (int i = 0; i <= clan_diplomacy_max; i++)
        buf << i << " - " << clan_diplomacy_names_table[i].color
            << clan_diplomacy_names_table[i].long_name
            << "{x (" << clan_diplomacy_names_table[i].eng_name
            << ')' << endl;
    
    buf << endl;
    pc->send_to( buf );
}

/*
 * clan diplomacy help
 */
void CClan::clanDiplomacyHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
   
    buf << "{Wклан дипломатия{x             - показать клановую дипломатию" << endl
        << "{Wклан дипломатия предложения{x - показать предложения по изменению дипломатии" << endl
        << "{Wклан дипломатия список{x      - список всех возможных дипломатий" << endl
        << endl
        << "Для лидеров:" << endl
        << "{Wклан дипломатия установить {x<клан> <номер>" << endl
        << "   - изменить политику по отношению к какому-либо клану" << endl;

    pc->send_to( buf );
}

/*
 * clan scan
 */
void CClan::clanScan( PCharacter *pc )
{
    ostringstream buf;
    ClanManager *cm = ClanManager::getThis( );
    
    for (int j = 0; j < cm->size( ); j++) {
        Clan::Pointer c = cm->find( j );
        
        buf << "[" << j << "] "
            << c->getName( ) << ", " << c->getShortName( ) << " " 
            << (c->isValid( ) ? "valid" : "non-valid") << " "
            << (c->isHidden( ) ? "hidden" : "non-hidden") << " ";

        if (c->getData( )) {
            if (c->getData( )->getBank( ))
                buf << "bank ";
            if (c->getData( )->hasItem( ))
                buf << "item ";
        }

        buf << endl;
    }

    pc->send_to( buf );
}
