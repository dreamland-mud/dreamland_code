/* $Id$
 *
 * ruffina, 2004
 */
#include <stdarg.h>

#include "logstream.h"

#include "commandinterpreter.h"
#include "commandbase.h"
#include "character.h"

#include "interp.h"
#include "merc.h"
#include "def.h"

bool interpret( Character *ch, const char *line )
{
    InterpretArguments iargs;
    static int phases [] = { 
        CMDP_LOG_INPUT,
        CMDP_GRAB_WORD,
        CMDP_FIND,
        CMDP_LOG_CMD,
        0
    };

    iargs.d = ch->desc;
    iargs.ch = ch;
    iargs.line = line;
    iargs.phases = phases;

    CommandInterpreter::getThis( )->run( iargs );
    
    if (!iargs.pCommand)
        return false;

    if (iargs.pCommand->dispatch( iargs ) == RC_DISPATCH_OK)
        iargs.pCommand->entryPoint( ch, iargs.cmdArgs );

    return true;
}

void interpret_fmt( Character *ch, const char *format, ... ) 
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, format );
    vsprintf( buf, format, ap );
    va_end( ap );
    
    interpret( ch, buf );
}

bool interpret_cmd( Character *ch, const char *cmd, const char *argsFormat, ... )
{
    InterpretArguments iargs;
    static int phases [] = {
        CMDP_FIND,
        0
    };
    char args[MAX_STRING_LENGTH];
    va_list ap;
    
    va_start( ap, argsFormat );
    vsprintf( args, argsFormat, ap );
    va_end( ap );

    iargs.d = ch->desc;
    iargs.ch = ch;
    iargs.cmdName = cmd;
    iargs.cmdArgs = args;
    iargs.phases = phases;

    CommandInterpreter::getThis( )->run( iargs );

    if (iargs.pCommand)
        if (iargs.pCommand->dispatch( iargs ) == RC_DISPATCH_OK) {
            iargs.pCommand->entryPoint( ch, iargs.cmdArgs );
            return true;
        }

    return false;
}

void interpret_raw( Character *ch, const char *cmd, const char *format, ... ) 
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;
    InterpretArguments iargs;
    static int phases [] = {
        CMDP_FIND,
        0
    };

    va_start( ap, format );
    vsprintf( buf, format, ap );
    va_end( ap );
    
    iargs.d = ch->desc;
    iargs.ch = ch;
    iargs.cmdName = cmd;
    iargs.cmdArgs = buf;
    iargs.phases = phases;

    CommandInterpreter::getThis( )->run( iargs );

    if (iargs.pCommand)
        iargs.pCommand->entryPoint( ch, iargs.cmdArgs );
//    else
//        LogStream::sendWarning( ) << "No command '" << cmd << "' for raw interpret!" << endl;
}


