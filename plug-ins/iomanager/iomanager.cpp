/* $Id: iomanager.cpp,v 1.1.4.4.6.3 2009/09/24 14:09:12 rufina Exp $
 *
 * ruffina, 2005
 */

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "so.h"
#include "lastlogstream.h"
#include "mocregistrator.h"

#include "iomanager.h"
#include "comm.h"
#include "serversocketcontainer.h"

#include "dlscheduler.h"
#include "pcharacter.h"
#include "descriptor.h"


void
IOTask::after()
{
    DLScheduler::getThis( )->putTaskInitiate( Pointer(this) );
}

int IOInitTask::getPriority( ) const
{
    return SCDP_IOINIT;
}

void
IOInitTask::run()
{
    IOManager::getThis( )->ioInit( );
}


int IOPollTask::getPriority( ) const
{
    return SCDP_IOPOLL;
}

void
IOPollTask::run()
{
    IOManager::getThis( )->ioPoll( );
}


int IOReadTask::getPriority( ) const
{
    return SCDP_IOREAD;
}

void
IOReadTask::run()
{
    IOManager::getThis( )->ioRead( );
}

int IOWriteTask::getPriority( ) const
{
    return SCDP_IOWRITE;
}

void
IOWriteTask::run()
{
    IOManager::getThis( )->ioWrite( );
}

int IOFinalyTask::getPriority( ) const
{
    return SCDP_FINAL;
}

void
IOFinalyTask::run()
{
    IOManager::getThis( )->ioFinaly( );
}


IOManager *IOManager::thisClass = 0;

IOManager::IOManager()
{
    thisClass = this;
}
IOManager::~IOManager()
{
    thisClass = 0;
}

void
IOManager::initialization()
{
    DLScheduler::getThis( )->putTaskInitiate( IOInitTask::Pointer( NEW ) );
    DLScheduler::getThis( )->putTaskInitiate( IOPollTask::Pointer( NEW ) );
    DLScheduler::getThis( )->putTaskInitiate( IOReadTask::Pointer( NEW ) );
    DLScheduler::getThis( )->putTaskInitiate( IOWriteTask::Pointer( NEW ) );
    DLScheduler::getThis( )->putTaskInitiate( IOFinalyTask::Pointer( NEW ) );
}

void
IOManager::destruction()
{
    DLScheduler::getThis( )->slay( IOInitTask::Pointer( NEW ) );
    DLScheduler::getThis( )->slay( IOPollTask::Pointer( NEW ) );
    DLScheduler::getThis( )->slay( IOReadTask::Pointer( NEW ) );
    DLScheduler::getThis( )->slay( IOWriteTask::Pointer( NEW ) );
    DLScheduler::getThis( )->slay( IOFinalyTask::Pointer( NEW ) );
}

void
IOManager::ioInit()
{
    Descriptor *d;

    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );

    ServerSocketContainer::FD_SETBeforeSelect( &in_set );

    for ( d = descriptor_list; d; d = d->next ) {
        /* always process pending output */
        FD_SET( d->descriptor, &out_set );
        
        /* ignore closed descriptors */
        if (d->connected == CON_CLOSED)
            continue;

        FD_SET( d->descriptor, &in_set  );
        FD_SET( d->descriptor, &exc_set );
    }
}

void
IOManager::ioPoll()
{
    static struct timeval null_time = { 0, 0 };

    if ( select( FD_SETSIZE, &in_set, &out_set, &exc_set, &null_time ) < 0 )
        throw Exception(DLString("Game_loop: select: poll::") + strerror( errno ));
}

void
IOManager::kick(Descriptor *d)
{
    FD_CLR( d->descriptor, &exc_set );
    FD_CLR( d->descriptor, &in_set  );
    FD_CLR( d->descriptor, &out_set );

    d->outtop = 0;
    d->close( );
}

void
IOManager::ioRead()
{
    Descriptor *d;
    
    // New connection?
    ServerSocketContainer::checkNewConnaection( &in_set );

    // Kick out the freaky folks.
    for ( d = descriptor_list; d != 0; d = d->next )
        if ( FD_ISSET( d->descriptor, &exc_set ) )
            kick(d);

    /*
     * Process input.
     */
    for( d = descriptor_list; d; d = d->next ) {
        if (d->connected == CON_CLOSED)
            continue;

        d->fcommand = false;
        Character *ch = d->character;

        if (FD_ISSET( d->descriptor, &in_set )) {
            LastLogStream::send( ) <<  "Input data"  << endl;

            if (ch)
                ch->timer = 0;

            if (!d->readInput()) {
                kick(d);
                continue;
            }
        }
        
        if (ch) {
            if (ch->daze > 0)
                --ch->daze;

            if (ch->wait > 0) {
                --d->character->wait;
                /* waitstate - no command parsing! */
                continue;
            }

            if (!ch->is_npc( )
                && ch->getPC( )->getAttributes( ).isAvailable( "speedwalk" ))
                continue;
        }

        if (!d->buffer_handler)
            continue;

        if (!d->buffer_handler->read(d)) 
            continue;
        
        if (*d->incomm) {
            char *command2;

            d->fcommand        = true;
            command2 = get_multi_command( d, d->incomm );

            if (!d->handle_input.empty( ) && d->handle_input.front( ))
                d->handle_input.front( )->handle( d, command2 );
        }
    }
}

void
IOManager::ioWrite()
{
    Descriptor *d;

    for ( d = descriptor_list; d != 0; d = d->next ) {
        if (!FD_ISSET(d->descriptor, &out_set))
            continue;

        if (d->fcommand || d->outtop > 0)
            if (!process_output( d, true ))
                kick(d);
    }
}

void
IOManager::ioFinaly()
{
    Descriptor *d, *d_next;

    for ( d = descriptor_list; d != 0; d = d_next ) {
        d_next = d->next;

        if (d->connected != CON_CLOSED)
            continue;

        d->slay( );
    }
}

