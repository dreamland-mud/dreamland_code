/* $Id: thread.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          thread.h  -  description
                             -------------------
    begin                : Thu Jun 21 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef THREAD_H
#define THREAD_H

#ifndef __MINGW32__
#include <pthread.h>
#else
#include <windows.h>
#endif

#include "dlobject.h"

/**
 * @author Igor S. Petrenko
 * @short Реализация потоков
 */
class Thread : public virtual DLObject
{
public: 
    Thread( );
    virtual ~Thread( );
    virtual void after( )
    {
    }
    
    virtual void before( )
    {
    }
    
    void run( );
    virtual void process( ) = 0;
    
    void join( );
    void cancel( );

protected:
    void detach( );

private:
#ifndef __MINGW32__
    pthread_t thread;
#else
    DWORD threadId;
    HANDLE thread;
#endif

    static void* loop_pthread( void* voidThreadClass );
};

#endif
