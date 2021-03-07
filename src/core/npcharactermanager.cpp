/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          charactermanager.cpp  -  description
                             -------------------
    begin                : Sun May 20 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "class.h"
#include "npcharactermanager.h"
#include "npcharacter.h"

#include "dreamland.h"
#include "merc.h"
#include "def.h"

NPCharacterManager* NPCharacterManager::thisClass = 0;
NPCharacterManager::ExtractList NPCharacterManager::extractList;

NPCharacterManager::NPCharacterManager( ) 
{
    checkDuplicate( thisClass );
    thisClass = this;
}

NPCharacterManager::~NPCharacterManager( )
{
    extractList.clear_delete( );
    thisClass = 0;
}

void NPCharacterManager::extract( NPCharacter* ch )
{
    ch->extract( );
    extractList.push_back( ch );
}

NPCharacter* NPCharacterManager::getNPCharacter( )
{
    NPCharacter *ch;

    if( !extractList.empty( ) )
    {
        ch = *extractList.begin( );
        extractList.erase( extractList.begin( ) );
    }
    else
    {
        ch = dallocate( NPCharacter );
    }
    
    ch->setID( dreamland->genID( ) );
    return ch;
}

NPCharacter* NPCharacterManager::find(long long ID) 
{
    for (Character *ch = char_list; ch; ch = ch->next)
        if (ch->is_npc() && ch->getID() == ID)
            return ch->getNPC();

    return 0;
}

