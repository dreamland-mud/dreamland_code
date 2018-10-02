#include "craft_utils.h"
#include "craftattribute.h"
#include "pcharacter.h"

::Pointer<XMLAttributeCraft> craft_attr(PCharacter *ch)
{
    return ch->getAttributes( ).getAttr<XMLAttributeCraft>("craft");
}

::Pointer<XMLAttributeCraft> craft_attr(Character *ch)
{
    if (ch->getPC())
        return craft_attr(ch->getPC());
    else
        return XMLAttributeCraft::Pointer();
}

