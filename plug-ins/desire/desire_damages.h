/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DESIRE_DAMAGES_H
#define DESIRE_DAMAGES_H

#include "damage_impl.h"

// obsolates damage( .. TYPE_HUNGER .. );
struct HungerDamage : public SelfDamage {
    HungerDamage( Character *ch, int dam );

    virtual void message( );
    virtual void reportState( ) { }
};

// obsolates damage( .. TYPE_THIRST.. );
struct ThirstDamage : public SelfDamage {
    ThirstDamage( Character *ch, int dam );

    virtual void message( );
    virtual void reportState( ) { }
};

#endif

