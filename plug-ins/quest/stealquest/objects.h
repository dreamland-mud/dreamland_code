/* $Id: objects.h,v 1.1.2.3.22.1 2007/09/29 19:34:09 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef STEALQUEST_OBJECTS_H
#define STEALQUEST_OBJECTS_H

#include "objquestbehavior.h"

class HiddenChest : public ObjQuestBehavior {
XML_OBJECT
public:
    typedef ::Pointer<HiddenChest> Pointer;

    virtual bool canLock( Character * );
};

class LockPick : public PersonalItem {
XML_OBJECT
public:
    typedef ::Pointer<LockPick> Pointer;

protected:
    virtual bool ourMobile( NPCharacter * );
    virtual void getByHero( PCharacter * );
    virtual void getByOther( Character * );
};

class RobbedItem : public PersonalItem, public MandatoryItem {
XML_OBJECT
public:
    typedef ::Pointer<RobbedItem> Pointer;

protected:
    virtual void getByHero( PCharacter * );
    virtual void getByOther( Character * );
};

#endif

