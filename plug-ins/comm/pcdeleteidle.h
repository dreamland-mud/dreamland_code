/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcdeleteidle.h  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef PCDELETEIDLE_H
#define PCDELETEIDLE_H

#include <list>

#include "dlstring.h"
#include "schedulertaskroundpcharacter.h"
#include "schedulertaskroundplugin.h"
#include "descriptorstatelistener.h"

class PCDeleteIdle : public SchedulerTaskRoundPlugin,
                     public virtual SchedulerTaskRoundPCMemory,
                     public DescriptorStateListener
{
public:
        typedef ::Pointer<PCDeleteIdle> Pointer;

        PCDeleteIdle( );
        virtual ~PCDeleteIdle( );
        
        virtual void initialization( );
        virtual void destruction( );
        virtual void after( );
        virtual void run( PCMemoryInterface* pcm );
        virtual void run( int, int, Descriptor * );

protected:
        std::list<DLString> eraseList;        
        static int getDiff( PCMemoryInterface * );
};

#endif
