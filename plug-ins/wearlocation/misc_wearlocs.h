/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MISC_WEARLOCS_H__
#define __MISC_WEARLOCS_H__

#include "defaultwearlocation.h"
#include "xmlmap.h"

class StuckInWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<StuckInWearloc> Pointer;

    virtual bool equip( Character *ch, Object *obj );
    virtual void unequip( Character *ch, Object *obj );
    virtual bool givesAffects() const { return false; }
};

class ShieldWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<ShieldWearloc> Pointer;

    virtual int canWear( Character *ch, Object *obj, int flags );
};

struct SheathConfig : public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<SheathConfig> Pointer;

    XML_VARIABLE XMLString msgDisplay;
    XML_VARIABLE XMLString msgRoomWear;
    XML_VARIABLE XMLString msgSelfWear;
    XML_VARIABLE XMLString msgRoomRemove;
    XML_VARIABLE XMLString msgSelfRemove;
};

class SheathWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<SheathWearloc> Pointer;

    virtual bool matches( Character *ch );
    virtual bool displayFlags(Character *ch, Object *obj);
    virtual DLString displayName(Character *ch, Object *obj);
    virtual DLString displayLocation(Character *ch, Object *obj);
    
    virtual void onFight(Character *ch, Object *obj);

protected:
    virtual const DLString &getMsgSelfWear(Object *obj) const;
    virtual const DLString &getMsgSelfRemove(Object *obj) const;
    virtual const DLString &getMsgRoomWear(Object *obj) const;
    virtual const DLString &getMsgRoomRemove(Object *obj) const;

    const SheathConfig & getConfig(Object *obj) const;
    
    XML_VARIABLE XMLMapBase<SheathConfig> config;
};


class HorseWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<HorseWearloc> Pointer;

    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual bool canRemove( Character *ch, Object *obj, int flags );
};

class HairWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<HairWearloc> Pointer;

    virtual bool equip( Character *ch, Object *obj );
    virtual void unequip( Character *ch, Object *obj );
    virtual bool matches( Character *ch );
    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual bool givesAffects() const { return false; }
protected:    
    virtual void affectsOnEquip( Character *ch, Object *obj );
    virtual void affectsOnUnequip( Character *ch, Object *obj );
    virtual void triggersOnWear( Character *ch, Object *obj ) { }
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

class TailWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<TailWearloc> Pointer;

    virtual bool equip( Character *ch, Object *obj );
    virtual void unequip( Character *ch, Object *obj );
    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual bool givesAffects() const { return false; }
    
protected:    
    virtual void affectsOnEquip( Character *ch, Object *obj );
    virtual void affectsOnUnequip( Character *ch, Object *obj );
    virtual void triggersOnWear( Character *ch, Object *obj ) { }
};
#endif
