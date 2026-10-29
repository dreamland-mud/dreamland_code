#ifndef __MAGICITEMRECONCILE_H__
#define __MAGICITEMRECONCILE_H__

#include "eventbus.h"
#include "plugin.h"
#include "oneallocate.h"

class ItemCreatedEvent;
class ItemReadEvent;

/**
 * Brings saved magic items up to date with a retuned prototype.
 * A prototype carries props.magic_rev, an instance the revision it was built from:
 * a new instance is stamped with its prototype's revision, and a saved one older
 * than its prototype gets the prototype's type, values, level and charged spell.
 */
class MagicItemReconcile : public Plugin, public EventHandler, public OneAllocate {
public:
    typedef ::Pointer<MagicItemReconcile> Pointer;

    MagicItemReconcile();
    virtual ~MagicItemReconcile();
    virtual void initialization();
    virtual void destruction();
    virtual void handleEvent(const type_index &eventType, const Event &event) const;

protected:
    static MagicItemReconcile *thisClass;

    void eventItemCreated(const ItemCreatedEvent &event) const;
    void eventItemRead(const ItemReadEvent &event) const;
};

#endif // __MAGICITEMRECONCILE_H__
