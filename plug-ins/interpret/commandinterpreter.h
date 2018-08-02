/* $Id$
 *
 * ruffina, 2004
 */
#ifndef COMMANDINTERPRETER_H
#define COMMANDINTERPRETER_H

#include <vector>
#include <map>

#include "plugin.h"
#include "interpretlayer.h"
#include "oneallocate.h"


class InterpretPhase : public map<int, InterpretLayer::Pointer> {
public:
    InterpretPhase( );

    bool run( InterpretArguments & );
    void eraseLayer( InterpretLayer * );
};

typedef map<int, InterpretPhase> InterpretPhasesMap;
    

class CommandInterpreter : public OneAllocate, public Plugin {
public:
    CommandInterpreter( );
    ~CommandInterpreter( );
    
    virtual void initialization( );
    virtual void destruction( );

    void run( InterpretArguments & );
    void eraseLayer( InterpretLayer * );
    void put( InterpretLayer *, int, int );
    
    inline static CommandInterpreter *getThis( );

protected:
    bool advance( InterpretPhasesMap::iterator &, InterpretArguments & );
    void runPhase( InterpretArguments &, InterpretPhasesMap::iterator & );

    InterpretPhasesMap phases;

private:
    static CommandInterpreter *thisClass;
};

inline CommandInterpreter * CommandInterpreter::getThis( )
{
    return thisClass;
}

#endif
