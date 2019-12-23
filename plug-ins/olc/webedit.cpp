/* $Id$
 *
 * ruffina, 2004
 */

#include "commandtemplate.h"
#include "commonattributes.h"
#include "xmlpcstringeditor.h"

#include "descriptorstatemanager.h"
#include "websocketrpc.h"
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
    
    if (!is_websock(ch)) {
        ch->println("Этой командой можно пользоваться только изнутри веб-клиента.");
        return;
    }

    std::vector<DLString> args(2);

    // Pass editor buffer as the first argument.
    Editor::reg_t &reg = pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0];
    for(Editor::reg_t::const_iterator j = reg.begin(); j != reg.end(); j++)
        args[0].append(*j).append("\n");

    // Pass arg this comand was called with as the second argument, e.g. 'webedit help'.
    args[1] = argument;
    ch->desc->writeWSCommand("editor_open", args);
}

