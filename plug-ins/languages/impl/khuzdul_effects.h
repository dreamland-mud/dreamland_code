/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __KHUZDUL_EFFECTS_H__
#define __KHUZDUL_EFFECTS_H__

#include "wordeffect.h"

class FireproofWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<FireproofWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class EnchantWeaponWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<EnchantWeaponWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class BerserkWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<BerserkWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};

class MendingWE : public WordEffect {
XML_OBJECT
public:
    typedef ::Pointer<MendingWE> Pointer;
    
    virtual bool run( PCharacter *, Character * ) const;
};


#endif
