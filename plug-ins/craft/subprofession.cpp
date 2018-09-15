#include "subprofession.h"

#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "alignment.h"
#include "room.h"
#include "race.h"
#include "merc.h"
#include "dl_strings.h"
#include "def.h"


/*-------------------------------------------------------------------
 * CraftProfessionHelp 
 *------------------------------------------------------------------*/
const DLString CraftProfessionHelp::TYPE = "CraftProfessionHelp";

void CraftProfessionHelp::setProfession( CraftProfession::Pointer prof )
{
    StringSet kwd;

    this->prof = prof;
    
    if (!keyword.empty( ))
	kwd.fromString( keyword );

    kwd.insert( prof->getName( ) );
    kwd.insert( prof->getRusName( ).ruscase( '1' ) );
    kwd.insert( prof->getMltName( ).ruscase( '1' ) );
    fullKeyword = kwd.toString( ).toUpper( );

    helpManager->registrate( Pointer( this ) );
}

void CraftProfessionHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    fullKeyword = "";
}

void CraftProfessionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Дополнительная профессия {C" << prof->getRusName( ).ruscase( '1' ) << "{x или {C"
       << prof->getName( ) << "{x" << endl << endl;
        
    in << *this << endl;
}

/*-------------------------------------------------------------------
 * CraftProfession
 *------------------------------------------------------------------*/
CraftProfession::CraftProfession( )
{
}

CraftProfession::~CraftProfession( )
{
}

void CraftProfession::loaded( )
{
    craftProfessionManager->load( Pointer( this ) );

    if (help)
	help->setProfession( Pointer( this ) );
}

void CraftProfession::unloaded( )
{
    if (help)
	help->unsetProfession( );

    craftProfessionManager->load( Pointer( this ) );
}

DLString CraftProfession::getNameFor( Character *ch, const Grammar::Case &c ) const
{
    if (ch && ch->getConfig( )->rucommands)
	return getRusName( ).ruscase( c );
    else
	return getName( );
}

/*-------------------------------------------------------------------
 * CraftProfessionManager
 *------------------------------------------------------------------*/
CraftProfessionManager * craftProfessionManager = 0;

CraftProfessionManager::CraftProfessionManager( )
{
    checkDuplicate( craftProfessionManager );
    craftProfessionManager = this;
}

CraftProfessionManager::~CraftProfessionManager( )
{
    craftProfessionManager = 0;
}

void CraftProfessionManager::load( CraftProfession::Pointer prof )
{
    profs[prof->getName()] = prof;
}

void CraftProfessionManager::unload( CraftProfession::Pointer prof )
{
    Professions::iterator p = profs.find(prof->getName());
    profs.erase(p);
}

CraftProfession::Pointer CraftProfessionManager::get( const DLString &name ) const
{
    Professions::const_iterator p = profs.find(name);
    if (p == profs.end())
        return CraftProfession::Pointer();
    else
        return p->second;
}

CraftProfession::Pointer CraftProfessionManager::lookup( const DLString &arg ) const
{
    Professions::const_iterator p;
    for (p = profs.begin(); p != profs.end(); p++)
        if (is_name(arg.c_str(), p->first.c_str()))
            return p->second;
    return CraftProfession::Pointer();
}

void CraftProfessionManager::initialization( ) 
{
}

void CraftProfessionManager::destruction( ) 
{
}

