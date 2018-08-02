/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __GROUP_NECROMANCY__
#define __GROUP_NECROMANCY__

#include "basicmobilebehavior.h"
#include "summoncreaturespell.h"
#include "savedcreature.h"

/*
#include "wanderer.h"

class NecroCreature : public SummonedCreature, public Wanderer {
XML_OBJECT
public:
    typedef ::Pointer<NecroCreature> Pointer;

    
    virtual void entry( );
    
    bool startMoving( );
    
protected:
    virtual bool canEnter( Room *const );
    virtual bool specIdle( );

public:
    XML_VARIABLE XMLLongLong masterID;
    XML_VARIABLE XMLInteger  masterRoomVnum;
};
*/

class NecroCreature : public SummonedCreature, 
		      public SavedCreature,
                      public BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<NecroCreature> Pointer;

    virtual ~NecroCreature( );
};

class AdamantiteGolem : public NecroCreature {
XML_OBJECT
public:
    typedef ::Pointer<AdamantiteGolem> Pointer;

    virtual void fight( Character * );
};

#endif
