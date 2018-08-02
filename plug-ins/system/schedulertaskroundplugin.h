/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          schedulertaskroundplugin.h  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef SCHEDULERTASKROUNDPLUGIN_H
#define SCHEDULERTASKROUNDPLUGIN_H

#include "plugin.h"
#include "schedulertask.h"


/**
 * @author Igor S. Petrenko
 */
class SchedulerTaskRoundPlugin : public virtual Plugin, public virtual SchedulerTask
{
public:
	typedef ::Pointer<SchedulerTaskRoundPlugin> Pointer;

protected:	
	virtual void initialization( );
	virtual void destruction( );
};

#endif
