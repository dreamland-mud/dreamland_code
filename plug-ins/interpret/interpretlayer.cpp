/* $Id$
 *
 * ruffina, 2004
 */
#include "dl_ctype.h"
#include "logstream.h"
#include "act.h"
#include "interpretlayer.h"
#include "commandinterpreter.h"
#include "commandbase.h"


InterpretArguments::InterpretArguments( ) 
            : d( 0 ), ch( 0 ), index( 0 ), phases( NULL )
{
}

void InterpretArguments::advance( int newPhase )
{
    int i;

    for (i = index + 1; phases[i] != 0; i++)
        if (phases[i] >= newPhase) {
            index = i;
            return;
        }
}

void InterpretArguments::advance( )
{
    index++;
}

void InterpretArguments::splitLine( )
{
    const char *arg;

    line.stripLeftWhiteSpace( );
    arg = line.c_str( );
    
    if (line.empty( )) {
        cmdArgs = cmdName = line;
        return;
    }
    
    if (dl_isalpha( arg[0] )
                || isdigit( arg[0] )
                || (arg[0] == '*' && arg[1] != ' '))
    {
        cmdArgs = line;
        cmdName = cmdArgs.getOneArgument( );
    }
    else
    {
        cmdName.assign( arg[0] );
        cmdArgs = line;
        cmdArgs.erase( 0, 1 );
        cmdArgs.stripLeftWhiteSpace( );
    }
}   

InterpretLayer::InterpretLayer( )
{
    interp = CommandInterpreter::getThis( );
}

void InterpretLayer::initialization( )
{
    putInto( );
}

void InterpretLayer::destruction( )
{
    interp->eraseLayer( this );
}

