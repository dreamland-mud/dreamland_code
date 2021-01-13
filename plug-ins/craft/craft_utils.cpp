#include "craft_utils.h"
#include "craftattribute.h"
#include "pcmemoryinterface.h"

::Pointer<XMLAttributeCraft> craft_attr(CharacterMemoryInterface *ch)
{
    if (ch->getPCM())
        return ch->getPCM()->getAttributes( ).getAttr<XMLAttributeCraft>("craft");
    else
        return XMLAttributeCraft::Pointer();
}

