/* $Id: cquest.h,v 1.1.4.3.6.3 2009/02/15 01:44:56 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef CQUEST_H
#define CQUEST_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;
class NPCharacter;

class CQuest : public CommandPlugin, public DefaultCommand {
XML_OBJECT;
public:
        typedef ::Pointer<CQuest> Pointer;

        CQuest( );
        
        virtual void run( Character*, const DLString& constArguments );
        
private:
        void doInfo( PCharacter * );
        void doNewInfo( PCharacter *, const DLString & );
        void doPoints( PCharacter * );
        void doTime( PCharacter * );
        void doSet( PCharacter *, DLString& );
        void doStat( PCharacter * );
        
        bool gprog_questinfo( PCharacter * );
        void usage( PCharacter * );

        static const DLString COMMAND_NAME;
};


#endif
