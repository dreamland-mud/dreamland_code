/* $Id: cgquest.h,v 1.1.2.1.6.3 2009/02/15 01:43:34 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef CGQUEST_H
#define CGQUEST_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;
namespace Scripting {
class IdRef;
};

class CGQuest : public CommandPlugin, public DefaultCommand {
public:
        typedef ::Pointer<CGQuest> Pointer;

        CGQuest( );
        
        virtual void run( Character*, const DLString& constArguments );

        static bool gqprog( PCharacter *, Scripting::IdRef & );
        static bool gqprog_info( PCharacter * );
        static bool gqprog_progress( PCharacter * );
        static bool gqprog_notify( PCharacter * );

private:
        void doInfo( PCharacter * );
        void doProgress( PCharacter * );
        void doNoExp( PCharacter *, DLString& );
        void doVictory( PCharacter * );
        void doStat( PCharacter * );
        void doSet( PCharacter *, DLString& );
        void doList( PCharacter * );
        void doStart( PCharacter *, DLString& );
        void doStop( PCharacter *, DLString& );
        void doTime( PCharacter *, DLString& );
        void doTalk( PCharacter *, DLString& );
        void doAuto( PCharacter *, DLString& );
        void doRead( PCharacter *, DLString& );

        void usage( PCharacter * );
        
        static const DLString COMMAND_NAME;
};

#endif
