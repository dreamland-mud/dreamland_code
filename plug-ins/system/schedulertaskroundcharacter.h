/* $Id: schedulertaskroundcharacter.h,v 1.1.4.1.6.1 2009/09/24 14:09:13 rufina Exp $
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
#ifndef __SCHEDULERTASKROUNDCHARACTER_H__
#define __SCHEDULERTASKROUNDCHARACTER_H__

#pragma interface

#include "schedulertask.h"

class Character;

/**
 * @short Задача для планировщика, которой на вход подаются мобы
 * @author Igor S. Petrenko
 * @see Scheduler
 * @see SchedulerTask
 */
struct SchedulerTaskRoundCharacter : public virtual SchedulerTask
{
	typedef ::Pointer<SchedulerTaskRoundCharacter> Pointer;
	
	virtual void run( );
	/** Обработать моба */
	virtual void run( Character* ch ) = 0;
	virtual int getPriority( ) const;
	    
};

#endif
