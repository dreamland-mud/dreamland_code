/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultreligion.h"
#include "logstream.h"

#include "character.h"
#include "race.h"

#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * ReligionHelp 
 *------------------------------------------------------------------*/
const DLString ReligionHelp::TYPE = "ReligionHelp";

void ReligionHelp::setReligion( Religion::Pointer religion )
{
    StringSet kwd;

    this->religion = religion;
    
    if (!keyword.empty( ))
        kwd.fromString( keyword );

    kwd.insert( religion->getName( ) );
    kwd.insert( religion->getRussianName( ).ruscase( '1' ) );
    fullKeyword = kwd.toString( ).toUpper( );
    LogStream::sendNotice() << "registered help for " << religion->getName() << ", kw=" << fullKeyword << endl;

    helpManager->registrate( Pointer( this ) );
}

void ReligionHelp::unsetReligion( )
{
    helpManager->unregistrate( Pointer( this ) );
    religion.clear( );
    fullKeyword = "";
}

struct CommaSet : public set<string> {
    void print( ostream &buf ) const {
        bool found = false;
        for (const_iterator i = begin( ); i != end( ); i++) {
            if (found)
                buf << ", ";
            buf << *i;
            found = true;
        }
    }
};
inline ostream& operator << ( ostream& ostr, const CommaSet& cset )
{
    cset.print( ostr );
    return ostr;
}

void ReligionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Религия {C" << religion->getRussianName().ruscase('1') << "{x ({C"
       << religion->getShortDescr() << "{x), ";

    if (religion->isAllowed(ch))
        in << "доступна для тебя.";
    else
        in << "недоступна тебе.";
    in << endl << endl
       << *this;
}

/*----------------------------------------------------------------------
 * DefaultReligion 
 *---------------------------------------------------------------------*/
DefaultReligion::DefaultReligion( )
                : align( 0, &align_table ),
                  ethos( 0, &ethos_table ),
                  races( raceManager )
{
}


const DLString & DefaultReligion::getName( ) const
{
    return Religion::getName( );
}

void DefaultReligion::setName( const DLString &name ) 
{
    this->name = name;
}

bool DefaultReligion::isValid( ) const
{
    return true;
}

bool DefaultReligion::isAllowed( Character *ch ) const
{
    if (!ethos.isSetBitNumber( ch->ethos ))
        return false;

    if (!align.isSetBitNumber( ALIGNMENT(ch) ))
        return false;

    if (!races.empty( ) && !races.isSet( ch->getRace( ) ))
        return false;

    return true;
}

const DLString &DefaultReligion::getRussianName( ) const
{
    return nameRus;
}

const DLString& DefaultReligion::getNameFor( Character *looker ) const
{
    if (!looker || !looker->getConfig( )->ruskills) 
        return shortDescr;

    return nameRus;
}

void DefaultReligion::loaded( )
{
    religionManager->registrate( Pointer( this ) );
    if (help)
        help->setReligion(Pointer(this));
}

void DefaultReligion::unloaded( )
{
    if (help)
        help->unsetReligion();

    religionManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultReligion::getShortDescr( ) const
{
    return shortDescr.getValue( );
}

const DLString & DefaultReligion::getDescription( ) const
{
    return description.getValue( );
}

