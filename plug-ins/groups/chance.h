#ifndef CHANCE_H

class Character;

struct Chance {
    Chance(Character *ch, int effective, int maximum);
    bool roll();
    bool reroll();

    Character *ch;
    int effective;
    int maximum;    
};
#endif