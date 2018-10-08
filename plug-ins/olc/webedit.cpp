/* $Id$
 *
 * ruffina, 2004
 */

#include "commandtemplate.h"
#include "commonattributes.h"
#include "xmlpcstringeditor.h"

#include "descriptorstatemanager.h"
#include "pcharacter.h"
#include "merc.h"
#include "descriptor.h"
#include "mercdb.h"
#include "def.h"

CMDRUNP( webedit )
{
    PCharacter *pch = ch->getPC();
    
    if(!pch || !ch->desc) {
        ch->println("Нет дескриптора, нет и редактора.");
        return;
    }
    
    if (ch->desc->websock.state != WS_ESTABLISHED) {
        ch->println("Этой командой можно пользоваться только изнутри веб-клиента.");
        return;
    }

    Editor::reg_t &reg = pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0];

    std::vector<DLString> args(1);

    for(Editor::reg_t::const_iterator j = reg.begin(); j != reg.end(); j++)
        args[0].append(*j).append("\n");

    ch->desc->writeWSCommand("editor_open", args);
}

