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

     // Return player religion name, "gods" for NPC or previosly saved random deity's name.
     DLString godName(Character *ch);
}

#endif