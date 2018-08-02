/* $Id$
 *
 * ruffina, 2004
 */
#include "wrongcommand.h"

#include "character.h"

WrongCommand::WrongCommand( const DLString &name )
{
    this->name = name;
}

void WrongCommand::run( Character *ch, const DLString & )
{
    ch->println( msgWrong.c_str( ) );
}

