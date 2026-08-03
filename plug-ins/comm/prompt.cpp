#include "commandtemplate.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "arg_utils.h"
#include "descriptor.h"
#include "merc.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

CMDRUNP( prompt )
{
    DLString old;

    if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->comm,COMM_PROMPT))
        {
            ch->pecho(_("Вывод строки состояния выключен."));
            REMOVE_BIT(ch->comm,COMM_PROMPT);
        }
        else
        {
            ch->pecho(_("Вывод строки состояния включен."));
            SET_BIT(ch->comm,COMM_PROMPT);
        }
        return;
    }

    if (arg_is_all( argument )) {
        old = ch->prompt;
        ch->prompt = Player::defaultPrompt( Player::displayLang(ch), false );
    }
    else if (arg_is_show( argument )) {
        ch->pecho( _("Текущая строка состояния:") );
        ch->desc->send( ch->prompt.c_str( ) );
        ch->pecho("");
        return;
    }    
    else {
          old = ch->prompt;
        ch->prompt = argument;
    }
    
    if (!old.empty( )) {
            ch->send_to( fmt( ch, _("Предыдущая строка состояния: ") ) );
            ch->desc->send(  old.c_str( ) );   
               ch->pecho("");
    }
    ch->pecho(_("Новая строка состояния: %s"),ch->prompt.c_str( ) );
}

CMDRUNP( battleprompt )
{
    DLString old;

   if ( argument[0] == '\0' )
   {
      ch->pecho(_("Необходимо указать вид строки состояния.\nДля получения более подробной информации напиши {y{hcсправка строка состояния{x'"));
      return;
   }

    if (arg_is_all( argument )) {
        old = ch->batle_prompt;
        ch->batle_prompt = Player::defaultPrompt( Player::displayLang(ch), true );
    }
    else if (arg_is_show( argument )) {
        ch->pecho( _("Текущая строка состояния в бою:") );
        ch->desc->send( ch->batle_prompt.c_str( ) );
        ch->pecho("");
        return;
    }    
    else {
        old = ch->batle_prompt;
        ch->batle_prompt = argument;
    }

    if (!old.empty( )) {
            ch->send_to( fmt( ch, _("Предыдущая строка состояния в бою: ") ) );
            ch->desc->send(  old.c_str( ) );   
               ch->pecho("");
    }
    ch->pecho(_("Новая строка состояния в бою: %s"),ch->batle_prompt.c_str( ) );
}


