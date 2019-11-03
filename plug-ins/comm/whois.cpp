/* $Id: whois.cpp,v 1.1.2.25.6.6 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2003
 */

#include "whois.h"
#include "playerattributes.h"

#include "class.h"
#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcrace.h"

#include "commonattributes.h"
#include "clanreference.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "descriptor.h"
#include "loadsave.h"
#include "skillreference.h"
#include "fight.h"
#include "def.h"

GSN(manacles);
GSN(jail);
PROF(none);

COMMAND(Whois, "whois")
{
    std::basic_ostringstream<char> buf;
    Descriptor *d;
    LinesList lines;
    PCharacter *pch, *offline_pch = NULL;
    DLString args = constArguments;
   
    if (ch->getPC( ) == 0)
        return;

    if (args.empty( )) {
        ch->send_to( "Имя, сестра, имя!\r\n" );
        return;
    }
        
    pch = get_player_world(ch->getPC(), args.c_str(), false);

    if (!pch) {
        offline_pch = dallocate(PCharacter);
        args.capitalize();
        offline_pch->setName(args);
        if (PCharacterManager::load(offline_pch))
            pch = offline_pch;
        else
            ddeallocate(offline_pch);
    }

    if (!pch) {
        ch->send_to( "Никого нет с таким именем.\r\n" );
        return;
    }
    
    /* Pretitle Name Title */
    player_fmt( "{W%N{w%p{w", pch, buf, ch );
    lines.add( buf );

    /* Race, Sex, Class, Level */
    buf << "{W";
    
    if (pch->getSex( ) == SEX_FEMALE)
        buf << russian_case( pch->getRace( )->getPC( )->getFemaleName( ), '1' );
    else
        buf << russian_case( pch->getRace( )->getPC( )->getMaleName( ), '1' );
        
    buf << "{w";
    
    if (ch->getPC( )->canSeeProfession( pch )) {
        buf << ", профессия {W" << pch->getProfession( )->getNameFor( ch ) << "{w";
        
        if (pch->getSubProfession( ) != prof_none)
            buf << " ({W" << pch->getSubProfession( )->getWhoNameFor( ch ) << "{w)";
    }
    
    if (ch->getPC( )->canSeeLevel( pch ))
        buf << ", уровень {W" << pch->getRealLevel( ) << "{w";

    lines.add( buf );

    /* Ethos-Align */
    if (ch->getClan( ) == pch->getClan( ) && !ch->getClan( )->isDispersed( ))
    {
        buf << "характер {W"
            << ethos_table.name( pch->ethos ) << "-"
            << align_table.name( ALIGNMENT(pch) ) << "{w";
        lines.add( buf );
    }

    /* Remorts. PK */
    if (pch->getRemorts( ).size( )) 
        buf << "количество перерождений: {W" << pch->getRemorts( ).size( ) << "{w    ";

    if (pch->getRealLevel( ) >= PK_MIN_LEVEL && !is_safe_nomessage( ch, pch )) 
        buf << "находится в твоем {R(PK){w";

    lines.add( buf );

    /* Clan name, clan level, (R) (L) */
    if (!pch->getClan( )->isHidden( )) {
        const Clan &clan = *pch->getClan( );
        
        buf << "клан {" << clan.getColor( ) << clan.getShortName( ) << "{w, "
            << "клановое звание [{" << clan.getColor( ) << clan.getTitle( pch ) << "{w]";
        
        if (clan.isLeader( pch ))
            buf << ", лидер";
        else if (clan.isRecruiter( pch ))
            buf << ", рекрутер";

        lines.add( buf );
    }
    
    /* gather info from attributes (selfrate, marriage etc) */
    list<DLString> attrLines;
    
    if (pch->getAttributes( ).handleEvent( WhoisArguments( pch, ch->getPC(), attrLines ) ))
        for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
            buf << *l;
            lines.add( buf );
        }

    /* Flags */
    std::vector<const char *> flags;
    
    if (IS_SET( pch->comm, COMM_AFK ))  flags.push_back( " {CA{w(afk)" );
    if (can_see_god(ch, pch)) {
        if (pch->incog_level >= LEVEL_HERO) 
            flags.push_back( " {DI{w(инкогнито)" );
        if (pch->invis_level >= LEVEL_HERO) 
            flags.push_back( " {DW{w(wizinv)" );
    }
    if (IS_KILLER( pch ))               flags.push_back( " {RK{w(убийца)" );
    if (IS_THIEF( pch ))                flags.push_back( " {RT{w(вор)" );
    if (IS_SLAIN( pch ))                flags.push_back( " {DS{w(убит)" );
    if (IS_GHOST( pch ))                flags.push_back( " {DG{w(призрак)" );
    if (IS_DEATH_TIME( pch ))           flags.push_back( " {DP{w(защита богов)" );
    if (IS_VIOLENT( pch ))              flags.push_back( " {BV{w(адреналин в крови)" );
    if (pch->curse != 100)              flags.push_back( " {DC{w(проклят богами)" );
    if (pch->bless)                     flags.push_back( " {CB{w(благословлен богами)" );
    if (IS_SET( pch->act, PLR_WANTED))  flags.push_back( " {RW{w(в розыске)" );
    if (pch->isAffected(gsn_manacles)) flags.push_back( " {mM{w(в кандалах)" );
    if (pch->isAffected(gsn_jail ))   flags.push_back( " {mJ{w(в тюрьме)" );

    if (pch->getAttributes( ).isAvailable( "nochannel" ))
        flags.push_back( " {mN{w(nochannel)" );

    if (pch->getAttributes( ).isAvailable( "nopost" ))
        flags.push_back( " {mP{w(nopost)" );

    if (pch->getAttributes( ).isAvailable( "teacher" ))
        flags.push_back( " {gT{w(может обучать других)" );
    
    if (!flags.empty( ))
        buf << "флаги:";

    for (std::vector<const char *>::iterator i = flags.begin( ); i != flags.end( ); i++) {
        if (DLString(buf.str()).colorLength() + DLString(*i).colorLength() > 70) {
            lines.add( buf );
            buf << "      ";
        }
        
        buf << *i;
    }

    lines.add( buf );

    XMLStringAttribute::Pointer bio = pch->getPC( )->getAttributes( ).findAttr<XMLStringAttribute>( "bio" );
    if (bio && !bio->getValue( ).empty( )) {
            char buf[1024];
            istringstream is(bio->getValue( )); 
         
            while( is.getline(buf, sizeof(buf)) )  {
                ostringstream bufStr;
                bufStr << buf;
                lines.add( bufStr, true );
            }
    }

    /* Output */

    ch->send_to( "/------------------------------------------------------------------------\\\r\n" );
    
    for (LinesList::iterator j = lines.begin( ); j != lines.end( ); j++)
        ch->send_to( *j );
    
    ch->send_to( "\\________________________________________________________________________/\r\n" );

    if (offline_pch)
        ddeallocate(offline_pch);
}

void Whois::LinesList::addNoCR( std::basic_ostringstream<char> &buf ) 
{
    add( buf, false );
}

void Whois::LinesList::add( std::basic_ostringstream<char> &buf, bool fCR ) {
    std::basic_ostringstream<char> str;

    if (buf.str( ).empty( ))
        return;
        
    int length = DLString( buf.str( ) ).colorLength( );

    str << "{w| " << buf.str( );

    for (int i = 0; i < 70 - length; i++)
        str << " ";
    
    str << " {d|";
    if (fCR)
        str << endl;

    push_back( str.str( ) );
    buf.str( "" );
}

