#include "commandtemplate.h"
#include "pcharacter.h"
#include "arg_utils.h"
#include "descriptor.h"
#include "merc.h"
#include "def.h"

CMDRUNP( prompt )
{
    DLString old;

    if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->comm,COMM_PROMPT))
        {
            ch->pecho("Вывод строки состояния выключен.");
            REMOVE_BIT(ch->comm,COMM_PROMPT);
        }
        else
        {
            ch->pecho("Вывод строки состояния включен.");
            SET_BIT(ch->comm,COMM_PROMPT);
        }
        return;
    }

    if (arg_is_all( argument )) {
        old = ch->prompt;
        ch->prompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг {W%X{xоп Вых:{g%d{x %S>%c";
    }
    else if (arg_is_show( argument )) {
        ch->pecho( "Текущая строка состояния:" );
        ch->desc->send( ch->prompt.c_str( ) );
        ch->pecho("");
        return;
    }    
    else {
          old = ch->prompt;
        ch->prompt = argument;
    }
    
    if (!old.empty( )) {
            ch->send_to( "Предыдущая строка состояния: " );
            ch->desc->send(  old.c_str( ) );   
               ch->pecho("");
    }
    ch->pecho("Новая строка состояния: %s",ch->prompt.c_str( ) );
}

CMDRUNP( battleprompt )
{
    DLString old;

   if ( argument[0] == '\0' )
   {
      ch->pecho("Необходимо указать вид строки состояния.\nДля получения более подробной информации напиши {y{hcсправка строка состояния{x'");
      return;
   }

    if (arg_is_all( argument )) {
        old = ch->batle_prompt;
        ch->batle_prompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг %Xоп Вых:{g%d{x %S> [{r%y{x:{Y%o{x]%c";
    }
    else if (arg_is_show( argument )) {
        ch->pecho( "Текущая строка состояния в бою:" );
        ch->desc->send( ch->batle_prompt.c_str( ) );
        ch->pecho("");
        return;
    }    
    else {
        old = ch->batle_prompt;
        ch->batle_prompt = argument;
    }

    if (!old.empty( )) {
            ch->send_to( "Предыдущая строка состояния в бою: " );
            ch->desc->send(  old.c_str( ) );   
               ch->pecho("");
    }
    ch->pecho("Новая строка состояния в бою: %s",ch->batle_prompt.c_str( ) );
}


