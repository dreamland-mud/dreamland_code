/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DREAMS_H__
#define __DREAMS_H__

#include "xmlvector.h"
#include "notethread.h"
#include "schedulertaskroundpcharacter.h"
#include "schedulertaskroundplugin.h"
#include "oneallocate.h"
#include "playerattributes.h"

class DreamThread;
extern DreamThread *dreamThread;

class DreamThread : public NoteThread, public OneAllocate {
XML_OBJECT
public:
    typedef ::Pointer<DreamThread> Pointer;
    
    DreamThread( );
    virtual ~DreamThread( );
    
    virtual void getUnreadMessage( int, ostringstream & ) const;
    virtual bool canWrite( const PCharacter * ) const;

protected:
    static const DLString THREAD_NAME;    
};

class XMLAttributeDream : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeDream> Pointer;

    XMLAttributeDream( );
    virtual ~XMLAttributeDream( );
    
    void run( PCharacter * );

protected:
    void setLines( const DLString & );

    XML_VARIABLE XMLLong sleepTime;
    XML_VARIABLE XMLLong lastDreamID;
    XML_VARIABLE XMLLong currentDreamID;
    XML_VARIABLE XMLListBase<XMLString> lines;
};

class DreamManager : public virtual SchedulerTaskRoundPlugin,
                     public virtual SchedulerTaskRoundPCharacter,
                     public OneAllocate 
{
public:
    typedef ::Pointer<DreamManager> Pointer;
    
    DreamManager( );
    virtual ~DreamManager( );

    virtual void run( PCharacter* );
    virtual void after( );

private:
    static DreamManager *thisClass;
};

#endif
