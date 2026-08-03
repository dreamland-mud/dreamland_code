#ifndef RELIGIONUTILS_H
#define RELIGIONUTILS_H

class Religion;
class Character;
class DLString;

namespace ReligionUtils {
    // Choose a random deity to fulfil the prayer and remember their name.
     Religion * setRandomGod(Character *ch);

     // Return previously saved random deity.
     Religion * getRandomGod(Character *ch);

     // The deity this character prays to: their religion, a mob prototype's sole
     // religion, or a previously saved random deity. Null means "no one in
     // particular", which callers render as the generic plural "gods".
     Religion * godReligion(Character *ch);

     // Return player religion name, "gods" for NPC or previosly saved random deity's name.
     // Russian only -- use godReligion() when the reader's language matters.
     DLString godName(Character *ch);
}

#endif