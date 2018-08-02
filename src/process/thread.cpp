/* $Id: thread.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 * weakly based on Thread by NoFate, 2001
 */

#include "thread.h"

Thread::Thread( ) : thread( 0 )
{
#ifndef __MINGW32__

#else
    thread = CreateThread( 
	NULL,              // default security attributes
	0,                 // use default stack size  
	(LPTHREAD_START_ROUTINE)loop_pthread,      // thread function 
	this,              // argument to thread function 
	CREATE_SUSPENDED,  // use default creation flags 
	&threadId);	   // returns the thread identifier 
#endif
}

Thread::~Thread( )
{
#ifndef __MINGW32__

#else
    CloseHandle(thread);
#endif
}

void Thread::join( )
{
#ifndef __MINGW32__
    pthread_join( thread, 0 );
#else
#warning no Thread::join
#endif
}

void Thread::cancel( )
{
#ifndef __MINGW32__
    pthread_cancel( thread );
#else
    TerminateThread(thread, 1);
#endif
}

void Thread::detach( )
{
#ifndef __MINGW32__
    pthread_detach( thread );
#else
#warning no Thread::detach
#endif
}

void Thread::run( )
{
#ifndef __MINGW32__
    size_t ssize;
    pthread_attr_t attr;

    pthread_attr_init( &attr );
    pthread_attr_getstacksize( &attr, &ssize );
    pthread_attr_setstacksize( &attr, ssize * 2 );

    pthread_create( &thread, &attr, &loop_pthread, this );
#else
    ResumeThread(thread);
#endif
}

void* Thread::loop_pthread( void* voidThreadClass )
{
    Thread* threadClass = static_cast<Thread*>( voidThreadClass );
    
    threadClass->before( );
    threadClass->process( );
    threadClass->after( );

    return 0;
}
