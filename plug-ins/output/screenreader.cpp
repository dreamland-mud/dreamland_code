#include "screenreader.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

bool uses_screenreader(Descriptor *d)
{
    return d && d->character && uses_screenreader(d->character);
}

bool uses_screenreader(Character *ch)
{
    if (!ch)
        return false;
        
    if (ch->desc && ch->desc->telnet.ttype == TTYPE_LYNTIN)
        return true;

    if (!ch->is_npc() && IS_SET(ch->getPC()->config, CONFIG_SCREENREADER))
        return true;

    return false;
}
