#include "pcharacter.h"
#include "commandtemplate.h"
#include "wiznet.h"
#include "infonet.h"
#include "messengers.h"
#include "player_account.h"
#include "act.h"
#include "def.h"

void password_set( PCMemoryInterface *pci, const DLString &plainText );
bool password_check( PCMemoryInterface *pci, const DLString &plainText );

CMDRUN( password )
{
    DLString args(constArguments);
    DLString argOld = args.getOneArgument();
    DLString argNew = args.getOneArgument();
    
    if (argOld.empty() || argNew.empty())
    {
        ch->pecho("Синтаксис: пароль <старый> <новый>.");
        return;
    }
    
    if (!password_check( ch->getPC( ), argOld ))
    {
        ch->setWait(40 );
        ch->pecho("Неверный пароль. Подожди 10 секунд.");
        return;
    }

    if (argNew.length() < 5)
    {
        ch->pecho("Новый пароль должен содержать более пяти символов.");
        return;
    }

    password_set( ch->getPC( ), argNew );
    ch->getPC( )->save( );
    ch->pecho("Ok.");
}


/* RT code to delete yourself */

CMDRUNP( delete )
{
    PCharacter *pch;
    
    if ( ch->is_npc() )
           return;
    
    pch = ch->getPC( );

    if (pch->getAttributes().isAvailable("nodelete")) {
        pch->pecho("Твой персонаж могут удалить только Боги.");
        return;
    }

    if (pch->confirm_delete)
    {
        if (!password_check( pch, argument ))
        {
            pch->pecho("Попытка суицида отменена -- неверный пароль.");
            pch->confirm_delete = false;
            return;
        }
        else
        {
            wiznet( WIZ_SECURE, 0, pch->get_trust( ), 
                   "%1$^C1 превращает себя в помехи в проводах.", pch );
            DLString msg;
            msg = fmt(0, "{1{C%1$^C1 идет по пути Арханта и совершает суицид, навсегда покидая этот мир.", pch);
            infonet(pch, 0, "{CТихий голос из $o2: ", msg.c_str());
            send_to_discord_stream(":ghost: " + msg); // discord only here, explicitly asked for by players
            
            Player::quitAndDelete( pch );
            return;
        }
    }

    pch->pecho("{RВНИМАНИЕ: {WЭТО НЕОБРАТИМОЕ ДЕЙСТВИЕ, ТВОЙ ПЕРСОНАЖ БУДЕТ УДАЛЕН НАСОВСЕМ!{x");
    pch->pecho("Введи {yудалить <твой пароль>{x для подтверждения команды.");
    pch->pecho("Чтобы отменить попытку суицида, введи {yудалить без пароля.");
    pch->getPC( )->confirm_delete = true;
    wiznet( WIZ_SECURE, 0, pch->get_trust( ), 
            "%^C1 собирается удалить своего персонажа.", pch );
}
