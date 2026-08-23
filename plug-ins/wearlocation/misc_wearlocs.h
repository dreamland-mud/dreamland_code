/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MISC_WEARLOCS_H__
#define __MISC_WEARLOCS_H__

#include "defaultwearlocation.h"
#include "xmlmap.h"
#include "xmlattribute.h"

class StuckInWearloc : public DefaultWearlocation {
XML_OBJECT
public:
    typedef ::Pointer<StuckInWearloc> Pointer;

    virtual bool equip( Character *ch, Object *obj );
    virtual void unequip( Character *ch, Object *obj );
    virtual bool remove( Object *obj, int flags );
    virtual bool canRemove( Character *ch, Object *obj, int flags );
    virtual bool givesAffects() const { return false; }

protected:
    virtual const DLString &getMsgSelfRemove(Object *obj) const;
    virtual const DLString &getMsgRoomRemove(Object *obj) const;
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

    // Trilingual (sheath.xml carries l="en"/"ru"/"ua"): a plain XMLString would
    // collapse the variants to one loaded value and show it to every viewer.
    // msgDisplay and msgSelfWear are read straight (getForLang), so they must be
    // XMLMultiString. The three room/remove lines flow through the base's _()
    // wrapper (DefaultWearlocation::wearAtomic/remove), which resolves them per
    // viewer from the catalog, so they stay plain strings (the RU catalog key).
    XML_VARIABLE XMLMultiString msgDisplay;
    XML_VARIABLE XMLString msgRoomWear;
    XML_VARIABLE XMLMultiString msgSelfWear;
    XML_VARIABLE XMLString msgRoomRemove;
    XML_VARIABLE XMLString msgSelfRemove;
};

class SheathWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<SheathWearloc> Pointer;

    virtual bool matches( Character *ch );
    virtual bool displayFlags(Character *ch, Object *obj);
    virtual DLString displayName(Character *ch, Object *obj, lang_t lang);
    virtual DLString displayLocation(Character *ch, Object *obj, lang_t lang);

    virtual void onFight(Character *ch, Object *obj);

protected:
    virtual const DLString &getMsgSelfWear(Character *ch, Object *obj) const;
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

/**
 * Per-player state for the purchasable 'personal' wearlocation: the display
 * label the player chose for their slot. Ownership of the slot itself lives in
 * Character::wearloc (the same bitvector the ear piercing purchase sets), so
 * this attribute only exists once a player names the slot.
 *
 * The label is stored sanitized (see slot_label_sanitize in
 * wearloc_commands.cpp): no color codes, no control characters, capped length.
 * It is rendered verbatim inside other players' equipment lists, so nothing
 * unsanitized may ever be written here.
 */
class XMLAttributeWearslot : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeWearslot> Pointer;

    /** Key under which this attribute is stored on a PCharacter. */
    static const DLString ATTR_NAME;

    const DLString & getLabel( ) const;
    void setLabel( const DLString & );

protected:
    XML_VARIABLE XMLString label;
};

/**
 * The purchasable 'personal' flavor slot: one item of any type, worn purely
 * for show. No stats (givesAffects false, affects never applied), no item-type
 * restriction. Ownership is a Character::wearloc bit sold by the quest trader
 * (WearslotQuestArticle), so the default matches(Character) is exactly the
 * gate: needRib && wearloc.isSet(this). The slot label in the equipment list
 * is player-customizable (XMLAttributeWearslot), hence displayLocation.
 */
class PersonalWearloc : public DefaultWearlocation {
XML_OBJECT
public:
    typedef ::Pointer<PersonalWearloc> Pointer;

    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual DLString displayLocation(Character *ch, Object *obj, lang_t lang);
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
