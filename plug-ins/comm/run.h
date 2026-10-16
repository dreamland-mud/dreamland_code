/* $Id: run.h,v 1.5.2.6.6.4 2008/02/24 05:12:24 rufina Exp $
 * 
 * ruffina, 2004
 */

#ifndef __RUN_H__
#define __RUN_H__

#include <set>
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlvariablecontainer.h"
#include "xmlattribute.h"

#include "schedulertaskroundpcharacter.h"
#include "schedulertaskroundplugin.h"

class PCharacter;

class SpeedWalkUpdateTask : 
                            public SchedulerTaskRoundPlugin,
                            public virtual SchedulerTaskRoundPCharacter
{
public:
    typedef ::Pointer<SpeedWalkUpdateTask> Pointer;

    virtual void run( PCharacter * );
    virtual void after( );
};


class XMLAttributeSpeedWalk : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT    
public: 
    typedef ::Pointer<XMLAttributeSpeedWalk> Pointer;

    inline bool isEmpty( ) const;
    inline void setValue( const DLString & );
    
    inline void incSteps( );
    inline void clearSteps( );
    inline int getSteps( ) const;
    inline void startLeg( int );
    inline bool visit( int );

    char getFirstCommand( ) const;
    void clearFirstCommand( );
    int getFirstDoor( ) const;
    bool isLastStep( ) const;
    void show(PCharacter *) const;
    
private:
    XML_VARIABLE XMLString path;
    XML_VARIABLE XMLInteger steps;
    // Rooms the current run-to-the-end leg has passed; not saved.
    std::set<int> legRooms;
};


inline bool XMLAttributeSpeedWalk::isEmpty( ) const
{
    return path.getValue( ).empty( );
}

inline void XMLAttributeSpeedWalk::setValue( const DLString& value )
{
    path.setValue( value );
    steps = 0;
}

inline void XMLAttributeSpeedWalk::incSteps( )
{
    steps++;
}

inline void XMLAttributeSpeedWalk::clearSteps( )
{
    steps = 0;
}

inline int XMLAttributeSpeedWalk::getSteps( ) const
{
    return steps.getValue( );
}

inline void XMLAttributeSpeedWalk::startLeg( int vnum )
{
    legRooms.clear( );
    legRooms.insert( vnum );
}

/** Remember a room on this leg; false if the leg has been there already. */
inline bool XMLAttributeSpeedWalk::visit( int vnum )
{
    return legRooms.insert( vnum ).second;
}

#endif
