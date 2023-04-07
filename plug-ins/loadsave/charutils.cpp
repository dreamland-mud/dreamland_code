#include "charutils.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "wearlocation.h"
#include "affect.h"
#include "merc.h"
#include "def.h"

bool CharUtils::hasPaws(Character *ch)
{
    return !ch->form.isSet(FORM_SENTIENT) || ch->form.isSet(FORM_DRAGON);
}

bool  CharUtils::hasLegs(Character *ch)
{
    return IS_SET(ch->parts, PART_LEGS);
}

bool CharUtils::hasHooves(Character *ch)
{
    return IS_SET(ch->parts, PART_FOUR_HOOVES | PART_TWO_HOOVES);
}

bool CharUtils::hasEyes(Character *ch)
{
    return IS_SET(ch->parts, PART_EYE);
}

bool CharUtils::lostWearloc(Character *ch, WearlocationReference &wearloc)
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

