/* $Id: thread.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 * weakly based on Thread by NoFate, 2001
 */

#include "thread.h"

Thread::Thread( ) : thread( 0 )
{
}

Thread::~Thread( )
{
}

void Thread::join( )
{
    pthread_join( thread, 0 );
}

void Thread::cancel( )
{
    pthread_cancel( thread );
}

void Thread::detach( )
{
    pthread_detach( thread );
}

void Thread::run( )
{
    size_t ssize;
    pthread_attr_t attr;

    pthread_attr_init( &attr );
    pthread_attr_getstacksize( &attr, &ssize );
    pthread_attr_setstacksize( &attr, ssize * 2 );

    pthread_create( &thread, &attr, &loop_pthread, this );
}

void* Thread::loop_pthread( void* voidThreadClass )
{
    Thread* threadClass = static_cast<Thread*>( voidThreadClass );
    
    threadClass->before( );
    threadClass->process( );
    threadClass->after( );

    return 0;
}
