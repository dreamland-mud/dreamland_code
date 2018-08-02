/* $Id: globalquestmanager.h,v 1.1.2.1.6.5 2009/09/24 14:09:12 rufina Exp $
 * 
 * ruffina, 2003
 */

#ifndef GLOBALQUESTMANAGER_H
#define GLOBALQUESTMANAGER_H

#include <map>

#include "schedulertaskroundplugin.h"
#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlpointer.h"
#include "dbio.h"

class PCMemoryInterface;
class GlobalQuestInfo;
class GlobalQuest;
class XMLReward;
struct Descriptor;

class GlobalQuestManager : public SchedulerTaskRoundPlugin, public XMLVariableContainer 
{
XML_OBJECT    
public:

    typedef ::Pointer<GlobalQuestManager> Pointer;
    typedef ::Pointer<GlobalQuestInfo> GlobalQuestInfoPointer;
    typedef ::Pointer<GlobalQuest> GlobalQuestPointer;
    
    typedef std::map<DLString, GlobalQuestPointer> RunList;
    typedef std::map<DLString, GlobalQuestInfoPointer> RegistryList;

    GlobalQuestManager( );
    virtual ~GlobalQuestManager( );

    virtual void run( );
    virtual void after( );
    virtual int getPriority( ) const;
	
    
    void registrate( GlobalQuestInfo * );
    void unregistrate( GlobalQuestInfo * );
    void activate( GlobalQuest * );
    void deactivate( GlobalQuest * );
   
    RunList & getRunning( );
    RegistryList & getRegistry( );
    GlobalQuestPointer findGlobalQuest( const DLString & );
    GlobalQuestInfoPointer findGlobalQuestInfo( const DLString & );

    static void rewardChar( PCMemoryInterface *, XMLReward & );

    void load( GlobalQuestInfo * );
    GlobalQuestPointer loadRT( GlobalQuestInfo * );
    void save( GlobalQuestInfo * );
    void saveRT( GlobalQuest * );
    void removeRT( GlobalQuest * );

    static inline GlobalQuestManager* getThis( ) {
	return thisClass;
    }
    
private:
    
    static const DLString TABLE_NAME;
    static const DLString NODE_NAME_QINFO;
    static const DLString NODE_NAME_QUEST;

    RunList running;
    RegistryList registry;
    DBIO dbioTable, dbioRuntime;
    
    static GlobalQuestManager *thisClass;
};

#endif

