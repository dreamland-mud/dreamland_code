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
        ch->pecho("{C[{cDebug{C]{W %s: {C%s", label, result.c_str());
    }
}        

Debug & Debug::log(float chance, const char *msg)
{
    buf << msg << "{W={c" << chance << "{C, ";
    return *this;
}



