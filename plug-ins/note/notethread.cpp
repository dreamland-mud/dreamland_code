/* $Id: notethread.cpp,v 1.1.2.11.6.11 2010-09-01 08:21:32 rufina Exp $
 *
 * ruffina, 2005
 */
#include "notethread.h"
#include "noteattrs.h"
#include "notemanager.h"
#include "note.h"
#include "logstream.h"

#include "commandmanager.h"
#include "commandflags.h"

#include "pcharacter.h"
#include "comm.h"
#include "merc.h"
#include "def.h"

const DLString NoteHelp::TYPE = "NoteHelp";

void NoteHelp::save() const
{
   if (command) {
        NoteThread::Pointer thread = command.getDynamicPointer<NoteCommand>()->getThread();
        if (thread) {
            NoteManager::getThis()->saveXML(*thread, thread->getName());
            return;
        }
   }

   LogStream::sendNotice() << "Failed to save command " << (command ? command->getName() : "") << endl;
}

NoteThread::NoteBucket::NoteBucket( )
                          : NoteThread::NoteBucket::Base( true )
{
}

bool NoteThread::NoteBucket::nodeFromXML( const XMLNode::Pointer& child )
{
    if (Base::nodeFromXML( child )) {
        if (thread->isExpired( &back( ) ))
            pop_back( );
        else
            thread->xnotes.push_back( &back( ) );
            
        return true;
    }
    else
        return false;
}

const DLString NoteThread::NODE_NAME = "NoteBucket";
const int NoteThread::HASH_LEN = 16;

NoteThread::NoteThread( const DLString &n ) 
    : name( n ),
      gender( 0, &sex_table ), 
      godsSeeAlways( false )
{
    hash.resize( HASH_LEN );
}

void NoteThread::initialization( )
{
    NoteManager::getThis( )->load( this );

    if (command)
        command->setThread(Pointer(this));

    loadAllBuckets( );
}

void NoteThread::destruction( )
{
//  saveAllBuckets( );
    NoteManager::getThis( )->unLoad( this );

    if (command)
        command->unsetThread();
}

DLString NoteThread::getTableName( ) const
{
    return NoteManager::getThis( )->getTableName( ) + '/' + getName( );
}

DLString NoteThread::getNodeName( ) const
{
    return NODE_NAME;
}

static inline bool __cmp_note_ptr__( const Note *a, const Note *b )
{
    return *a < *b;
}

void NoteThread::loadAllBuckets( )
{
    xnotes.clear( );

    for (short i = 0; i < HASH_LEN; i++) {
        hash[i].setThread( this );
        loadXML( &hash[i], DLString( i ), true );
    }

    xnotes.sort( __cmp_note_ptr__ );
}

void NoteThread::saveAllBuckets( ) const
{
    for (short i = 0; i < HASH_LEN; i++) 
        saveXML( &hash[i], DLString( i ) );
}

void NoteThread::saveBucket( time_t stamp ) const
{
    short id = stamp % HASH_LEN;
    
    saveXML( &hash[id], DLString( id ), true );
}

void NoteThread::attach( const Note *note )
{
    NoteBucket &b = hash[note->getID( ) % HASH_LEN];
    
    b.push_back( *note );
    xnotes.push_back( &b.back( ) ); 
    
    saveBucket( note->getID( ) );
}

void NoteThread::remove( const Note *note )
{
    NoteList::iterator i;
    NoteBucket::iterator j;
    short id = note->getID( ) % HASH_LEN;

    for (i = xnotes.begin( ); i != xnotes.end( ); i++)
        if (note == *i) {
            xnotes.erase( i );
            break;
        }
    
    for (j = hash[id].begin( ); j != hash[id].end( ); j++)
        if (note == &*j) {
            hash[id].erase( j );
            saveXML( &hash[id], DLString( id ), true );
            break;
        }
}

void NoteThread::dump( ) const
{
    NoteList::const_iterator i;
    XMLListBase<Note> xmlNotes( true );
    
    for (i = xnotes.begin( ); i != xnotes.end( ); i++)
        xmlNotes.push_back( **i );

    saveXML( &xmlNotes, "all" );
}

bool NoteThread::canWrite( const PCharacter *ch ) const
{
    return ch->get_trust( ) >= writeLevel.getValue( );
}

bool NoteThread::canRead( const PCharacter *ch ) const
{
    return ch->get_trust( ) >= readLevel.getValue( );
}

bool NoteThread::isExpired( const Note *note ) const
{
    return keepDays.getValue( ) > 0 
            && dreamland->getCurrentTime( ) 
                    > keepDays.getValue( ) * 24 * 60 * 60 + note->getID( );
}

time_t NoteThread::getStamp( PCharacter *ch ) const
{
    return ch->getAttributes( ).getAttr<XMLAttributeLastRead>("lastread" )->getStamp( this );
}

int NoteThread::countSpool( PCharacter *ch ) const
{
    NoteList::const_iterator i;
    int cnt = 0;
    time_t stamp = getStamp( ch );

    for (i = xnotes.begin( ); i != xnotes.end( ); i++)
        if (!isNoteHidden( *i, ch, stamp ))
            cnt++;
    
    return cnt;
}

bool NoteThread::isNoteHidden( const Note *note, PCharacter *ch, time_t stamp ) const
{
    if (!canRead( ch ))
        return true;

    if (note->getID( ) <= stamp)
        return true;
    
    if (note->isNoteFrom( ch ))
        return true;

    if (!note->isNoteTo( ch ))
        return true;

    return false;
}

const Note * NoteThread::getNoteAtPosition( PCharacter *ch, int anum ) const
{
    NoteList::const_iterator i;
    int vnum;
    
    if (anum < 0 || anum >= (int)xnotes.size( ))
        return NULL;

    for (vnum = 0, i = xnotes.begin( ); i != xnotes.end( ); i++) 
        if ((*i)->isNoteTo( ch ) || (*i)->isNoteFrom( ch )) 
            if (anum == vnum++) 
                return *i;

    return NULL;
}

const NoteThread::NoteList & NoteThread::getNoteList( ) const
{
    return xnotes;
}

const Note * NoteThread::getNextUnreadNote( PCharacter *ch ) const
{
    NoteList::const_iterator i;
    time_t stamp = getStamp( ch );

    for (i = xnotes.begin( ); i != xnotes.end( ); i++) 
        if (!isNoteHidden( *i, ch, stamp )) 
            return *i;

    return NULL;
}

int NoteThread::getNoteNumber( PCharacter *ch, const Note *note ) const
{
    NoteList::const_iterator i;
    int vnum = 0;

    for (i = xnotes.begin( ); i != xnotes.end( ); i++) {
        if (note == *i)
            return vnum;
            
        if ((*i)->isNoteTo( ch ) || (*i)->isNoteFrom( ch ))
            vnum++;
    }
    
    return vnum;
}

void NoteThread::showNoteToChar( PCharacter *ch, const Note *note ) const
{
    ostringstream buf;

    note->toStream( getNoteNumber( ch, note ), buf );
    page_to_char( buf.str( ).c_str( ), ch );
    ch->getAttributes( ).getAttr<XMLAttributeLastRead>( 
                        "lastread" )->updateStamp( this, note );
}

const Note * NoteThread::findNote( time_t id ) const
{
    NoteList::const_iterator i;
    for (i = xnotes.begin( ); i != xnotes.end( ); i++) 
        if ((*i)->getID( ) == id)
            return *i;

    return NULL;
}

Note * NoteThread::findNote( time_t id )
{
    NoteList::iterator i;
    for (i = xnotes.begin( ); i != xnotes.end( ); i++) 
        if ((*i)->getID( ) == id)
            return *i;

    return NULL;
}

const Note * NoteThread::findNextNote( PCharacter *ch, time_t id ) const
{
    NoteList::const_iterator i;

    for (i = xnotes.begin( ); i != xnotes.end( ); i++) 
        if ((*i)->getID( ) > id && (*i)->isNoteTo( ch ))
            return *i;

    return NULL;
}

