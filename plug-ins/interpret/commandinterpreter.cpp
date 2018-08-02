/* $Id$
 *
 * ruffina, 2004
 */

#include "commandinterpreter.h"
#include "commandbase.h"

InterpretPhase::InterpretPhase( )
{
}

bool InterpretPhase::run( InterpretArguments &iargs )
{
    iterator j;

    for (j = begin( ); j != end( ); j++) {
	int index = iargs.index;
	
	if (!j->second->process( iargs ))
	    return false;
	
	if (index != iargs.index)
	    return true;
    }
    
    iargs.advance( );
    return true;
}

void InterpretPhase::eraseLayer( InterpretLayer *lay )
{
    iterator j, j_next;

    for (j = begin( ); j != end( ); j = j_next) {
	j_next = j;
	j_next++;

	if (j->second.getPointer( ) == lay) 
	    erase( j );
    }
}

CommandInterpreter * CommandInterpreter::thisClass = NULL;

CommandInterpreter::CommandInterpreter( )
{
    thisClass = this;
}

CommandInterpreter::~CommandInterpreter( )
{
    thisClass = NULL;
}

void CommandInterpreter::initialization(  )
{
}

void CommandInterpreter::destruction(  )
{
}

void CommandInterpreter::put( InterpretLayer *lay, int phase, int prio )
{
    phases[phase][prio] = lay;
}

void CommandInterpreter::eraseLayer( InterpretLayer *lay )
{
    InterpretPhasesMap::iterator i;
    
    for (i = phases.begin( ); i != phases.end( ); i++) 
	i->second.eraseLayer( lay );
}

bool CommandInterpreter::advance( InterpretPhasesMap::iterator &iter,
                                  InterpretArguments &iargs )
{
    for ( ; iargs.phases[iargs.index] != 0; iargs.index++) {
	InterpretPhasesMap::iterator i;
	
	for (i = iter; i != phases.end( ); i++)
	    if (i->first >= iargs.phases[iargs.index]) {
		iter = i;
		return true;
	    }
    }

    return false;
}

void CommandInterpreter::run( InterpretArguments &iargs )
{
    InterpretPhasesMap::iterator iter = phases.begin( );
    
    runPhase( iargs, iter );
}

void CommandInterpreter::runPhase( InterpretArguments &iargs, 
                                   InterpretPhasesMap::iterator &iter )
{
    if (advance( iter, iargs )) 
	if (iter->second.run( iargs ))
	    runPhase( iargs, iter );
}

