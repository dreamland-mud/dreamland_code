/* $Id: group_vampiric.h,v 1.1.2.3.6.1 2007/06/26 07:15:13 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __VAMPIRE_H__
#define __VAMPIRE_H__

#include "genericskill.h"

class VampireSkill : public GenericSkill {
XML_OBJECT
public:
    typedef ::Pointer<VampireSkill> Pointer;

    virtual bool canPractice( PCharacter *ch, std::ostream & buf ) const
    {
	buf << "Ты не можешь практиковать это, попробуй пообщаться с гильдмастером." << endl;
	return false;
    }
};

#endif
