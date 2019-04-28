/* $Id: defaultcommand.cpp,v 1.1.2.7 2008/07/31 04:56:05 rufina Exp $
 *
 * ruffina, 2007
 */

#include "defaultcommand.h"
#include "dl_ctype.h"
#include "logstream.h"

#include "merc.h"
#include "def.h"

DefaultCommand::DefaultCommand( ) 
    : extra( defaultExtra.getValue( ), defaultExtra.getTable( ) ), 
      position( defaultPosition.getValue( ), defaultPosition.getTable( ) ),
      order( defaultOrder.getValue( ), defaultOrder.getTable( ) ),
      cat(defaultCategory.getValue(), defaultCategory.getTable())
{
    
}

DefaultCommand::~DefaultCommand( )
{
}

const DLString & DefaultCommand::getRussianName( ) const
{
    if (russian.empty( ))
        return DLString::emptyString;
    else 
        return russian.front( );
}

bool DefaultCommand::matchesExactly( const DLString &cmdName ) const
{
    if (Command::matchesExactly( cmdName ))
        return true;

    if (dl_isrusalpha( cmdName.at( 0 ) )) {
        for (XMLStringList::const_iterator i = russian.begin( ); i != russian.end( ); i++) 
            if (*i == cmdName) 
                return true;
    } else {
        for (XMLStringList::const_iterator i = aliases.begin( ); i != aliases.end( ); i++) 
            if (*i == cmdName) 
                return true;
    }

    return false;
}

bool DefaultCommand::matchesAlias( const DLString& command ) const
{
    if (Command::matchesAlias( command ))
        return true;

    if (command.empty( ))
        return false;
    
    if (dl_isrusalpha( command.at( 0 ) )) {
        for (XMLStringList::const_iterator r = russian.begin( ); r != russian.end( ); r++) 
            if (command.strPrefix( *r )) 
                return true;
    }
    else {
        for (XMLStringList::const_iterator a = aliases.begin( ); a != aliases.end( ); a++) 
            if (command.strPrefix( *a )) 
                return true;
    }

    return false;
}

void DefaultCommand::run( Character * ch, const DLString & constArguments ) 
{
    char argument[MAX_STRING_LENGTH];

    strcpy( argument, constArguments.c_str( ) );
    run( ch, argument );
}

void DefaultCommand::run( Character *, char * ) 
{ 
}


const XMLStringList & DefaultCommand::getAliases( ) const
{
    return aliases;
}

const XMLStringList & DefaultCommand::getRussianAliases( ) const
{
    return russian;
}
