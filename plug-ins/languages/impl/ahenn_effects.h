/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __AHENN_EFFECTS_H__
#define __AHENN_EFFECTS_H__

#include "wordeffect.h"

class BadSpellWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<BadSpellWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};
    
class InspirationWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<InspirationWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

#endif

