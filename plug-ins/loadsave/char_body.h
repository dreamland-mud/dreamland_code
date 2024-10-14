#ifndef CHAR_BODY_H
#define CHAR_BODY_H

class Character;
class NPCharacter;
class WearlocationReference;

namespace Char {
    /** Check if char hands are to be called 'paws'. Available in Fenia as .tmp.mob.hasPaws(). */
    bool hasPaws(Character *ch);

    /** Check if char has legs (lower limbs) and can be knocked over. Available in Fenia as .tmp.mob.hasLegs(). */
    bool hasLegs(Character *ch);

    /** Check if char has hooves rather than legs/hind paws.*/
    bool hasHooves(Character *ch);

    /** Check if char has eyes and can be blinded. */
    bool hasEyes(Character *ch);

    /** True if char used to have e.g. left arm but it got chopped off. */
    bool lostWearloc(Character *ch, WearlocationReference &wearloc);
}

#endif
