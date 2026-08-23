/* $Id$
 *
 * ruffina, 2004
 */
#include "clantitles.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "act.h"
#include "merc.h"

#include "def.h"

/*-----------------------------------------------------------------
 * ClanTitles
 *----------------------------------------------------------------*/
ClanTitles::~ClanTitles( )
{
}

/*-----------------------------------------------------------------
 * ClanLevelNames
 *----------------------------------------------------------------*/
ClanLevelNames::~ClanLevelNames( )
{
}

void ClanLevelNames::toStream( ostringstream &buf ) const
{
    buf << fmt(0, "%24s (%24s)  ", 
            male.getValue( ).c_str( ), 
            female.getValue( ).c_str( ) );

    if (!abbr.getValue( ).empty( ))
        buf << fmt(0, "[%2s]  ", abbr.getValue( ).c_str( ) );

    buf << english;
    if (!ukrainian.getValue( ).empty( ))
        buf << " / " << ukrainian;
}

/*-----------------------------------------------------------------
 * ClanTitlesByClass
 *----------------------------------------------------------------*/
const DLString ClanTitlesByClass::TYPE = "ClanTitlesByClass";

const DLString & ClanTitlesByClass::build( PCMemoryInterface *pcm, lang_t lang ) const
{
    const_iterator i = find( pcm->getProfession( )->getName( ) );

    if (i == end( )) {
        static DLString allName( "all" );
        i = find( allName );
    }

    const ClanLevelNames &names = i->second[pcm->getClanLevel( )];
    if (lang == LANG_EN && !names.english.getValue( ).empty( ))
        return names.english.getValue( );
    if (lang == LANG_UA && !names.ukrainian.getValue( ).empty( ))
        return names.ukrainian.getValue( );
    return (pcm->getSex( ) == SEX_FEMALE
                ? names.female.getValue( ) : names.male.getValue( ));
}

void ClanTitlesByClass::toStream( ostringstream &buf ) const
{
    const_iterator i;

    for (i = begin( ); i != end( ); i++) {
        if (i->first == "all")
            buf << "{WДля всех классов:{x" << endl;
        else
            buf << "{WДля класса " << i->first << ":{x" << endl;
        
        for (int j = 0; j < (int) i->second.size( ); j++) {
            buf << fmt(0, "%-3d", j );
            i->second[j].toStream( buf );
            buf << endl;
        }

        buf << endl;
    }
}

int ClanTitlesByClass::size( ) const
{ 
    const_iterator i = begin( );

    if (i == end( ))
        return 0;

    return i->second.size( );
}

/*-----------------------------------------------------------------
 * ClanTitlesByLevel
 *----------------------------------------------------------------*/
const DLString ClanTitlesByLevel::TYPE = "ClanTitlesByLevel";

const DLString & ClanTitlesByLevel::build( PCMemoryInterface *pcm, lang_t lang ) const
{
    int cl = pcm->getClanLevel( );

    if (cl >= size( ))
        return DLString::emptyString;

    const ClanLevelNames &names = (*this)[cl];

    if (lang == LANG_EN && !names.english.getValue( ).empty( ))
        return names.english.getValue( );
    if (lang == LANG_UA && !names.ukrainian.getValue( ).empty( ))
        return names.ukrainian.getValue( );
    return (pcm->getSex( ) == SEX_FEMALE
                ? names.female.getValue( ) : names.male.getValue( ));
}

void ClanTitlesByLevel::toStream( ostringstream &buf ) const
{
    for (int j = 0; j < (int) size( ); j++) {
        buf << fmt(0, "%-3d", j );
        (*this)[j].toStream( buf );
        buf << endl;
    }
}

int ClanTitlesByLevel::size( ) const
{
    return ClanLevelNamesVector::size( );
}

