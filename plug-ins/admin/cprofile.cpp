#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "commonattributes.h"
#include "arg_utils.h"
#include "dreamland.h"
#include "interp.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

CMDADM( profile )
{
    DLString arguments = constArguments; 
    DLString arg = arguments.getOneArgument();
    DLString playerName = arguments.getOneArgument();
    
    if (!ch->isCoder()) {
        ch->pecho(_("Эта команда не для тебя."));
        return;
    }

    if (arg.empty() || playerName.empty()) {
        ch->pecho(
            _("Формат:\r\n"
            "profile backup <player_name>  -- сохранить профайлы игрока в каталогах var/db/backup и var/db/oldstyle/backup\r\n"
            "profile recover <player_name> -- восстановить профайлы из backup-каталогов, поверх существующих профайлов\r\n"
            "profile undelete <player_name> -- восстановить профайлы из каталогов var/db/delete и var/db/oldstyle/delete, поверх существующих профайлов\r\n"
            "profile unremort <player_name> <number> -- восстановить конкретный профайл перед ремортом из каталогов var/db/remort и var/db/oldstyle/remort, поверх существующих профайлов\r\n"));

        if (dreamland->hasOption(DL_BUILDPLOT))
            ch->pecho(
            _("profile buildplot <player_name> -- скопировать профайлы с основного мира на стройплощадку, выдав права на OLC и феню\r\n"));

        return;            
    }

    PCMemoryInterface *pcm = PCharacterManager::find(playerName);

    if (arg_is(arg, "recover")) {
        if (pcm && pcm->isOnline()) {
            ch->pecho(_("Персонаж сейчас в мире, не могу перезаписать профайлы."));
            return;
        }

        if(!PCharacterManager::pfRecover(playerName, "", 0)) {
            ch->pecho(_("Упс, не могу восстановить профайл, неверное имя?"));
            return;
        }

        ch->pecho(_("Персонаж восстановлен из резервной копии."));
        return;
    }

    if (arg_is(arg, "unremort")) {
        if (pcm && pcm->isOnline()) {
            ch->pecho(_("Персонаж сейчас в мире, не могу перезаписать профайлы."));
            return;
        }

        DLString num = arguments.getOneArgument();
        Integer remorts;
        if (!Integer::tryParse(remorts, num)) {
            ch->pecho(_("Номер реморта должен быть числом."));
            return;
        }

        if(!PCharacterManager::pfRecover(playerName, "remort", remorts)) {
            ch->pecho(_("Упс, не могу восстановить профайл, неверное имя?"));;
            return;
        }

        ch->pecho(_("Персонаж восстановлен после реморта."));
        return;
    }

    if (arg_is(arg, "undelete")) {
        if (pcm && pcm->isOnline()) {
            ch->pecho(_("Персонаж сейчас в мире, не могу перезаписать профайлы."));
            return;
        }

        if(!PCharacterManager::pfRecover(playerName, "delete", 0)) {
            ch->pecho(_("Не могу восстановить профайл. Возможно, забыли убрать случайное расширение в конце имени файла."));
            return;
        }

        ch->pecho(_("Персонаж восстановлен после удаления."));
        return;
    }

    if (arg_is(arg, "buildplot")) {
        if (!dreamland->hasOption(DL_BUILDPLOT)) {
            ch->pecho(_("Эту команду разумнее использовать на стройплощадке."));
            return;
        }

        if (!pcm) {
            if(!PCharacterManager::pfRecover(playerName, "tmp", 0)) {
                ch->pecho(_("Упс, не могу восстановить профайл. Сперва выполните profile backup на основном мире."));
                return;
            }
            ch->pecho(_("Персонаж восстановлен из резервной копии в каталоге /tmp."));
        }

        pcm = PCharacterManager::find(playerName);
        // Allow to execute fenia scripts, grant OLC rights.
        pcm->getAttributes( ).getAttr<XMLIntegerAttribute>( "feniasecurity" )->setValue(110);
        pcm->setSecurity(99);        
        PCharacterManager::saveMemory(pcm);
        ch->pecho(_("Установлены права на феню и OLC."));

        // Grant editing permissions for most common areas.
        if (!pcm->getAttributes().isAvailable("olc")) {
            DLString cmdArgs = playerName + " set 1 50000 9";
            interpret_raw(ch, "olcvnum", cmdArgs.c_str());
        }

        // Create sandbox area for this player.
        {
            DLString cmdArgs = playerName + " create";
            interpret_raw(ch, "olcvnum", cmdArgs.c_str());
        }
        return;
    }

    if (!pcm) {
        ch->pecho(_("Персонаж %s не найден."), playerName.c_str());
        return;
    }

    if (arg_is(arg, "backup")) {
        if (pcm->isOnline()) {
            ch->pecho(_("Персонаж в мире, сохраняю."));
            pcm->getPlayer()->save();
        }

        if (!PCharacterManager::pfBackup(playerName)) {
            ch->pecho(_("Упс, ошибка при сохранении копии профайла."));
            return;
        }

        ch->pecho(_("Копия профайла сохранена в var/db/backup и /tmp."));
        return;
    }    

    ch->pecho(_("Неправильная подкоманда, см. {y{hcprofile{x для списка."));
}


