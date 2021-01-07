#ifndef __WEAPONRANDOMIZER_H__
#define __WEAPONRANDOMIZER_H__

#include "eventbus.h"
#include "plugin.h"

class Object;
class ItemReadEvent;
class ItemResetEvent;
class ItemEditedEvent;

/**
 * Global event listener that reacts to 'item is reset', 'item is read from a profile'
 * events and applies weapon randomizer logic.
 */
class WeaponRandomizer : public Plugin, public EventHandler {
public:
    typedef ::Pointer<WeaponRandomizer> Pointer;

    virtual void initialization();
    virtual void destruction();
    virtual void handleEvent(const type_index &eventType, const Event &event) const;

protected:    
    void eventItemRead(const ItemReadEvent &event) const;
    void eventItemReset(const ItemResetEvent &event) const;
    void eventItemEdited(const ItemEditedEvent &event) const;

    void randomizeWeaponStats(Object *obj, int bestTierOverride = -1) const;
    void randomizeWeapon(Object *obj, int level, int bestTier) const;
    void clearWeapon(Object *obj) const;
    void adjustTimer(Object *obj) const;
    int getAlign(Object *obj) const;
    int getTier(Object *obj, int bestTierOverride) const;
};


#endif // __WEAPONRANDOMIZER_H__