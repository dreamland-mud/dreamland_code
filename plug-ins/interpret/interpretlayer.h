/* $Id$
 *
 * ruffina, 2004
 */
#ifndef INTERPRETLAYER_H
#define INTERPRETLAYER_H

#include "plugin.h"
#include "dlstring.h"
#include "stringlist.h"

class Descriptor;
class Character;
class CommandInterpreter;
class CommandBase;

enum {
    CMD_PRIO_FIRST  = 1,
    CMD_PRIO_LAST   = 100,
};

enum {
    CMDP_PREFIX     = 100,
    CMDP_SUBST_ALIAS= 200,
    CMDP_LOG_INPUT  = 300,
    CMDP_GRAB_WORD  = 400,
    CMDP_FIND       = 500,
    CMDP_LOG_CMD    = 600
};

struct InterpretArguments
{
    InterpretArguments( );
    
    void advance( int );
    void advance( );
    void splitLine( );
    
    Descriptor *d;
    Character *ch;
    DLString line, cmdName, cmdArgs;
    ::Pointer<CommandBase> pCommand;
    int index;
    int * phases;
    StringList hints1, hints2, translit;
};

class InterpretLayer : public virtual Plugin {
public:
    typedef ::Pointer<InterpretLayer> Pointer;
    
    InterpretLayer( );
    
    virtual bool process( InterpretArguments & ) = 0;
    
protected:
    virtual void initialization( );
    virtual void destruction( );
    virtual void putInto( ) = 0;

    CommandInterpreter *interp;
};

#endif
