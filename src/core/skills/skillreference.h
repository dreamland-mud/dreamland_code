/* $Id: skillreference.h,v 1.1.2.6.6.1 2007/09/11 00:01:43 rufina Exp $
 * 
 * ruffina, 2005
 */
#ifndef __SKILLREFERENCE_H__
#define __SKILLREFERENCE_H__

#include "globalreference.h"
#include "xmlglobalreference.h"

#include "skillmanager.h"
#include "skill.h"

#define GSN(var) static SkillReference gsn_##var( #var )

GLOBALREF_DECL(Skill)
XMLGLOBALREF_DECL(Skill)

#endif
