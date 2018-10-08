/* $Id$
 *
 * ruffina, 2004
 */
#include "commonattributes.h"
#include "commandtemplate.h"

#include "pcharacter.h"

#include "merc.h"
#include "descriptor.h"
#include "xmlpcstringeditor.h"

#include "gsn_plugin.h"
#include "handler.h"
#include "vnum.h"
#include "mercdb.h"
#include "def.h"

static void desc_show( Character *ch )
{
        if (ch->desc) {
            ch->send_to( "Твое описание:\n\r");
            ch->desc->send(ch->getDescription( ) ? ch->getDescription( ) : "(Отсутствует).\n\r");
        }
}

static void desc_usage( Character *ch )
{
    ostringstream buf;

    buf << "Формат: " << endl
        << "    {W{lRописание показать{lEdescription show{x: показать описание в 'сыром' виде с тегами цветов" << endl
        << "    {W{lRописание +{lEdescription +{x: добавить строку к описанию" << endl
        << "    {W{lRописание -{lEdescription -{x: удалить последнюю строку" << endl
        << "    {W{lRописание копировать{lEdescription copy{x: скопировать описание в буфер редактора (только в веб-клиенте)" << endl
        << "    {W{lRописание вставить{lEdescription paste{x: заменить описание на содержимого буфера редактора (только в веб-клиенте)" << endl;

    ch->send_to( buf );
}

CMDRUNP( description )
{
    DLString args = argument;
    DLString arg = args.getOneArgument();
    char buf[MAX_STRING_LENGTH];

    if (arg_oneof( arg, "show", "показать" )) {
        desc_show( ch );
        return;
    }

    if (arg_oneof( arg, "copy", "копировать" )) {
        if (!ch->getPC( )) 
            return;

        if (!ch->getDescription( ) || !ch->getDescription( )[0]) {
                ch->println("Твое описание пусто, копировать в буфер нечего.");
                return;
        }        

        ch->getPC( )->getAttributes().getAttr<XMLAttributeEditorState>("edstate") 
            ->regs[0].split(ch->getDescription( )); 

        if (ch->desc->websock.state != WS_ESTABLISHED) {
                ch->println("Описание скопировано в буфер редактора, однако пользоваться редактором можно только изнутри веб-клиента.");
        } else {
                ch->println("Описание скопировано в буфер редактора, используй команду {lRвебредактор{lEwebedit{x для редактирования.");
        }
        return;
    }

    if (arg_oneof( arg, "paste", "вставить" )) {
        if (!ch->getPC( )) 
            return;

        DLString str = ch->getPC( )->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].dump( );
        if (str.empty( )) {
            ch->println( "Буфер редактора пуст!" );
            return;
        }
        if (str.size( ) >= MAX_STRING_LENGTH) {
            ch->println("Слишком длиное описание.");
            return;
        }

        ch->setDescription( str.c_str( ));
        ch->println( "Новое описание вставлено из буфера редактора." );
        desc_show( ch );
        return;
    }

    if (argument[0] == '\0') {
        desc_show( ch );
        ch->println("\r\nПодробности читай в {W? {lRописание{lEdescription{x.");
        return;
    }

    {
        buf[0] = '\0';

            if (argument[0] == '-')
            {
            int len;
            bool found = false;
            
            if (!ch->getDescription( ) || !ch->getDescription( )[0])
            {
                ch->send_to("Нет ничего для удаления.\n\r");
                return;
            }
        
              strcpy(buf,ch->getDescription( ));

            for (len = strlen(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = true;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
                        ch->setDescription( buf );

                        if (ch->desc) {
                            ch->send_to( "Твое описание:\n\r");
                            ch->desc->send(ch->getDescription( ) ? ch->getDescription( ) : "(Отсутствует).\n\r");
                        }
                        return;
                    }
                }
            }
            buf[0] = '\0';
            ch->setDescription( buf );
            ch->send_to("Описание удалено.\n\r");
            return;
        }
        else if ( argument[0] == '+' )
        {
            if (ch->getDescription( ))
                strcat( buf, ch->getDescription( ) );
            argument++;
            while ( dl_isspace(*argument) )
                argument++;
        } else {
            desc_usage( ch );
            return;
        }

        if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2 )
        {
            ch->send_to( "Слишком длинное описание.\n\r");
            return;
        }

        strcat( buf, argument );
        strcat( buf, "\n\r" );
        ch->setDescription( buf );
    }
   
    desc_show( ch ); 
}



