/* $Id: writing.cpp,v 1.1.2.9.6.3 2007/09/11 00:27:23 rufina Exp $
 *
 * ruffina, 2004
 */
#include <string.h>

#include "writing.h"

#include "class.h"
#include "logstream.h"

#include "pcharacter.h"
#include "object.h"

#include "act.h"
#include "fight.h"
#include "merc.h"
#include "loadsave.h"
#include "string_utils.h"
#include "mudtags.h"
#include "xmlpcstringeditor.h"
#include "websocketrpc.h"
#include "arg_utils.h"
#include "def.h"
#include "l10n.h"
#include "fenia_utils.h"

#define log(x) LogStream::sendNotice() << x << endl

COMMAND(CWrite, "write")
{
    Object *target;
    DLString arguments = constArguments;
    DLString targetName = arguments.getOneArgument( );
    
    if (targetName.empty( )) {
        usage( ch );
        return;
    }
    
    if (( target = get_obj_carry( ch, targetName.c_str( ) ) ))
        writeOnPaper( ch, target, arguments );
    else if (( target = get_obj_room( ch, targetName.c_str( ) ) ))
        writeOnWall( ch, target, arguments );
    else
        ch->pecho(_("У тебя нет этого."));
}

void CWrite::writeOnWall( Character *ch, Object *wall, DLString &arguments )
{
    ExtraDescription *ed;
    DLString cmd;
    Object *nail;

    if (wall->item_type != ITEM_PARCHMENT) {
        oldact(_("Тебе не удастся ничего нацарапать на $o6."), ch, wall, 0, TO_CHAR);
        return;
    }

    if (wall->can_wear(ITEM_TAKE)) {
        oldact(_("Подними $o4 с земли, а потом пиши."), ch, wall, 0, TO_CHAR);
        return;
    }
    
    if (!( nail = findNail( ch ) )) {
        ch->pecho(_("Ты не держишь в руках ничего колющего или царапающего."));
        return;
    }

    if (arguments.empty( )) {
        oldact(_("Что именно ты хочешь выцарапать на $o6?"), ch, wall, 0, TO_CHAR);
        return;
    }

    cmd = arguments.getOneArgument( );
    cmd.stripWhiteSpace( );
    
    ed = wall->getProperDescription();

    if (cmd == "+") {
        if (!ed) 
            ed = wall->addProperDescription();

        mudtags_strip_web( arguments, ch );
        const DLString &text = wall->getExtraDescr(ed->keyword, LANG_DEFAULT);
        wall->addExtraDescr(ed->keyword, String::addLine(text, arguments), LANG_DEFAULT);

        oldact(_("Ты выцарапываешь $O5 надпись на $o6."), ch, wall, nail, TO_CHAR );
        oldact(_("$c1 выцарапывает $O5 надпись на $o6."), ch, wall, nail, TO_ROOM );

        if (wall->pIndexData->vnum == 50102)
            achievement( ch, "writing_on_the_wall" );
    }
    else if (cmd == "-" && ch->is_immortal( )) {
        if (ed) {
            const DLString &text = wall->getExtraDescr(ed->keyword, LANG_DEFAULT);
            wall->addExtraDescr(ed->keyword, String::delLine(text), LANG_DEFAULT);

            oldact(_("Ты отшкрябываешь последнюю строчку с $o2."), ch, wall, 0, TO_CHAR );
            oldact(_("$c1 скребет $o4."), ch, wall, 0, TO_ROOM );
        }
        else
            oldact(_("$o1 девственно чист(а) - удалять нечего."), ch, wall, 0, TO_CHAR );
    }
    else if (cmd == "clear" && ch->is_immortal( )) {
        wall->clearProperDescription();
        oldact(_("Ты тщательно отшкрябываешь все надписи с $o2."), ch, wall, 0, TO_CHAR );
        oldact(_("$c1 тщательно отшкрябывает все надписи с $o2."), ch, wall, 0, TO_ROOM );
    }                
    else
        usage( ch );
}

void CWrite::writeOnPaper( Character *ch, Object *paper, DLString &arguments )
{
    ExtraDescription *ed;
    DLString cmd, keyword;
    bool fFace = false;

    if (paper->item_type != ITEM_PARCHMENT) {
        ch->pecho(_("Это не пергамент."));
        return;
    }

    if (arguments.empty( )) {
        oldact(_("Что именно ты хочешь записать на $o4?"), ch, paper, 0, TO_CHAR);
        return;
    }
    
    keyword = arguments.getOneArgument( );
    keyword.stripWhiteSpace( );

    if (arg_is_strict(keyword, "face")) {
        keyword = String::toString(paper->getKeyword());
        fFace = true;
    }

    cmd = arguments.getOneArgument( );
    cmd.stripWhiteSpace( );
    
    if (cmd != "+" && cmd != "-" && cmd != "clear" && !arg_is_copy(cmd) && !arg_is_paste(cmd)) {
        usage( ch );
        return;
    }

    if (arg_is_strict(cmd, "clear")) {
        paper->extraDescriptions.findAndDestroy(keyword);
        
        if (fFace)
            oldact(_("Ты стираешь все надписи с лицевой стороны $o2."), ch, paper, 0, TO_CHAR );
        else
            oldact(_("Ты стираешь с $o2 все, что касается '$T'."), ch, paper, keyword.c_str( ), TO_CHAR );

        return;
    }
    
    if (fFace)
        ed = paper->getProperDescription();
    else
        ed = paper->extraDescriptions.findUnstrict(keyword);

    if (arg_is_copy(cmd)) {
        copyToBuffer( ch, paper, ed );
        return;
    }

    if (arg_is_paste(cmd)) {
        pasteFromBuffer( ch, paper, ed, keyword, fFace );
        return;
    }

    if (cmd == "+") {
        if (!ed) {
            if (fFace)
                ed = paper->addProperDescription();
            else
                ed = paper->addExtraDescr(keyword, DLString::emptyString, LANG_DEFAULT);
        }

        mudtags_strip_web( arguments, ch );
        const DLString &text = paper->getExtraDescr(ed->keyword, LANG_DEFAULT);
        paper->addExtraDescr(ed->keyword, String::addLine(text, arguments), LANG_DEFAULT);
        
        if (fFace)
            oldact(_("Ты делаешь запись на $o6."), ch, paper, 0, TO_CHAR );
        else
            oldact(_("Ты делаешь запись на $o6 в раздел '$T'"), ch, paper, keyword.c_str( ), TO_CHAR );
    }
    else if (cmd == "-") {
        if (!ed) {
            oldact(_("Удалять с $o2 больше нечего."), ch, paper, 0, TO_CHAR );

        } else {
            const DLString &text = paper->getExtraDescr(ed->keyword, LANG_DEFAULT);
            paper->addExtraDescr(ed->keyword, String::delLine(text), LANG_DEFAULT);

            if (fFace)
                oldact(_("Ты удаляешь последнюю строку с $o2."), ch, paper, 0, TO_CHAR );
            else
                oldact(_("Ты удаляешь последнюю строку с $o2 в разделе '$T'."), ch, paper, keyword.c_str( ), TO_CHAR );
        } 
    }
}

void CWrite::copyToBuffer( Character *ch, Object *paper, ExtraDescription *ed )
{
    if (!ch->getPC( ))
        return;

    if (!ed) {
        oldact(_("Копировать с $o2 нечего."), ch, paper, 0, TO_CHAR );
        return;
    }

    ch->getPC( )->getAttributes( ).getAttr<XMLAttributeEditorState>("edstate")
        ->regs[0].split(paper->getExtraDescr(ed->keyword, LANG_DEFAULT));

    if (is_websock(ch))
        ch->pecho(_("Запись скопирована в буфер редактора, используй команду вебредактор для редактирования."));
    else
        ch->pecho(_("Запись скопирована в буфер редактора, однако пользоваться редактором можно только изнутри веб-клиента."));
}

void CWrite::pasteFromBuffer( Character *ch, Object *paper, ExtraDescription *ed, const DLString &keyword, bool fFace )
{
    if (!ch->getPC( ))
        return;

    DLString text = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeEditorState>("edstate")->regs[0].dump( );

    if (text.empty( )) {
        ch->pecho(_("Буфер редактора пуст!"));
        return;
    }

    if (text.size( ) >= MAX_STRING_LENGTH) {
        ch->pecho(_("Слишком длинный текст."));
        return;
    }

    if (!ed) {
        if (fFace)
            ed = paper->addProperDescription();
        else
            ed = paper->addExtraDescr(keyword, DLString::emptyString, LANG_DEFAULT);
    }

    mudtags_strip_web( text, ch );
    paper->addExtraDescr(ed->keyword, text, LANG_DEFAULT);

    if (fFace)
        oldact(_("Ты переписываешь лицевую сторону $o2 из буфера редактора."), ch, paper, 0, TO_CHAR );
    else
        oldact(_("Ты переписываешь раздел '$T' на $o6 из буфера редактора."), ch, paper, keyword.c_str( ), TO_CHAR );
}

Object * CWrite::findNail( Character *ch )
{
    Object *nail;
    
    nail = get_eq_char( ch, wear_wield );

    if (!nail)
        nail = get_eq_char( ch, wear_hold );
    if (!nail)
        nail = get_eq_char( ch, wear_second_wield );
    if (!nail)
        return NULL;

    DLString objname = String::toString(nail->getKeyword());
    // TODO move to synonms
    if (is_name( "гвоздь", objname.c_str() ) || is_name( "nail", objname.c_str() ))
        return nail;
        
    if (nail->item_type == ITEM_WEAPON)
        switch (attack_table[nail->value3()].damage) {
        case DAM_PIERCE:
        case DAM_SLASH:
            return nail;
        }

    return NULL;
}

void CWrite::usage( Character *ch )
{
    ch->pecho(_("Подробнее см. 'help write'."));
}


