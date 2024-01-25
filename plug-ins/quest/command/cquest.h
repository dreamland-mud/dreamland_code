/* $Id: cquest.h,v 1.1.4.3.6.3 2009/02/15 01:44:56 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef CQUEST_H
#define CQUEST_H

#include "commandplugin.h"

class PCharacter;
class NPCharacter;

class CQuest : public CommandPlugin {
XML_OBJECT;
public:
        typedef ::Pointer<CQuest> Pointer;

        CQuest( );
        
        virtual void run( Character*, const DLString& constArguments );
        
private:
        void doSummary( PCharacter *, const DLString & );
        void doInfo( PCharacter * );
        void doPoints( PCharacter * );
        void doTime( PCharacter * );
        void doSet( PCharacter *, DLString& );
        void doStat( PCharacter * );
        
        void usage( PCharacter * );
        void autoQuestInfo(PCharacter *, ostringstream &);

        static const DLString COMMAND_NAME;
};


#endif
