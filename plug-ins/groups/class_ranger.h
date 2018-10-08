/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLASS_RANGER__
#define __CLASS_RANGER__

#include "objectbehavior.h"
#include "basicmobilebehavior.h"
#include "summoncreaturespell.h"
#include "savedcreature.h"

class RangerStaff : public ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<RangerStaff> Pointer;

    virtual bool death( Character * );
    virtual void fight( Character * );
    virtual bool canEquip( Character * );
};

class RangerCreature : public SummonedCreature, 
                       public SavedCreature,
                       public BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<RangerCreature> Pointer;

    virtual ~RangerCreature( );
};

#endif
