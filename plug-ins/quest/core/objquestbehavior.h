/* $Id: objquestbehavior.h,v 1.1.4.2.10.1 2007/09/29 19:33:59 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef OBJQUESTBEHAVIOR_H
#define OBJQUESTBEHAVIOR_H

#include "objectbehavior.h"
#include "questentity.h"

class ObjQuestBehavior : public ObjectBehavior, public virtual QuestEntity {
XML_OBJECT
public:
    typedef ::Pointer<ObjQuestBehavior> Pointer;
};

class MandatoryItem : public virtual ObjQuestBehavior {
public:
    virtual bool extract( bool );
};

class PersonalItem : public virtual ObjQuestBehavior {
public:
    virtual void get( Character * );

protected:
    virtual void getByHero( PCharacter * ) = 0;
    virtual void getByOther( Character * ) = 0;
};

#endif

