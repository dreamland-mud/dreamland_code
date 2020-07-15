/* $Id$
 *
 * ruffina, 2004
 */
#include "badnames.h"
#include "logstream.h"
#include "dl_ctype.h"
#include "mercdb.h"
#include "merc.h"
#include "loadsave.h"
#include "religion.h"

BadNames *badNames = NULL;

BadNames::BadNames( )
{
    checkDuplicate( badNames );
    badNames = this;
}

BadNames::~BadNames( )
{
    badNames = NULL;
}

void BadNames::initialization( )
{
    XMLConfigurablePlugin::initialization( );

    NameList::const_iterator n;
    for (n = patterns.begin( ); n != patterns.end( ); n++) 
        regexps.push_back( RegExp( n->c_str( ) ) );
}

/**
 * Check English name against criteria and return string error code.
 */
DLString BadNames::checkName( const DLString &name ) const
{
    DLString rc;

    if (!(rc = nameLength(name)).empty())
        return rc;

    if (!nameEnglish(name))
        return "bad_en_letter";

    if (!nameMobiles(name))
        return "mob";

    if (!nameReserved(name))
        return "reserved";

    if (!nameReligion(name, false))
        return "god";

    return DLString::emptyString;
}

/**
 * Check English name against criteria and return string error code.
 */
DLString BadNames::checkRussianName( const DLString &name ) const
{
    DLString rc;

    if (!(rc = nameLength(name)).empty())
        return rc;

    if (!nameRussian(name))
        return "bad_ru_letter";

    if (!nameMobiles(name))
        return "mob";

    if (!nameReserved(name))
        return "reserved";

    if (!nameReligion(name, true))
        return "god";

    return DLString::emptyString;
}

bool BadNames::nameReligion(const DLString &name, bool fRussian) const
{
    for (int i = 0; i < religionManager->size(); i++) {
        Religion *rel = religionManager->find(i);
        DLString godName = fRussian ? rel->getRussianName() : rel->getName();

        if (name.equalLess(godName))
            return false;
    }

    return true;
}

DLString BadNames::nameLength( const DLString &name ) const
{
    if (name.length( ) < 2)
        return "too_short";
    if (name.length( ) > 15)
        return "too_long";

    return DLString::emptyString;
}

bool BadNames::nameEnglish( const DLString &name ) const
{
    DLString::const_iterator n;
    for (n = name.begin( ); n != name.end( ); n++)
        if (!isalpha( *n ))
            return false;

    return true;
}

bool BadNames::nameRussian( const DLString &name ) const
{
    DLString::const_iterator n;
    for (n = name.begin( ); n != name.end( ); n++)
        if (!dl_isrusalpha( *n ))
            return false;

    return true;
}

/*
 * Prevent players from naming themselves after mobs.
 */
bool BadNames::nameMobiles( const DLString &name ) const
{
    extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
    MOB_INDEX_DATA *pMobIndex;
    int iHash;
    char mobname[MAX_STRING_LENGTH], player_name[MAX_STRING_LENGTH];

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
        for ( pMobIndex  = mob_index_hash[iHash]; pMobIndex != 0; pMobIndex  = pMobIndex->next ) {
            strcpy( player_name, pMobIndex->player_name );

            for ( ; ; ) {
                strcpy( player_name, one_argument( player_name, mobname ) );
                
                if (mobname[0] == 0)
                    break;
                
                if ( !str_cmp( name.c_str( ), mobname ) )
                    return false;
            }
        }
    }

    return true;
}

bool BadNames::nameReserved( const DLString &name ) const
{
    NameList::const_iterator n;
    for (n = names.begin( ); n != names.end( ); n++)
        if (name.equalLess( *n ))
            return false;
    
    RegexpList::const_iterator r;
    for (r = regexps.begin( ); r != regexps.end( ); r++) 
        if (r->match( name ))
            return false;

    return true;
}

