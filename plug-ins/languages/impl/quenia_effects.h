/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __QUENIA_EFFECTS_H__
#define __QUENIA_EFFECTS_H__

#include "wordeffect.h"

class GoodSpellWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<GoodSpellWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class AccuracyWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<AccuracyWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

#endif
