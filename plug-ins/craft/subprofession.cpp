#include "subprofession.h"

#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "alignment.h"
#include "room.h"
#include "race.h"
#include "merc.h"
#include "def.h"


/*-------------------------------------------------------------------
 * SubProfessionHelp 
 *------------------------------------------------------------------*/
const DLString SubProfessionHelp::TYPE = "SubProfessionHelp";

void SubProfessionHelp::setProfession( SubProfession::Pointer prof )
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

void SubProfessionHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    fullKeyword = "";
}

void SubProfessionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Дополнительная профессия {C" << prof->getRusName( ).ruscase( '1' ) << "{x или {C"
       << prof->getName( ) << "{x" << endl << endl;
        
    in << *this << endl;
}

/*-------------------------------------------------------------------
 * SubProfession
 *------------------------------------------------------------------*/
SubProfession::SubProfession( )
{
}

SubProfession::~SubProfession( )
{
}

void SubProfession::loaded( )
{
    //subProfessionManager->registrate( Pointer( this ) );

    if (help)
	help->setProfession( Pointer( this ) );
}

void SubProfession::unloaded( )
{
    if (help)
	help->unsetProfession( );

    //subProfessionManager->unregistrate( Pointer( this ) );
}

