#ifndef __WEAPONRANDOMIZER_H__
#define __WEAPONRANDOMIZER_H__

#include "eventbus.h"
#include "plugin.h"
#include "oneallocate.h"

class Object;
class ItemReadEvent;
class ItemResetEvent;
class ItemEditedEvent;

/**
 * Global event listener that reacts to 'item is reset', 'item is read from a profile'
 * events and applies weapon randomizer logic.
 */
class WeaponRandomizer : public Plugin, public EventHandler, public OneAllocate {
public:
    typedef ::Pointer<WeaponRandomizer> Pointer;

    WeaponRandomizer();
    virtual ~WeaponRandomizer();
    virtual void initialization();
    virtual void destruction();
    virtual void handleEvent(const type_index &eventType, const Event &event) const;
    static inline WeaponRandomizer *getThis() { return thisClass; }

protected:    
    static WeaponRandomizer *thisClass;

    void eventItemRead(const ItemReadEvent &event) const;
    void eventItemReset(const ItemResetEvent &event) const;
    void eventItemEdited(const ItemEditedEvent &event) const;

    void randomizeWeaponStats(Object *obj, int bestTierOverride = -1) const;
    void randomizeWeapon(Object *obj, int bestTier) const;
    void clearWeapon(Object *obj) const;
    void adjustTimer(Object *obj) const;
    int getAlign(Object *obj) const;
    int getLevel(Object *obj) const;
    int getTier(Object *obj, int bestTierOverride) const;
};


#endif // __WEAPONRANDOMIZER_H__