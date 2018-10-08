/* $Id: note.cpp,v 1.1.2.12.6.5 2009/11/08 17:41:56 rufina Exp $
 *
 * ruffina, 2005
 */
#include "note.h"

#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "clanreference.h"
#include "merc.h"
#include "def.h"

CLAN(none);

Note::Note( ) : godsSeeAlways( false )
{
}

Note::~Note( )
{
}

inline bool isSingleOrPlural( const DLString &a, const DLString &b )
{
    return (a ^ b || a ^ (b + "s"));
}
inline bool argIsAll( const DLString &arg )
{
    return (arg ^ "all" || arg ^ "все" || arg ^ "всем");
}
inline bool argIsImmortal( const DLString &arg )
{
    return (isSingleOrPlural( arg, "immortal" ) 
	    || isSingleOrPlural( arg, "imm" )
	    || arg ^ "боги"
	    || arg ^ "богам");
}
inline bool argIsCoder( const DLString &arg )
{
    return (isSingleOrPlural( arg, "coder" )
	    || arg ^ "кодеры"
	    || arg ^ "кодерам");
}
inline bool argIsPlayer( const DLString &arg, const PCMemoryInterface *pcm )
{
    if (arg.empty( ))
	return false;
    if (arg ^ pcm->getName( ))
	return true;
    if ( pcm->getRussianName( ).decline('7').isName( arg ))
	return true;
    return false;
}


void Note::toStream( int vnum, ostringstream &buf ) const
{
    buf << "[" << vnum << "] "
	<< "{C" << getFrom( ) << "{x: "
	<< getSubject( ) << "{x" << endl
	<< getDate( ).getTimeAsString( ) << endl
	<< "To: " << getRecipient( ) << "{x" << endl
	<< getText( ) << "{x" << endl;
}

void Note::toForwardStream( ostringstream &buf ) const
{
    buf << ">>> Start of original message <<<" << endl
        << ">>> " << getFrom( ) << "{x: " << getSubject( ) << "{x" << endl
	<< ">>> " << getDate( ).getTimeAsString( ) << endl
	<< ">>> To: " << getRecipient( ) << "{x" << endl
	<< endl
	<< getText( ) << "{x" << endl
	<< ">>> End of original message <<<" << endl;
}

bool Note::isNoteFrom( PCMemoryInterface *pcm ) const
{
    if (getAuthor( ) == pcm->getName( ))
	return true;

    return false;
}

bool Note::isNoteTo( PCMemoryInterface *pcm ) const
{
    ostringstream buf; 
    DLString arg, arguments = getRecipient( );
    Race *race;
    Clan *clan;
    Profession *prof;

    while (!( arg = arguments.getOneArgument( ) ).empty( )) {
	if (findRecipient( pcm, arg, buf ))
	    return true;

	if (pcm->getClan( ) != clan_none) 
	    if (( clan = findClan( arg ) ) && pcm->getClan( ) == clan)
		return true;
    	
	if (( race = findRace( arg ) ) && pcm->getRace( ) == race)
	    return true;
	   
	if (( prof = findProf( arg ) ) && pcm->getProfession( ) == prof)
	    return true;
    }
    
    return false;
}


bool Note::isNoteToAll( ) const
{
    DLString arg, arguments = getRecipient( );

    while (!( arg = arguments.getOneArgument( ) ).empty( )) {
        if (argIsAll( arg )) 
            return true;
    }
    
    return false;
}

bool Note::parseRecipient( PCharacter *ch, const DLString &cArguments, ostringstream &buf )
{
    Race *race;
    Clan *clan;
    Profession *prof;
    PCMemoryInterface *pci;
    DLString arg, arguments = cArguments;
    bool found = false;

    while (!( arg = arguments.getOneArgument( ) ).empty( )) {
	ostringstream ostr;

	findRecipient( ch, arg, ostr );

	if (( clan = findClan( arg ) )) 
	{
	    ostr << "члены клана {" << clan->getColor( ) << clan->getShortName( ) << "{x";
	}
	else if (( race = findRace( arg ) )) 
	{
	    ostr << "представители расы {W" << race->getName( ) << "{x";
	}
	else if (( prof = findProf( arg ) )) 
	{
	    ostr << "представители профессии {W" << prof->getName( ) << "{x";
	}
	else if (( pci = PCharacterManager::find( arg ) ))
	{
	    ostr << "{W" << pci->getName( ) << "{x";
	}
	
	if (!ostr.str( ).empty( )) {
	    found = true;
	    buf << ostr.str( ) << endl;
	}
    }

    return found;
}

bool Note::findRecipient( PCMemoryInterface *pcm, DLString &arg, ostringstream &buf )
{
    if (argIsAll( arg )) {
	buf << "все";
	return true;
    }
    
    if (argIsImmortal( arg )) {
	buf << "Боги";
	return (pcm->get_trust( ) >= LEVEL_IMMORTAL);
    }
    
    if (argIsPlayer( arg, pcm )) {
	return true;
    }
    
    if (argIsCoder( arg )) {
	buf << "кодеры";
	return (pcm->get_trust( ) >= MAX_LEVEL);
    }

    return false;
}

Profession * Note::findProf( const DLString &arg )
{
    Profession *profession;
    
    if (( profession = professionManager->findExisting( arg ) ))
	return profession;
	
    if (arg.at( arg.size( ) - 1 ) == 's')
	if (( profession = professionManager->findExisting( arg.substr( 0, arg.size( ) - 1 ) ) ))
	    return profession;
    
    return NULL;
}


Clan * Note::findClan( const DLString &arg )
{
    Clan *clan;
    
    if (( clan = clanManager->findExisting( arg ) ))
	return clan;
	
    if (arg.at( arg.size( ) - 1 ) == 's')
	if (( clan = clanManager->findExisting( arg.substr( 0, arg.size( ) - 1 ) ) ))
	    return clan;
    
    return NULL;
}

Race * Note::findRace( const DLString &arg )
{
    Race *race;
    
    if (( race = raceManager->findExisting( arg ) ))
	return race;
	
    if (arg.at( arg.size( ) - 1 ) == 's')
	if (( race = raceManager->findExisting( arg.substr( 0, arg.size( ) - 1 ) ) ))
	    return race;
    
    return NULL;
}

WebNote::WebNote( ) 
{
}

WebNote::~WebNote( )
{
}

