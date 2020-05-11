#include "debug_utils.h"
#include "pcharacter.h"


Debug::Debug(Character *_ch, const char *_attr, const char *_label)
    : ch(_ch), attr(_attr), label(_label)
{
}

Debug::~Debug() 
{
    DLString result = buf.str();
    if (!result.empty() 
            && !ch->is_npc() 
            && ch->getPC()->getAttributes().isAvailable(attr)) 
    {
        ch->printf("%s chances: %s\r\n", label, result.c_str());
    }
}        

Debug & Debug::log(float chance, const char *msg)
{
    buf << msg << "={C" << chance << "{x, ";
    return *this;
}



