/* $Id: questgirth.h,v 1.1.2.4.22.1 2007/09/11 00:31:25 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef QUESTGIRTH_H
#define QUESTGIRTH_H

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"
#include "personalquestreward.h"

class Affect;

class QuestGirth : public PersonalQuestReward {
XML_OBJECT
public:
	typedef ::Pointer<QuestGirth> Pointer;
	
	virtual void wear( Character * );                  
	virtual void equip( Character * );                           

protected:
	void addAffect( Character *, Affect * );
};



#endif

