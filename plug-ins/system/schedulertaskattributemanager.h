/* $Id: schedulertaskattributemanager.h,v 1.3.2.5 2005/04/27 18:46:17 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
                          schedulertaskattributemanager.h  -  description
                             -------------------
    begin                : Mon Oct 22 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef SCHEDULERTASKATTRIBUTEMANAGER_H
#define SCHEDULERTASKATTRIBUTEMANAGER_H

#include "schedulertaskroundpcharacter.h"
#include "schedulertaskroundplugin.h"
#include "allocateclass.h"

/**
 * @author Igor S. Petrenko
 */
class SchedulerTaskAttributeManager :
	public virtual SchedulerTaskRoundPlugin,
	public virtual SchedulerTaskRoundPCharacter,
	public AllocateClass
{
public:
	typedef ::Pointer<SchedulerTaskAttributeManager> Pointer;

public:
	SchedulerTaskAttributeManager( );
	virtual ~SchedulerTaskAttributeManager( );
	
	inline virtual const DLString& getName( ) const
	{
	    return PLUGIN_NAME;
	}
	
	virtual void run( PCharacter*  );
	virtual void after( );
	
	
	static inline SchedulerTaskAttributeManager* getThis( )
	{
	    return thisClass;
	}
	
	virtual DLObject::Pointer set( DLObject::Pointer arg1, DLObject::Pointer arg2 );
private:
	static SchedulerTaskAttributeManager* thisClass;

	static const DLString PLUGIN_NAME;
};


class ScheduledPCMemoryAttributeManager :
	public virtual SchedulerTaskRoundPlugin,
	public virtual SchedulerTaskRoundPCMemory,
	public AllocateClass
{
public:
	typedef ::Pointer<ScheduledPCMemoryAttributeManager> Pointer;

	ScheduledPCMemoryAttributeManager( );
	virtual ~ScheduledPCMemoryAttributeManager( );
	
	virtual void run( PCMemoryInterface * );
	virtual void after( );
	
	inline virtual const DLString& getName( ) const
	{
	    return PLUGIN_NAME;
	}
	
	static inline ScheduledPCMemoryAttributeManager* getThis( )
	{
	    return thisClass;
	}
	
	virtual DLObject::Pointer set( DLObject::Pointer arg1, DLObject::Pointer arg2 );
private:
	static ScheduledPCMemoryAttributeManager* thisClass;

	static const DLString PLUGIN_NAME;
};

#endif
