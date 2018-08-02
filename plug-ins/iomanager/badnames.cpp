/* $Id$
 *
 * ruffina, 2004
 */
#include "badnames.h"
#include "dl_ctype.h"
#include "mercdb.h"
#include "merc.h"
#include "loadsave.h"

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

bool BadNames::checkName( const DLString &name ) const
{
    return nameLength( name )
           && nameEnglish( name )
           && nameMobiles( name )
           && nameReserved( name );
}

bool BadNames::checkRussianName( const DLString &name ) const
{
    return nameLength( name )
           && nameRussian( name )
           && nameMobiles( name )
           && nameReserved( name );
}

bool BadNames::nameLength( const DLString &name ) const
{
    if (name.length( ) < 2)
        return false;
    if (name.length( ) > 12)
        return false;
    return true;
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

