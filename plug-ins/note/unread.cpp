/* $Id: unread.cpp,v 1.1.2.8.6.3 2009/08/16 02:50:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "unread.h"
#include "notethread.h"
#include "notemanager.h"
#include "noteattrs.h"
#include "note.h"

#include "so.h"
#include "class.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "arg_utils.h"

/*-----------------------------------------------------------------------------
 * 'unread' command 
 *-----------------------------------------------------------------------------*/
COMMAND(Unread, "unread")
{
    PCharacter *pch;
    DLString args = constArguments;
    
    if (ch->is_npc( ))
        return;
    
    pch = ch->getPC( );
    
    if (args.empty( )) {
        doSpool( pch, true );
        doUnfinished( pch );
    }
    else if (arg_oneof( args, "next", "следующий", "дальше" ))
        doNext( pch );
}

void Unread::doSpool( PCharacter *ch, bool fVerbose )
{
    ostringstream buf;
    NoteManager::Threads::const_iterator i;
    const NoteManager::Threads &th = NoteManager::getThis( )->getThreads( );

    for (i = th.begin( ); i != th.end( ); i++) {
        int count = i->second->countSpool( ch );

        if (count > 0)
            i->second->getUnreadMessage( count, buf );
    }

    if (!buf.str( ).empty( )) {
        if (!fVerbose)
            ch->pecho("");
        ch->send_to( buf );
    }
    else if (fVerbose)
        ch->pecho("У тебя нет непрочитанных сообщений.");
}


void Unread::doUnfinished( PCharacter *ch )
{
    XMLAttributeNoteData::Pointer attr;
    XMLAttributeNoteData::iterator i;

    attr = ch->getAttributes( ).findAttr<XMLAttributeNoteData>( "notedata" );

    if (attr)
        for (i = attr->begin( ); i != attr->end( ); i++) {
            const DLString &threadName = i->second.getThreadName( );
            NoteThread::Pointer thread = NoteManager::getThis( )->findThread( threadName );

            ch->pecho("Ты не закончи%Gло|л|ла писать {W%N4{x!", 
                      ch, thread ? thread->getRussianThreadName( ).c_str( ) : threadName.c_str( ) );
        }
}

void Unread::doNext( PCharacter *ch )
{
    NoteManager::Threads::const_iterator i;
    const NoteManager::Threads &th = NoteManager::getThis( )->getThreads( );

    const NoteThread *oldest = NULL;
    const Note *onote = NULL;
    time_t minStamp = 2000000000, stamp;

    for (i = th.begin( ); i != th.end( ); i++) {
        const Note *note = i->second->getNextUnreadNote( ch );

        if (note) {
            stamp = note->getID( );

            if (stamp <= minStamp) {
                minStamp = stamp;
                oldest = *i->second;
                onote = note;
            }
        }
    }

    if (!oldest) 
        ch->pecho("У тебя нет непрочитанных сообщений.");
    else {
        ch->pecho( "{W%s{x:", oldest->getName( ).c_str( ) );
        oldest->showNoteToChar( ch, onote );
    }
}

/*-----------------------------------------------------------------------------
 * UnreadListener: notify on logon 
 *-----------------------------------------------------------------------------*/

void UnreadListener::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;

    if (!d->character || !( ch = d->character->getPC( ) ))
        return;
    
    if (oldState == CON_CREATE_DONE && ch->getRemorts( ).size( ) == 0) {
        time_t stamp;
        XMLAttributeLastRead::Pointer attr;
        NoteManager::Threads::const_iterator i;
        const NoteManager::Threads &th = NoteManager::getThis( )->getThreads( );

        stamp = dreamland->getCurrentTime( ) - 2 * Date::SECOND_IN_MONTH; // 2 months ago
        attr = ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" );
        
        for (i = th.begin( ); i != th.end( ); i++) 
            if (i->second->canRead( ch )) 
                attr->setStamp( *i->second, stamp );
            
    }
    else if (newState == CON_PLAYING) {
        Unread::doSpool( ch, false );
        Unread::doUnfinished( ch );
    }
}

