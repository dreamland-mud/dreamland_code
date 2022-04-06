/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLASS_RANGER__
#define __CLASS_RANGER__

#include "objectbehaviormanager.h"
#include "basicmobilebehavior.h"
#include "summoncreaturespell.h"

class RangerStaff : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<RangerStaff> Pointer;

    virtual bool death( Character * );
    virtual void fight( Character * );
    virtual bool canEquip( Character * );
};

class RangerCreature : public SummonedCreature, 
                       public SavedCreature {
XML_OBJECT
public:
    typedef ::Pointer<RangerCreature> Pointer;

    virtual ~RangerCreature( );
};

#endif
