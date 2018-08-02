/* $Id: cskills.cpp,v 1.1.2.3.6.2 2008/02/23 13:41:24 rufina Exp $
 *
 * ruffina, 2004
 */

#include "allskillslist.h"
#include "commandtemplate.h"

#include "pcharacter.h"
#include "comm.h"

/*
 * skills sortby name|level|learned
 * skills <group>
 * skills <level1> <level2>
 */
CMDRUN( skills )
{
    AllSkillsList slist;
    std::basic_ostringstream<char> buf;
    DLString argument = constArguments;
    
    slist.fSpells = false;
    slist.cmd = getName( );
    slist.rcmd = getRussianName( );

    if (!slist.parse( argument, buf, ch )) {
	ch->send_to( buf );
	return;
    }
    
    slist.make( ch );
    slist.display( buf );

    page_to_char( buf.str( ).c_str( ), ch );
}



/*
 * spells sortby name|level|learned
 * spells <group>
 * spells <level1> <level2>
 */
CMDRUN( spells )
{
    AllSkillsList slist;
    std::basic_ostringstream<char> buf;
    DLString argument = constArguments;
    
    slist.fSpells = true;
    slist.cmd = getName( );
    slist.rcmd = getRussianName( );

    if (!slist.parse( argument, buf, ch )) {
	ch->send_to( buf );
	return;
    }
    
    slist.make( ch );
    slist.display( buf );

    page_to_char( buf.str( ).c_str( ), ch );
}


