/* $Id: gquestnotifyplugin.cpp,v 1.1.2.2.6.2 2008/11/13 03:33:28 rufina Exp $
 *
 * ruffina, 2003
 */
#include <sstream>

#include "gquestnotifyplugin.h"
#include "globalquestmanager.h"
#include "globalquest.h"
#include "globalquestinfo.h"
#include "gqchannel.h"

#include "pcharacter.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"

void GQuestNotifyPlugin::run( int oldState, int newState, Descriptor *d ) 
{
    if (newState != CON_PLAYING)
        return;
    
    if (!d->character)
        return;

    std::basic_ostringstream<char> buf;
    Character *ch = d->character;

    if (ch->is_npc( ))
        return;
    
    GlobalQuest::Pointer gq;
    GlobalQuestInfo::Pointer gqi;
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    GlobalQuestManager::RunList::iterator i;
    GlobalQuestManager::RunList &rl = manager->getRunning( );

    for (i = rl.begin( ); i != rl.end( ); i++) {
        gq = i->second;
        gqi = manager->findGlobalQuestInfo( i->first );
        
        if (!gqi->canParticipate( ch->getPC( ) ))
            continue;

        if (!gq->isLevelOK( ch ))
            continue;
            
        buf << "         Квест " 
            << GQChannel::BOLD << "\""<< gqi->getQuestName( ) << "\""
            << GQChannel::NORMAL << " (для ";
            
        if (gq->hasLevels( ))
            buf << GQChannel::BOLD << gq->getMinLevel( ) 
                << "-" << gq->getMaxLevel( ) << GQChannel::NORMAL;
        else
            buf << "всех";
        
        buf << " уровней)" << endl;
    }

    if (!buf.str( ).empty( )) {
        GQChannel::pecho( ch, "\r\nГлобальные квесты, в которых ты можешь принять участие: ");
        GQChannel::pecho( ch, buf );
    }
}
    
