#ifndef PLAYER_UTILS_H
#define PLAYER_UTILS_H

#include "xmlmultistring.h"

class PCharacter;
class PCMemoryInterface;

namespace Player {
    bool isNewbie(PCMemoryInterface *pcm);

    lang_t lang(PCMemoryInterface *pcm);
}

#endif