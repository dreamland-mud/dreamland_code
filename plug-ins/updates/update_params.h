/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __UPDATE_PARAMS_H__
#define __UPDATE_PARAMS_H__

#include "schedulertaskroundcharacter.h"
#include "schedulertaskroundplugin.h"

/**
  * Регенерация хитпоинтов
  * Регенерация маны
  * Регенерация мувесов
  *@author Sergey L. Tereschenko
  */

class CharacterParamsUpdateTask : public SchedulerTaskRoundCharacter,
			          public SchedulerTaskRoundPlugin
{
public:
	typedef ::Pointer<CharacterParamsUpdateTask> Pointer;

	virtual void run( Character * );
	virtual void before( );
	virtual void after( );

protected:
	void gainHitPoint( Character * );
	void gainMana( Character * );
	void gainMove( Character * );
};


#endif
