#ifndef CRAFT_UTILS_H
#define CRAFT_UTILS_H

#include "pointer.h"

class XMLAttributeCraft;
class PCharacter;
class Character;

::Pointer<XMLAttributeCraft> craft_attr(PCharacter *ch);
::Pointer<XMLAttributeCraft> craft_attr(Character *ch);

#endif
