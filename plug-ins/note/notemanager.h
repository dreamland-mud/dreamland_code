/* $Id: notemanager.h,v 1.1.2.5.6.1 2007/06/26 07:18:19 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __NOTEMANAGER_H__
#define __NOTEMANAGER_H__

#include <map>
#include <list>

#include "oneallocate.h"
#include "plugin.h"
#include "dlxmlloader.h"

class Note;
class NoteThread;
typedef ::Pointer<NoteThread> NoteThreadPointer;

class NoteManager : public OneAllocate, public Plugin, public DLXMLLoader {
public:
    typedef std::map<DLString, NoteThreadPointer> Threads;
    
    NoteManager( );
    virtual ~NoteManager( );
    
    virtual void initialization( );
    virtual void destruction( );

    void load( NoteThread * );
    void unLoad( const NoteThread * );
    
    inline Threads & getThreads( );
    NoteThreadPointer findThread( const DLString & );
    
    virtual DLString getTableName( ) const;
    virtual DLString getNodeName( ) const;
    
    inline static NoteManager *getThis( );

private:
    Threads threads;
    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;
    static NoteManager *thisClass;
};

inline NoteManager::Threads & NoteManager::getThreads( )
{
    return threads;
}
inline NoteManager * NoteManager::getThis( )
{
    return thisClass;
}

#endif
