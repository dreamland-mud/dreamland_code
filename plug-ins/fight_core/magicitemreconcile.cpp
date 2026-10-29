#include "magicitemreconcile.h"
#include "logstream.h"
#include "character.h"
#include "core/object.h"
#include "itemevents.h"
#include "chargedprops.h"

#include "def.h"

MagicItemReconcile * MagicItemReconcile::thisClass = 0;

static const char *MAGIC_REV = "magic_rev";

MagicItemReconcile::MagicItemReconcile()
{
    checkDuplicate(thisClass);
    thisClass = this;
}

MagicItemReconcile::~MagicItemReconcile()
{
    thisClass = 0;
}

void MagicItemReconcile::initialization()
{
    eventBus->subscribe(typeid(ItemCreatedEvent), Pointer(this));
    eventBus->subscribe(typeid(ItemReadEvent), Pointer(this));
}

void MagicItemReconcile::destruction()
{
    eventBus->unsubscribe(typeid(ItemCreatedEvent), Pointer(this));
    eventBus->unsubscribe(typeid(ItemReadEvent), Pointer(this));
}

void MagicItemReconcile::handleEvent(const type_index &eventType, const Event &event) const
{
    if (eventType == typeid(ItemCreatedEvent))
        eventItemCreated(static_cast<const ItemCreatedEvent &>(event));
    else if (eventType == typeid(ItemReadEvent))
        eventItemRead(static_cast<const ItemReadEvent &>(event));
}

// OLC writes props as strings, so the revision is read leniently.
static int proto_rev(Object *obj)
{
    return obj->pIndexData ? charged_int(obj->pIndexData->props, MAGIC_REV, 0) : 0;
}

// Without the stamp a fresh item would count as stale on its next load
// and get its charges refilled.
void MagicItemReconcile::eventItemCreated(const ItemCreatedEvent &event) const
{
    int rev = proto_rev(event.obj);
    if (rev > 0)
        event.obj->props[MAGIC_REV] = rev;
}

void MagicItemReconcile::eventItemRead(const ItemReadEvent &event) const
{
    Object *obj = event.obj;
    int rev = proto_rev(obj);
    if (rev <= 0)
        return;

    // The instance's own stamp only: getProperty() would fall back to the prototype.
    int oldRev = charged_int(obj->props, MAGIC_REV, 0);
    if (oldRev >= rev)
        return;

    OBJ_INDEX_DATA *pObj = obj->pIndexData;
    obj->item_type = pObj->item_type;
    for (int i = 0; i < 5; i++)
        obj->valueByIndex(i, pObj->value[i]);
    obj->level = pObj->level;
    obj->props.removeMember("charged");
    obj->props[MAGIC_REV] = rev;

    Character *carrier = obj->getCarrier();
    notice("magic_rev: obj %d [%lld] of %s reconciled, rev %d -> %d.",
           pObj->vnum, obj->getID(),
           carrier ? carrier->getNameC() : "nobody", oldRev, rev);
}
