/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __GROUP_NECROMANCY__
#define __GROUP_NECROMANCY__

#include "basicmobilebehavior.h"
#include "summoncreaturespell.h"
#include "savedcreature.h"

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
