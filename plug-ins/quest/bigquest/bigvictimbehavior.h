/* $Id: victimbehavior.h,v 1.1.2.6.22.1 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef VICTIMBEHAVIOR_H
#define VICTIMBEHAVIOR_H

#include "mobquestbehavior.h"

class BigVictimBehavior : public MandatoryMobile, public HuntedVictim {
XML_OBJECT
public:
    typedef ::Pointer<BigVictimBehavior> Pointer;

    virtual void show( Character *victim, std::basic_ostringstream<char> &buf );

protected:
    virtual void deadFromHunter( PCMemoryInterface * );
    virtual void deadFromSuicide( PCMemoryInterface * );
    virtual void deadFromOther( PCMemoryInterface *, Character * );
    virtual void deadFromGroupMember( PCMemoryInterface *, Character * );
};

#endif

