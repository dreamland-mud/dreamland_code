/* $Id: questbag.h,v 1.1.2.4 2005/03/08 02:28:19 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef QUESTBAG_H
#define QUESTBAG_H

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"
#include "personalquestreward.h"

class QuestBag : public PersonalQuestReward {
XML_OBJECT
public:
	typedef ::Pointer<QuestBag> Pointer;
	
	virtual bool canLock( Character * );
};

#endif

