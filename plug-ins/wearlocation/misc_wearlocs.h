/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MISC_WEARLOCS_H__
#define __MISC_WEARLOCS_H__

#include "defaultwearlocation.h"

class StuckInWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<StuckInWearloc> Pointer;

    virtual bool equip( Character *ch, Object *obj );
    virtual void unequip( Character *ch, Object *obj );
};

class ShieldWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<ShieldWearloc> Pointer;

    virtual int canWear( Character *ch, Object *obj, int flags );
};

class HairWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<HairWearloc> Pointer;

    virtual bool equip( Character *ch, Object *obj );
    virtual void unequip( Character *ch, Object *obj );
    virtual bool matches( Character *ch );
    virtual int canWear( Character *ch, Object *obj, int flags );
protected:    
    virtual void affectsOnEquip( Character *ch, Object *obj );
    virtual void affectsOnUnequip( Character *ch, Object *obj );
};

class WieldWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<WieldWearloc> Pointer;

    virtual bool remove( Object *, int flags );
    virtual int wear( Object *obj, int flags );
    virtual int canWear( Character *ch, Object *obj, int flags );

private:
    void reportWeaponSkill( Character *ch, Object *obj );
};

class SecondWieldWearloc : public WieldWearloc {
XML_OBJECT    
public:
    typedef ::Pointer<SecondWieldWearloc> Pointer;
    
    virtual bool remove( Object *, int flags );
    virtual bool matches( Character *ch );
    virtual int canWear( Character *ch, Object *obj, int flags );
};

class TattooWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<TattooWearloc> Pointer;

    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual bool canRemove( Character *ch, Object *obj, int flags );
};

#endif
