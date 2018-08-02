/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WORLDKNOWLEDGE_H
#define WORLDKNOWLEDGE_H

#include <set>
#include <map>
#include <sstream>
#include "dlstring.h"
#include "schedulertaskroundplugin.h"
#include "oneallocate.h"

using namespace std;

class PCharacter;

class WorldKnowledge : public SchedulerTaskRoundPlugin, public OneAllocate {
public:
    typedef ::Pointer<WorldKnowledge> Pointer;
    typedef long long PlayerID;
    typedef int RoomVnum;
    typedef set<RoomVnum> VisitedRooms;
    typedef map<PlayerID, VisitedRooms> Visits;

    WorldKnowledge( );
    virtual ~WorldKnowledge( );
    
    virtual void initialization( );
    virtual void destruction( );
    virtual void run( );
    virtual int getPriority( ) const;

    void visit( PCharacter * );
    void report( PCharacter *, ostringstream & );
    void load( );
    void save( );
    
protected:
    Visits visits; 
    static const DLString FILE_NAME;
};

extern WorldKnowledge *worldKnowledge;

#endif
