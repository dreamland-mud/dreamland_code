#include "itemutils.h"
#include "core/object.h"
#include "room.h"
#include "character.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

bitstring_t Item::furnitureFlags(Object *obj)
{
    switch (obj->item_type) {
    default:
        return 0;

    case ITEM_FURNITURE:
        return obj->value2();
    
    case ITEM_FOUNTAIN:
        return obj->value4();
    }
}    
 
const char *Item::FURNITURE_PROPS = "furniture";

const char *Item::furnitureMsgTags[] = {
    "msgSitChar", "msgSitRoom",
    "msgSitRestChar", "msgSitRestRoom",
    "msgRestChar", "msgRestRoom",
    "msgSleepChar", "msgSleepRoom",
    "msgStandChar", "msgStandRoom",
    "msgWakeRestChar", "msgWakeRestRoom",
    "msgWakeSitChar", "msgWakeSitRoom",
    "msgWakeStandChar", "msgWakeStandRoom",
    0
};

DLString Item::furnitureMsgProp(const obj_index_data *pObj, const char *tag)
{
    const Json::Value &props = pObj->props;

    if (!props.isObject() || !props.isMember(FURNITURE_PROPS))
        return DLString::emptyString;

    const Json::Value &msgs = props[FURNITURE_PROPS];
    if (!msgs.isObject() || !msgs.isMember(tag) || !msgs[tag].isString())
        return DLString::emptyString;

    DLString msg = msgs[tag].asString(), error;
    if (!furnitureMsgValid(msg, error))
        return DLString::emptyString;

    return msg;
}

// Mirrors the MsgFormatter::run grammar (plug-ins/output/msgformatter.cpp),
// restricted to what is safe with exactly two noun-holder varargs.
bool Item::furnitureMsgValid(const DLString &msg, DLString &error)
{
    const char *s = msg.c_str();

    for (size_t i = 0; s[i]; i++) {
        if (s[i] != '%')
            continue;

        if (s[i + 1] == '%') {
            i++;
            continue;
        }

        if ((s[i + 1] != '1' && s[i + 1] != '2') || s[i + 2] != '$') {
            error = "после % допустимы только %1$ (предмет), %2$ (персонаж) или %%";
            return false;
        }

        size_t j = i + 3;
        if (s[j] == '#')
            j++;
        while (s[j] == '^' || s[j] == '_')
            j++;

        char cvt = s[j];
        switch (cvt) {
        case 'C': case 'O': case 'P': case 'T': {
            char maxCase = (cvt == 'T' ? '8' : '7');
            if (s[j + 1] < '1' || s[j + 1] > maxCase) {
                error = DLString("после %N$") + cvt + " нужна цифра падежа";
                return false;
            }
            i = j + 1;
            break;
        }
        case 'G': case 'n':
            i = j;
            break;
        default:
            error = "допустимые подстановки: C, O, P, T с падежом, G и n с формами";
            return false;
        }
    }

    return true;
}

bitstring_t Item::furnitureMaxPeople(Object *obj)
{
    switch (obj->item_type) {
    default:
        return 0;

    case ITEM_FURNITURE:
        return obj->value0();
    
    case ITEM_FOUNTAIN:
        return obj->value3();
    }
}

/* 
 * returns number of people on an object 
 */
int Item::countUsers(Object *obj)
{
    Character *fch;
    int count = 0;

    if (obj->in_room == 0)
        return 0;

    for (fch = obj->in_room->people; fch != 0; fch = fch->next_in_room)
        if (fch->on == obj)
            count++;

    return count;
}

bool Item::canDrop( Character *ch, Object *obj, bool verbose )
{
    if (!IS_SET(obj->extra_flags, ITEM_NODROP))
        return true;

    if (!ch->is_npc() && ch->getRealLevel( ) >= LEVEL_IMMORTAL)
        return true;

    if (verbose)
        ch->pecho(_("Ты не можешь избавиться от этого."));

    return false;
}

