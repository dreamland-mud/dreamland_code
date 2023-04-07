#include "charutils.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "wearlocation.h"
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

bool CharUtils::lostRaceWearloc(Character *ch, WearlocationReference &wearloc)
{
    // Never had this wearloc - nothing to lose.
    if (!ch->getRace()->getWearloc().isSet(wearloc))
        return false; 

    // Still has it?
    return !ch->getWearloc().isSet(wearloc);
}

