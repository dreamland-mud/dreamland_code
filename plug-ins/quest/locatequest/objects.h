/* $Id: objects.h,v 1.1.2.5.10.1 2007/09/29 19:34:06 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef LOCATEQUEST_OBJECTS_H
#define LOCATEQUEST_OBJECTS_H

#include "objquestbehavior.h"

class LocateItem : public MandatoryItem, public PersonalItem {
XML_OBJECT
public:
    typedef ::Pointer<LocateItem> Pointer;

protected:
    virtual void getByHero( PCharacter * );
    virtual void getByOther( Character * );
};


#endif

