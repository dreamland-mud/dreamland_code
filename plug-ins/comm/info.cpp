/* $Id$
 *
 * ruffina, 2004
 */
#include "commonattributes.h"
#include "commandtemplate.h"

#include "stringlist.h"
#include "pcharacter.h"

#include "merc.h"
#include "descriptor.h"
#include "xmlpcstringeditor.h"
#include "websocketrpc.h"
#include "interp.h"
#include "loadsave.h"
#include "vnum.h"
#include "arg_utils.h"

#include "def.h"

static list<DLString> desc_to_list(const char *text)
{
    char buf[MAX_STRING_LENGTH];
    list<DLString> result;

    if (!text || !text[0])
        return result;

    istringstream is(text);
    while (is.getline(buf, sizeof(buf)))
        result.push_back(buf);

    return result;
}

static DLString desc_from_list(list<DLString> &lines)
{
    ostringstream buf;

    for (auto &l: lines)
        buf << l << endl;

    return buf.str();
}

static void desc_show( Character *ch )
{
        if (ch->desc) {
            ch->pecho("Твое описание:");
            ch->desc->send(ch->getDescription( ) ? ch->getDescription( ) : "(Отсутствует).\n\r");
        }
}

static void desc_usage( Character *ch )
{
    ostringstream buf;

    buf << "Формат: " << endl
        << "    {Wописание показать{x: показать описание в 'сыром' виде с тегами цветов" << endl
        << "    {Wописание +{x: добавить строку к описанию" << endl
        << "    {Wописание -{x: удалить последнюю строку" << endl
        << "    {Wописание копировать{x: скопировать описание в буфер редактора (только в веб-клиенте)" << endl
        << "    {Wописание вставить{x: заменить описание на содержимого буфера редактора (только в веб-клиенте)" << endl;

    ch->send_to( buf );
}

CMDRUNP( description )
{
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (arg_is_show(arg)) {
        desc_show( ch );
        return;
    }

    if (arg_is_copy(arg)) {
        if (!ch->getPC( )) 
            return;

        if (!ch->getDescription( ) || !ch->getDescription( )[0]) {
                ch->pecho("Твое описание пусто, копировать в буфер нечего.");
                return;
        }        

        ch->getPC( )->getAttributes().getAttr<XMLAttributeEditorState>("edstate") 
            ->regs[0].split(ch->getDescription( )); 

        if (!is_websock(ch)) {
                ch->pecho("Описание скопировано в буфер редактора, однако пользоваться редактором можно только изнутри веб-клиента.");
        } else {
                ch->pecho("Описание скопировано в буфер редактора, используй команду вебредактор{x для редактирования.");
        }
        return;
    }

    if (arg_is(arg, "paste")) {
        if (!ch->getPC( )) 
            return;

        DLString str = ch->getPC( )->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].dump( );
        if (str.empty( )) {
            ch->pecho( "Буфер редактора пуст!" );
            return;
        }
        if (str.size( ) >= MAX_STRING_LENGTH) {
            ch->pecho("Слишком длиное описание.");
            return;
        }

        ch->setDescription( str.c_str( ));
        ch->pecho( "Новое описание вставлено из буфера редактора." );
        desc_show( ch );
        interpret_raw(ch, "confirm", "review");
        return;
    }

    if (argument[0] == '\0') {
        desc_show( ch );
        ch->pecho("\r\nПодробности читай в {W? описание{x.");
        return;
    }

    if (argument[0] == '-')
    {
        if (!ch->getDescription() || !ch->getDescription()[0])
        {
            ch->pecho("Нет ничего для удаления.");
            return;
        }

        list<DLString> lines = desc_to_list(ch->getDescription());
        if (lines.empty()) {
            ch->pecho("Нет ничего для удаления.");
            return;
        }

        lines.pop_back();

        ch->setDescription(desc_from_list(lines));
        desc_show(ch);
        interpret_raw(ch, "confirm", "review");
        return;
    }

    if (argument[0] == '+')
    {
        list<DLString> lines = desc_to_list(ch->getDescription());

        argument++;
        while (dl_isspace(*argument))
            argument++;

        lines.push_back(argument);

        DLString text = desc_from_list(lines);
        if (text.size() > MAX_STRING_LENGTH) {
            ch->pecho("Слишком длинное описание.");
            return;
        }

        ch->setDescription(text);
        desc_show(ch);
        interpret_raw(ch, "confirm", "review");
        return;
    }

    desc_usage(ch);
}



