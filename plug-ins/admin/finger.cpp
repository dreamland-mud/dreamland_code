/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          finger.cpp  -  description
                             -------------------
    begin                : Fri May 18 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "admincommand.h"
#include "lasthost.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "character.h"
#include "race.h"
#include "clanreference.h"
#include "merc.h"
#include "def.h"

CLAN(none);

CMDADM(finger)
{
    std::basic_ostringstream<char> str;

    if (!ch->is_immortal())
        return;

    if (constArguments.empty()) {
        ch->println("Использование:\r\nfinger имя   - поиск игрока по полному имени\r\nfinger адрес - список игроков, использовавших этот IP адрес");
        return;
    }

    // 'finger name' - display player stats and access details.
    if (PCMemoryInterface *pci = PCharacterManager::find(constArguments)) {
        str << "Name: " << pci->getName() << std::endl
            << "Level: " << pci->getLevel() << "  "
            << "Race: " << pci->getRace()->getName() << "  "
            << "Class: " << pci->getProfession()->getName().c_str() << std::endl;

        if (pci->getClan() != clan_none)
            str << "Clan: " << pci->getClan()->getShortName() << "  "
                << "ClanLevel: " << pci->getClanLevel() << endl;

        str << "Last time: " << pci->getLastAccessTime().getTimeAsString() << std::endl
            << "Last host: " << pci->getLastAccessHost() << endl;

        XMLAttributeLastHost::Pointer attr = pci->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
        if (attr) {
            str << "All hosts:" << endl;
            attr->showHosts(str);
        }
        ch->send_to(str);
        return;
    }

    // If argument starts with digit, assume it's an IP address and find everyone 
    // who ever used the same IP.
    if (isdigit(constArguments.at(0))) {
        const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
        PCharacterMemoryList::const_iterator p;
        bool found = false;

        for (p = pcm.begin( ); p != pcm.end( ); p++) {
            XMLAttributeLastHost::Pointer attr = p->second->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
            if (attr && attr->hasHost(constArguments)) {
                str << p->first << " ";
                found = true;
            }
        }

        if (!found) {
            ch->println("Не найдено никого с таким IP адресом.");
            return;
        }

        ch->println("Список всех игроков, заходивших с этого адреса:");
        str << endl;
        ch->send_to(str);
        return;
    } 

    ch->println("Не найдено никого с таким именем.");
}
