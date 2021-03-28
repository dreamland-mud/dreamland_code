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
#include "comm.h"
#include "merc.h"
#include "def.h"

CLAN(none);

CMDADM(finger)
{
    std::basic_ostringstream<char> str;
    DLString args = constArguments;
    DLString playerName = args.getOneArgument();
    DLString ipAddress = playerName;
    bool showIP = args.getOneArgument() == "ip";

    if (!ch->is_immortal())
        return;

    if (playerName.empty()) {
        ch->pecho("Использование:\r\n"
                    "finger имя   - поиск игрока по полному имени\r\n"
                    "finger имя ip - просмотр всех IP адресов игрока\r\n"
                    "finger адрес - список игроков, использовавших этот IP адрес");
        return;
    }

    // 'finger name [ip]'
    if (PCMemoryInterface *pci = PCharacterManager::find(playerName)) {
        // 'finger name ip' - display list of IP addresses only.
        if (showIP) {
            str << "List of all IP addresses for " << pci->getName() << ":" << endl;
            XMLAttributeLastHost::Pointer attr = pci->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
            if (attr) {
                attr->showHosts(str);
            }
            ch->send_to(str);
            return;
        }

        // 'finger name' - display player stats and access details.
        str << "{gName:{x " << pci->getName() << "  "
            << "{gRussian name:{x " << pci->getRussianName().getFullForm()
            <<  endl;

        str << "{gLevel:{x " << pci->getLevel() << "  "
            << "{gRace:{x " << pci->getRace()->getName() << "  "
            << "{gClass:{x " << pci->getProfession()->getName() << "  "
            << "{gHome:{x " << pci->getHometown()->getName() << "  "
            << endl;

        str << "{gRemorts:{x " << pci->getRemorts().size() << "  "
            << "{gQuest points:{x " << pci->getQuestPoints() << std::endl;

        if (pci->getClan() != clan_none || pci->getPetition() != clan_none)
            str << "{gClan:{x " << pci->getClan()->getShortName() << "  "
                << "{gClanLevel:{x " << pci->getClanLevel() << "  "
                << "{gPetition:{x " << pci->getPetition()->getShortName()
                << endl;

        str << "{gLast time:{x " << pci->getLastAccessTime().getTimeAsString() << std::endl
            << "{gLast host:{x " << pci->getLastAccessHost() << endl
            << "{gAll hosts:{x Use 'finger " << playerName << " ip'" << endl;

        str << "{gDescription:{x" << endl
            << pci->getDescription();

        ch->send_to(str);
        return;
    }

    // If the argument starts with a digit, assume it's an IP address and find everyone 
    // who ever used the same IP.
    if (isdigit(ipAddress.at(0))) {
        const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
        PCharacterMemoryList::const_iterator p;
        bool found = false;

        for (p = pcm.begin( ); p != pcm.end( ); p++) {
            XMLAttributeLastHost::Pointer attr = p->second->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
            if (!attr)
                continue;

            // Strict match.
            if (attr->hasHost(ipAddress)) {
                str << p->first << ": " << ipAddress << endl;
                found = true;
                continue;
            } 

            // Prefix match.
            DLString myIP = attr->getMatchingHost(ipAddress);
            if (!myIP.empty()) {
                str << p->first << ": " << myIP << endl;
                found = true;
            }
        }

        if (!found) {
            ch->pecho("Не найдено никого с таким IP адресом.");
            return;
        }

        ch->pecho("Список всех игроков, заходивших с этого или похожего адреса:");
        page_to_char(str.str().c_str(), ch);
        return;
    } 

    ch->pecho("Не найдено никого с таким именем.");
}
