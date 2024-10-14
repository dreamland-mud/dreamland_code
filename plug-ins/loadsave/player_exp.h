#ifndef PLAYER_EXP_H
#define PLAYER_EXP_H

class PCharacter;
class PCMemoryInterface;
class Character;

namespace Player {
    void gainExp(PCharacter *pch, int gain);

    void advanceLevel(PCharacter *pch);

};

#endif