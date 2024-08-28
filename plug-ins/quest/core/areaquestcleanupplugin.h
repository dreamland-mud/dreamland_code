#ifndef AREAQUESTCLEANUPPLUGIN_H 
#define AREAQUESTCLEANUPPLUGIN_H 

#include "descriptorstatelistener.h"

/** Auto-cancel stale area quests on player logon. */
class AreaQuestCleanupPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<AreaQuestCleanupPlugin> Pointer;

        virtual void run( int, int, Descriptor * );        
};



#endif
