/* $Id: idelete.cpp,v 1.1.2.3.24.4 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 */
#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "regexp.h"

void password_set( PCMemoryInterface *pci, const DLString &plainText );

CMDADM( idelete )
{
    PCMemoryInterface *pci;
    DLString name = constArguments;

    if (name.empty( )) {
	ch->println( "Удалить чей профайл?" );
	return;
    }

    if (!( pci = PCharacterManager::find( name ) )) {
	ch->println( "Персонаж с таким именем не найден." );
	return;
    }

    if (pci->isOnline( )) {
	ch->println( "Персонаж присутствует в мире, удаление невозможно." );
	return;
    }
    
    if (!PCharacterManager::pfDelete( constArguments ))
	ch->println( "Ошибка при удалении профайла!" );
    else
	ch->println( "Ok." );
}


CMDADM( ipassword )
{
    PCMemoryInterface *pci;
    DLString arguments = constArguments; 
    DLString name = arguments.getOneArgument( );
    DLString passwd = arguments.getOneArgument( );

    if (name.empty( )) {
	ch->println( "Установить пароль кому?" );
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
            }
        }

        ch->printf( "Захешированы пароли %d персонажей из %d.\r\n", cnt, pcm.size( ) );
        return;
    }

    if (passwd.empty( )) {
        ch->println("Использование: ipassword <player name> <new password>.");
        return;
    }

    if (!( pci = PCharacterManager::find( name ) )) {
	ch->println( "Персонаж с таким именем не найден." );
	return;
    }
    
    password_set( pci, passwd );
    ch->println( "Новый пароль установлен и сохранен." );
}


