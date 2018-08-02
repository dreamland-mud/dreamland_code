/* $Id: staffbehavior.h,v 1.1.2.5.22.1 2007/09/29 19:34:07 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef STAFFBEHAVIOR_H
#define STAFFBEHAVIOR_H

#include "objquestbehavior.h"

class StaffBehavior : public MandatoryItem, public PersonalItem {
XML_OBJECT
public:
    typedef ::Pointer<StaffBehavior> Pointer;

protected:
    virtual void getByHero( PCharacter * );
    virtual void getByOther( Character * );
};

#endif

