/* $Id$
 *
 * ruffina, 2004
 */
#include "dreams.h"
#include "noteattrs.h"
#include "logstream.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "merc.h"

#include "act.h"
#include "def.h"
#include "l10n.h"

/*-----------------------------------------------------------------------
 * DreamThread
 *----------------------------------------------------------------------*/
DreamThread *dreamThread = 0;
const DLString DreamThread::THREAD_NAME = "dream";

DreamThread::DreamThread( )
                : NoteThread( THREAD_NAME )
{
    checkDuplicate( dreamThread );
    dreamThread = this;
}
    
DreamThread::~DreamThread( )
{
    dreamThread = 0;
}

void DreamThread::getUnreadMessage( PCharacter *ch, int count, ostringstream &buf ) const
{
    // Example: Смертным приснили {W10{x новых снов ('{yсон{x').
    buf << fmt( ch, _("Смертным приснили {W%1$d{x нов%1$Iый|ых|ых с%1$Iон|на|нов ('{yсон{x')."), count ) << endl;
}

/* Mortals meet new dreams only in their sleep or in 'dream list': no login
 * count, no crystal-orb ping, no 'unread next'. Gods are told as before. */
bool DreamThread::announcesUnread( PCharacter *ch ) const
{
    return ch->is_immortal( );
}

/* Catalog file-key for the text of every dream. The dreams live in the note
 * database, not in this source file, so they are keyed under a name of their
 * own rather than a __FILE__ -- see config/translations/notes.json. */
static const DLString DREAM_L10N_FILE = "notes/dream";

/* A dream never tells a mortal who sent it, not even when they read it again
 * from the list. Gods still see the author. */
DLString DreamThread::getNoteFrom( const Note *note, PCharacter *ch ) const
{
    if (ch->is_immortal( ))
        return NoteThread::getNoteFrom( note, ch );

    return l( ch, "Фабрика грез" );
}

// Dreams are DATA, not a literal, so they cannot carry an _() wrapper --
// key the whole body under a catalog file of its own instead. Anything
// the catalog does not know (every dream a god writes from now on)
// falls back to its Russian text, byte for byte.
DLString DreamThread::getNoteText( const Note *note, PCharacter *ch ) const
{
    return MultiMessage( note->getText( ), DREAM_L10N_FILE ).getMessage( ch );
}

/* Old dreams are reruns, not a backlog: never nag a player with the
 * 'readall' banner because of them. */
bool DreamThread::hasFlooredBacklog( PCharacter * ) const
{
    return false;
}

bool DreamThread::canWrite( const PCharacter *ch ) const
{
    if (!NoteThread::canWrite( ch ))
        return false;

    return !ch->getAttributes( ).isAvailable( "nochannel" );
}

/*-----------------------------------------------------------------------
 * XMLAttributeDream
 *----------------------------------------------------------------------*/
XMLAttributeDream::XMLAttributeDream( ) 
{
}

XMLAttributeDream::~XMLAttributeDream( )
{
}

void XMLAttributeDream::setLines( const DLString &text )
{
    DLString::size_type i1 = 0, i2 = 0;

    while (true) {
        i2 = text.find_first_of( "\n", i1 );

        if (i2 == DLString::npos) 
            i2 = text.length( );
        
        lines.push_back( XMLString( text.substr( i1, i2 - i1 ) + "{x" ) );
        i1 = i2 + 1;

        if (i1 >= text.length( ))
            break;
    }
}

void XMLAttributeDream::run( PCharacter *ch )
{
    /*
     * wake up and cleanup.
     * if woke up/fall asleep again too quickly, same dream will continue
     */
    if (ch->position != POS_SLEEPING) {
        currentDreamID = 0;
        sleepTime = 0;
        lines.clear( );
        return;
    }
    
    /*
     * just started to sleep, do nothing
     */
    time_t now = dreamland->getCurrentTime( );

    if (sleepTime == 0) {
        sleepTime = now;
        return;
    }
    
    /*
     * second sleep tick, get some random delay and find a dream
     */
    if (currentDreamID == 0) {
        if (now - sleepTime < number_range( 1, 5 ))
            return;
    
        // Unseen dreams first, oldest of them first, and each one counts as
        // read the moment it starts (the same stamp 'dream read' moves).
        // Once nothing is unseen, fall back to the rerun cursor, which walks
        // the whole archive once and then stops, as dreams always did.
        const Note *note = dreamThread->getNextUnreadNote( ch );

        if (note) {
            // A caught-up sleeper's cursor sits right before this dream: step
            // it over so tomorrow's rerun is not the same dream again. Never
            // jump it further, or a new character loses the archive reruns.
            if (dreamThread->findNextNote( ch, lastDreamID ) == note)
                lastDreamID = note->getID( );

            ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" )
                ->updateStamp( dreamThread, note );
        }
        else {
            note = dreamThread->findNextNote( ch, lastDreamID );

            if (!note)
                return;

            lastDreamID = note->getID( );
        }

        setLines( dreamThread->getNoteText( note, ch ) );
        currentDreamID = note->getID( );
        LogStream::sendNotice( ) 
            << ch->getName( ) << " sees a dream from " << note->getAuthor( ) 
            << " to " << note->getRecipient( )
            << ", id " << note->getID( ) << endl;
    }
    /*
     * already dreaming, but dream got invalidated
     */
    else if (!dreamThread->findNote( currentDreamID )) {
        lines.clear( );
        return;
    }
    
    /*
     * dream is over
     */
    if (lines.empty( )) 
        return;

    /*
     * show next dream line
     */
    ch->pecho( lines.front( ) );
    lines.pop_front( );

    if (lines.empty( ))
        LogStream::sendNotice( ) << ch->getName( ) 
            << " finishes to dream, id " << currentDreamID << endl;
}

/*-----------------------------------------------------------------------
 * DreamManager 
 *----------------------------------------------------------------------*/
DreamManager * DreamManager::thisClass = 0;
DreamManager::DreamManager( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

DreamManager::~DreamManager( )
{
    thisClass = 0;
}

void DreamManager::run( PCharacter *ch )
{
    XMLAttributeDream::Pointer attr = ch->getAttributes( ).getAttr<XMLAttributeDream>( "dream" );

    if (attr)
        attr->run( ch );
}

void DreamManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 3, Pointer( this ) );
}

