#ifndef PLAYER_UTILS_H
#define PLAYER_UTILS_H

#include "xmlmultistring.h"

class PCharacter;
class PCMemoryInterface;
class Character;

namespace Player {
    bool isNewbie(PCMemoryInterface *pcm);

    lang_t lang(Character *ch);

    DLString title(PCMemoryInterface *pcm);
}

#endif