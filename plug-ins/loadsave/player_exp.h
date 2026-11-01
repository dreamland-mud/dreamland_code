#ifndef PLAYER_EXP_H
#define PLAYER_EXP_H

class PCharacter;
class PCMemoryInterface;
class Character;
class Object;

namespace Player {
    void gainExp(PCharacter *pch, int gain);

    void advanceLevel(PCharacter *pch);

    /** True if this magic jar holds pch's soul: one of the jar's own keyword
     *  words (not the prototype's) equals, caselessly, pch's login or one of
     *  the per-language nominative names the spell stamps on the jar. */
    bool magicJarHoldsSoul(Object *jar, PCharacter *pch);

    /** True if any magic jar instance in the world holds pch's soul. */
    bool soulIsJarred(PCharacter *pch);

    /** PLR_NO_EXP is set only by the magic jar spell. If no jar holds pch's
     *  soul any more, clear the bit and tell the player. Returns true if
     *  PLR_NO_EXP is still set afterwards. */
    bool checkSoulJar(PCharacter *pch);
};

#endif