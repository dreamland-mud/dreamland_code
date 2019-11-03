/* $Id: who.cpp,v 1.1.2.26.6.10 2009/08/16 20:28:04 rufina Exp $
 *
 * ruffina, 2003
 */

#include <sstream>

#include "who.h"
#include "class.h"
#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "object.h"
#include "pcrace.h"

#include "webmanip.h"
#include "clanreference.h"
#include "fight.h"
#include "act.h"
#include "skillreference.h"
#include "mercdb.h"
#include "handler.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"

GSN(manacles);
GSN(jail);

COMMAND(Who, "who")
{
    std::basic_ostringstream<char> buf;
    std::list<PCharacter *> victims;
    std::map<DLString, bool> rsClan;
    std::map<DLString, bool> rsRace;
    std::map<DLString, bool> rsClass;
    Clan *clan;
    Profession *prof;
    bool fPK = false, fImmortal = false; 
    int count, tattoo = 0, nNumber = 0, minLevel = 0, maxLevel = MAX_LEVEL;
    DLString arg, arguments = constArguments;
    
    if (ch->getPC( ) == 0)
        return;

    for( ; ; ) {
        arg = arguments.getOneArgument( );

        if (arg.empty( ))
            break;

        if (arg_is_pk( arg )) {
            fPK = true;
            continue;
        }

        if (arg == "tattoo" || arg == "татуировка") {
            if (!get_eq_char( ch, wear_tattoo )) {
                ch->send_to( "У тебя нет татуировки!\n\r");
                return;
            } else {
                tattoo = get_eq_char( ch, wear_tattoo )->pIndexData->vnum;
                continue;
            }
        }

        if (is_number( arg.c_str( ) )) {
            if (!ch->is_immortal( )) {
                ch->send_to( "Синтаксис 'who <minlevel> <maxlevel>' доступен только богам.\r\n" );
                return;
            }
            
            switch (++nNumber) {
            case 1:        minLevel = atoi( arg.c_str( ) ); break;
            case 2:        maxLevel = atoi( arg.c_str( ) ); break;
            default:        ch->send_to( "Слишком много аргументов.\n\r"); return;
            }

            continue;  
        }
    
        if (( clan = ClanManager::getThis( )->findUnstrict( arg ) )) {
            rsClan[clan->getName( )] = true;
            continue;
        } 

        if (arg.at( 0 ) == 'i') {
            fImmortal = true;
            continue;
        }

        prof = professionManager->findUnstrict( arg );

        if (!prof) {
            Race * race = raceManager->findUnstrict( arg );
            
            if (!race) {
                ch->send_to( "Неправильная раса.\n\r");
                return;
            }
            
            rsRace[race->getName( )] = true;

        } else { 
            if (!ch->is_immortal( )) {
                ch->send_to( "Синтаксис 'who <имя класса>' доступен только богам.\r\n" );
                return;
            }
            
            rsClass[prof->getName( )] = true;
        }

    }


    count = 0;
    for (Descriptor *d = descriptor_list; d; d = d->next) {
        Object *obj;
        PCharacter *victim;
        
        if (d->connected != CON_PLAYING || !d->character)
            continue;
        
        victim = d->character->getPC( );

        if (victim->getAttributes( ).isAvailable("nowho"))
            continue;

        // Add here explicit checks for wizinvis and incognito,
        // to avoid showing hidden immortals in total count.      
        if (!can_see_god(ch, victim)) 
            continue;

        count++;
        
        if (victim->getRealLevel( ) < minLevel || victim->getRealLevel( ) > maxLevel)
            continue;
        
        if (fPK && is_safe_nomessage( ch, victim ))
            continue;

        if (fImmortal && victim->getRealLevel( ) <= LEVEL_HERO)
            continue;

        if (!rsClass.empty( ))
            if (rsClass.find( victim->getProfession( )->getName( ).c_str( ) ) == rsClass.end( ))
                continue;
        
        if (!rsRace.empty( ))
            if (rsRace.find( victim->getRace( )->getName( ) ) == rsRace.end( ))
                continue;

        if (!rsClan.empty( ))
            if (rsClan.find( victim->getClan( )->getName( ) ) == rsClan.end( ))
                continue;

        if (tattoo)
            if (!(obj = get_eq_char( victim, wear_tattoo)) || tattoo != obj->pIndexData->vnum)
                continue;

        victims.push_back( victim );
    }

    for (std::list<PCharacter *>::iterator i = victims.begin( ); i != victims.end( ); i++)
        ch->send_to( formatChar( ch, *i ).c_str( ) );

    buf << endl 
        << "Всего игроков: " << count;
    if (count != victims.size()) 
        buf << ", найдено: " << victims.size( );
    buf << ". "
        << "Максимум на сегодня был: " << Descriptor::getMaxOnline( ) << "." << endl;
    if (!IS_SET( ch->act, PLR_CONFIRMED ) && ch->getPC( )->getRemorts( ).size( ) == 0) 
        buf << "Буква (U) рядом с твоим именем означает, что твое описание еще не одобрено богами." << endl
            << "Подробнее читай {W? подтверждение{x." << endl;
    ch->send_to( buf );
}

DLString Who::formatChar( Character *ch, PCharacter *victim ) {
    DLString result;
    std::basic_ostringstream<char> buf, tmp;
  
    /* Level, Race, Class */
    buf << "{x|" << leftColumn( ch, victim ) << "{x|";
        
    /* PK */
    if (victim->getModifyLevel( ) >= PK_MIN_LEVEL && !is_safe_nomessage( ch, victim ))
        tmp << "{x({rPK{x)";
    
    buf << dlprintf( "%4s", tmp.str( ).c_str( ) );
    tmp.str( "" );

    /* Clan, (R) (L) */
    if (!victim->getClan( )->isHidden( ) && !victim->is_immortal( )) {
        const Clan &clan = *victim->getClan( );

        tmp << "{x[{" << clan.getColor( ) << clan.getShortName( ).at( 0 ) << "{x]";
        
        if (clan.isLeader( victim ))
            tmp << "{R({CL{R){x";
        else if (clan.isRecruiter( victim ))
            tmp << "{R({CR{R){x";
        else 
            tmp << "   ";
    } 
    
    buf << dlprintf( "%6s", tmp.str( ).c_str( ) );
    tmp.str( "" );
   
    /* Remorts */
    int remorts = victim->getRemorts( ).size( );
    if (remorts) {
        if (remorts < 10)
                tmp << " {W({M" + DLString(remorts) + "{W)";
        else
                tmp << "{W({M" + DLString(remorts) + "{W)";
    }
    
    buf << dlprintf( "%4s", tmp.str( ).c_str( ) );
    tmp.str( "" );

    /* Flags */
    buf << dlprintf( "%9s", flags( victim ).c_str( ) );
        
    /* Pretitle, Name, Title */
    if (victim->getRealLevel( ) > LEVEL_HERO 
            || (victim->isCoder( ) && ch->isCoder( ) && victim->getRealLevel( ) >= LEVEL_HERO))
        buf << "{C";
    else
        buf << "{W";
   
    DLString descr = ch->seeName( victim );
    webManipManager->decorateCharacter( buf, descr, victim, ch );

    buf << "{x " << victim->getParsedTitle( ) << "{x" << endl;
    return buf.str( );
}

DLString Who::leftColumn( Character *ch, PCharacter *victim ) {
    std::basic_ostringstream<char> buf, tmp;
    
    if (victim->isCoder( ) && ch->isCoder( ) && victim->getRealLevel( ) >= LEVEL_HERO) {
        /* visible only to each other, like vampires >8) */
        buf << "{C    CODER    {x";
        return buf.str( );
    }
    
    switch (victim->getRealLevel( )) {
    case MAX_LEVEL - 0: buf << GET_SEX( victim, "{W IMPLEMENTOR {x",
                                                "{W IMPLEMENTOR {x",
                                                "{WIMPLEMENTRESS{x" );
                        break;
    case MAX_LEVEL - 1: buf << "{C   CREATOR   {x"; break;
    case MAX_LEVEL - 2: buf << "{C  SUPREMACY  {x"; break;
    case MAX_LEVEL - 3: buf << "{C    DEITY    {x"; break;
    case MAX_LEVEL - 4: buf << "{C     GOD     {x"; break;
    case MAX_LEVEL - 5: buf << "{G   IMMORTAL  {x"; break;
    case MAX_LEVEL - 6: buf << "{G   DEMIGOD   {x"; break;
    case MAX_LEVEL - 7: buf << "{G    ANGEL    {x"; break;
    case MAX_LEVEL - 8: buf << "{G   AVATAR    {x"; break;
    case MAX_LEVEL - 9: buf << "{w  LEGENDARY  {x"; break;
    }

    if (!buf.str( ).empty( ))
        return buf.str( );

    /* Level */
    if (ch->getPC( )->canSeeLevel( victim )) {
        if (!ch->getPC( )->canSeeProfession( victim ))
            tmp << "{c";

        tmp << victim->getRealLevel( ) << "{x";
    }

    buf << dlprintf( "%3s", tmp.str( ).c_str( ) );
    tmp.str( "" );
    
    /* Race */
    tmp << victim->getRace( )->getPC( )->getWhoNameFor( ch );

    if (tmp.str( ).size( ) < 4)
        buf << "  " << dlprintf( "%-4s", tmp.str( ).c_str( ) );
    else 
        buf << " " << dlprintf( "%-5s", tmp.str( ).c_str( ) );

    tmp.str( "" );

    /* Class */
    if (ch->getPC( )->canSeeProfession( victim ))
        tmp << "{Y" << victim->getProfession( )->getWhoNameFor( ch );

    buf << " " << dlprintf( "%3s", tmp.str( ).c_str( ) );
    return buf.str( );
}

DLString Who::flags( PCharacter *victim ) {
    std::basic_ostringstream<char> buf, result;
    XMLAttributes *attrs = &victim->getAttributes( );
    
    if (IS_SET( victim->comm, COMM_AFK ))  buf << "{CA";
    if (victim->incog_level >= LEVEL_HERO) buf << "{DI";
    if (victim->invis_level >= LEVEL_HERO) buf << "{DW";
    if (IS_KILLER( victim ))               buf << "{RK";
    if (IS_THIEF( victim ))                buf << "{RT";
    if (IS_SLAIN( victim ))                buf << "{DS";
    if (IS_GHOST( victim ))                buf << "{DG";
    if (IS_DEATH_TIME( victim ))           buf << "{DP";
    if (IS_VIOLENT( victim ))              buf << "{BV";
    if (victim->curse != 100)              buf << "{DC";
    if (victim->bless)                     buf << "{CB";
    if (IS_SET( victim->act, PLR_WANTED))  buf << "{RW";
    if (victim->isAffected(gsn_manacles)) buf << "{mM";
    if (victim->isAffected(gsn_jail ))   buf << "{mJ";
    if (attrs->isAvailable("nochannel"))   buf << "{mN";
    if (attrs->isAvailable( "nopost" ))    buf << "{mP";
    if (attrs->isAvailable( "teacher" ))   buf << "{gT";
    if (!IS_SET( victim->act, PLR_CONFIRMED )) buf << "{gU";

    if (!buf.str( ).empty( )) 
        result << "{x(" << buf.str( ) << "{x)";

    return result.str( );
}


