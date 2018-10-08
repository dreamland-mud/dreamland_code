/* $Id: schedulertaskroundpcharacter.h,v 1.1.4.1.6.1 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, 2003
 */
/***************************************************************************
                          schedulertask.h  -  description
                             -------------------
    begin                : Wed May 30 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef __SCHEDULERTASKROUNDPCHARACTER_H__
#define __SCHEDULERTASKROUNDPCHARACTER_H__

#pragma interface

#include "schedulertask.h"

class PCharacter;
class PCMemoryInterface;

/**
 * @short Задача для планировщика, которой на вход подаются PC online
 * @author Igor S. Petrenko
 * @see Scheduler
 * @see SchedulerTask
 */
struct SchedulerTaskRoundPCharacter : public virtual SchedulerTask
{
        typedef ::Pointer<SchedulerTaskRoundPCharacter> Pointer;
        
        virtual void run( );
        /** Обработать игрока */
        virtual void run( PCharacter* pc ) = 0;
        virtual int getPriority( ) const;
};

/**
 * @short Задача для планировщика, которой на вход подаются PC online/offline
 * @author Igor S. Petrenko
 * @see Scheduler
 * @see SchedulerTask
 */
struct SchedulerTaskRoundPCMemory : public virtual SchedulerTask
{
        typedef ::Pointer<SchedulerTaskRoundPCMemory> Pointer;
        
        virtual void run( );
        /**
         * Обработать игрока через PCMemoryInterface
         * @see PCMemoryInterface
         */
        virtual void run( PCMemoryInterface* pcm ) = 0;
        virtual int getPriority( ) const;
};

#endif
