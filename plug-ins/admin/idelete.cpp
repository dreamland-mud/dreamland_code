/* $Id: idelete.cpp,v 1.1.2.3.24.4 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "regexp.h"
#include "dreamland.h"
#include "comm.h"

HOMETOWN(frigate);
void password_set( PCMemoryInterface *pci, const DLString &plainText );

CMDADM( idelete )
{
    PCMemoryInterface *pci;
    DLString name = constArguments;

    if (name.empty( )) {
        ch->pecho( "Удалить чей профайл?" );
        return;
    }

    if (name == "idle" && ch->isCoder()) {
        ostringstream buf;
        list<PCMemoryInterface *> victims;

        buf << "Удаляю персонажей 1го уровня на Фрегате, не заходивших год:" << endl;
        
        for (auto &pair: PCharacterManager::getPCM()) {
            PCMemoryInterface *pc = pair.second;
            if (pc->getRemorts().size() > 0)
                continue;

            if (pc->getLevel() != 1)
                continue;

            if (pc->getHometown() != home_frigate)
                continue;
            
            const time_t lastAccessDate = pc->getLastAccessTime( ).getTime( );
            const time_t currentDate = dreamland->getCurrentTime( );
            const time_t diff = Date::SECOND_IN_YEAR;
            if (currentDate <= lastAccessDate + diff)
                continue;

            if (pc->isOnline())    
                continue;

            victims.push_back(pc);
        }

        for (auto &pc: victims) {
            if (PCharacterManager::pfDelete(pc->getName()))
                buf << "УДАЛЕН " << pc->getName() << ": " << pc->getLastAccessTime().getTimeAsString() << endl;
            else
                buf << "{RОШИБКА{x " << pc->getName() << ": " << pc->getLastAccessTime().getTimeAsString() << endl;
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }


    if (!( pci = PCharacterManager::find( name ) )) {
        ch->pecho( "Персонаж с таким именем не найден." );
        return;
    }

    if (pci->isOnline( )) {
        ch->pecho( "Персонаж присутствует в мире, удаление невозможно." );
        return;
    }
    
    if (!PCharacterManager::pfDelete( constArguments ))
        ch->pecho( "Ошибка при удалении профайла!" );
    else
        ch->pecho( "Ok." );
}


CMDADM( ipassword )
{
    PCMemoryInterface *pci;
    DLString arguments = constArguments; 
    DLString name = arguments.getOneArgument( );
    DLString passwd = arguments.getOneArgument( );

    if (name.empty( )) {
        ch->pecho( "Установить пароль кому?" );
        return;
    }

    if (name == "all") {
        PCharacterMemoryList::const_iterator i;
        const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
        static RegExp hashPattern("^[A-Z0-9\\$]{60,}$", true);
        int cnt = 0;

        for (i = pcm.begin( ); i != pcm.end( ); i++) {
            PCMemoryInterface *pci = i->second;
            const DLString &pwd = pci->getPassword( );

            if (!hashPattern.match( pwd )) {
                password_set( pci, pwd );
                cnt++;
                notice("[password] Hash password for player %s.", pci->getName().c_str());
            }
        }

        ch->printf( "Захешированы пароли %d персонажей из %d.\r\n", cnt, pcm.size( ) );
        return;
    }

    if (passwd.empty( )) {
        ch->pecho("Использование: ipassword <player name> <new password>.");
        return;
    }

    if (!( pci = PCharacterManager::find( name ) )) {
        ch->pecho( "Персонаж с таким именем не найден." );
        return;
    }
    
    password_set( pci, passwd );
    ch->pecho( "Новый пароль установлен и сохранен." );
}


