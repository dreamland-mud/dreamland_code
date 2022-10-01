#include "religionutils.h"
#include "pcharacter.h"
#include "religion.h"
#include "dl_math.h"
#include "commonattributes.h"

RELIG(none);

Religion * ReligionUtils::setRandomGod(Character *ch)
{
    if (ch->is_npc())
        return 0;

    XMLStringAttribute::Pointer randomGodAttr = ch->getPC()->getAttributes().getAttr<XMLStringAttribute>("randomGod");
    randomGodAttr->clear();

    Religion *randomGod = 0;
    int cnt = 0;

    for (int r = 0; r < religionManager->size(); r++) {
        Religion *rel = religionManager->find(r);
        if (rel->available(ch) && number_range(0, cnt++) == 0)
            randomGod = rel;
    }

    if (randomGod)
        randomGodAttr->setValue(randomGod->getName());

    return randomGod;
}

Religion * ReligionUtils::getRandomGod(Character *ch)
{
    if (ch->is_npc())
        return 0;

    XMLStringAttribute::Pointer randomGodAttr = ch->getPC()->getAttributes().findAttr<XMLStringAttribute>("randomGod");

    if (randomGodAttr && !randomGodAttr->getValue().empty())
        return religionManager->find(randomGodAttr->getValue());

    return 0;
}

DLString ReligionUtils::godName(Character *ch)
{
    static const char *gods = "бог|и|ов|ам|ов|ами|ах";

    if (ch->is_npc())
        return gods;

    if (ch->getReligion() != god_none)
        return ch->getReligion()->getRussianName();

    Religion *randomGod = ReligionUtils::getRandomGod(ch);
    return randomGod ? randomGod->getRussianName() : gods;
}
