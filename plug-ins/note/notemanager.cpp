/* $Id: notemanager.cpp,v 1.1.2.6.6.1 2007/06/26 07:18:19 rufina Exp $
 *
 * ruffina, 2005
 */
#include "notemanager.h"
#include "notethread.h"

const DLString NoteManager::TABLE_NAME = "notes";
const DLString NoteManager::NODE_NAME = "NoteThread";
NoteManager* NoteManager::thisClass = 0;

NoteManager::NoteManager( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

NoteManager::~NoteManager( )
{
    thisClass = 0;
}

void NoteManager::initialization( ) 
{
}
void NoteManager::destruction( ) 
{
}

DLString NoteManager::getTableName( ) const
{
    return TABLE_NAME;
}
DLString NoteManager::getNodeName( ) const
{
    return NODE_NAME;
}

void NoteManager::load( NoteThread *thread )
{
    DLString name = thread->getName( );

    loadXML( thread, name );
    threads[name] = NoteThread::Pointer( thread );
}

void NoteManager::unLoad( const NoteThread *thread ) 
{
    DLString name = thread->getName( );
    Threads::iterator ipos = threads.find( name );
    
//    saveXML( thread, name );
    threads.erase( ipos );
}

NoteThread::Pointer NoteManager::findThread( const DLString& name )
{
    Threads::iterator i = threads.find( name );
    
    if (i != threads.end( ))
	return i->second;
    else
	return NoteThread::Pointer( );
}
