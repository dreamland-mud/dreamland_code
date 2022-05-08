/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DESIRE_DAMAGES_H
#define DESIRE_DAMAGES_H

#include "damage_impl.h"

struct HungerDamage : public SelfDamage {
    HungerDamage( Character *ch, int dam );

    virtual void message( );
    virtual bool canDamage( );
    virtual void reportState( ) { }
};

struct ThirstDamage : public SelfDamage {
    ThirstDamage( Character *ch, int dam );

    virtual void message( );
    virtual bool canDamage( );
    virtual void reportState( ) { }
};

#endif

