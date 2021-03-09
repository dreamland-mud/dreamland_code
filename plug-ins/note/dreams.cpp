/* $Id$
 *
 * ruffina, 2004
 */
#include "dreams.h"
#include "logstream.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

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

void DreamThread::getUnreadMessage( int count, ostringstream &buf ) const
{
    // Example: Смертным приснили {W10{x новых снов ('{y{lRсон{lEdream{x').
    buf << fmt( 0, "Смертным приснили {W%1$d{x нов%1$Iый|ых|ых с%1$Iон|на|нов ('{y{lRсон{lEdream{x').", count ) << endl;
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
     * wake up, cleanup and remember last dream.
     * if woke up/fall asleep again too quickly, same dream will continue
     */
    if (ch->position != POS_SLEEPING) {
        if (currentDreamID) {
            lastDreamID = currentDreamID;
            currentDreamID = 0;
        }

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
    
        const Note *note = dreamThread->findNextNote( ch, lastDreamID );

        if (!note)
            return;

        setLines( note->getText( ) );
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

