/* $Id: schedulertaskroundroom.h,v 1.1.4.1.6.1 2009/09/24 14:09:13 rufina Exp $
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
#ifndef __SCHEDULERTASKROUNDROOM_H__
#define __SCHEDULERTASKROUNDROOM_H__

#pragma interface

#include "schedulertask.h"

class Room;

/**
 * @short Задача для планировщика, которой на вход подаются комнаты
 * @author Igor S. Petrenko
 * @see Scheduler
 * @see SchedulerTask
 */
struct SchedulerTaskRoundRoom : public virtual SchedulerTask
{
	typedef ::Pointer<SchedulerTaskRoundRoom> Pointer;
	
	virtual void run( );
	/** Обработать комнату */
	virtual void run( Room* room ) = 0;
	virtual int getPriority( ) const;
};

#endif
