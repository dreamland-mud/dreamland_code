#include "char_body.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "wearlocation.h"
#include "affect.h"
#include "merc.h"
#include "def.h"

bool Char::hasPaws(Character *ch)
{
    return !ch->form.isSet(FORM_SENTIENT) || ch->form.isSet(FORM_DRAGON);
}

bool  Char::hasLegs(Character *ch)
{
    return IS_SET(ch->parts, PART_LEGS);
}

bool Char::hasHooves(Character *ch)
{
    return IS_SET(ch->parts, PART_FOUR_HOOVES | PART_TWO_HOOVES);
}

bool Char::hasEyes(Character *ch)
{
    return IS_SET(ch->parts, PART_EYE);
}

bool Char::lostWearloc(Character *ch, WearlocationReference &wearloc)
{
    // Still has it?
    if (ch->getWearloc().isSet(wearloc))
        return false;

    // Check if char has affect that removes this particular wearloc.
    for (auto &paf: ch->affected) {
        if (paf->global.getRegistry() == wearlocationManager)
            if (paf->global.isSet(wearloc))
                return true;
    }

    return false;
}

