/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermanager.cpp  -  description
                             -------------------
    begin                : Mon May 14 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "logstream.h"
#include "grammar_entities_impl.h"

#include "pcharactermanager.h"
#include "pcharactermemorylist.h"
#include "pcharactermemory.h"
#include "pcharacter.h"

#include "dreamland.h"

const DLString PCharacterManager::ext = ".player";
const DLString PCharacterManager::NODE_NAME = "Player";

const DLString PCharacterManager::PLAYER_TABLE = "player";
const DLString PCharacterManager::REMORTS_TABLE = "remorts";
const DLString PCharacterManager::BACKUP_TABLE = "backup";
const DLString PCharacterManager::DELETED_TABLE = "delete";

PCharacterManager* PCharacterManager::thisClass = 0;
PCharacterManager::ExtractList PCharacterManager::extractList;
PCharacterMemoryList PCharacterManager::allList;

PCharacterManager::PCharacterManager( )
{
        checkDuplicate( thisClass );
        thisClass = this;
}

PCharacterManager::~PCharacterManager( )
{
        extractList.clear_delete( );

        allList.clear( );                
        
        thisClass = 0;
}

DLString PCharacterManager::getTableName( ) const
{
    return PLAYER_TABLE;
}

DLString PCharacterManager::getNodeName( ) const
{
    return NODE_NAME;
}

PCMemoryInterface* PCharacterManager::find( const DLString& name )
{
    PCharacterMemoryList::iterator ipos;
    
    if (name.empty( ))
        return NULL;

    if (name.isRussian( )) {
        for (ipos = allList.begin( ); ipos != allList.end( ); ipos++)
            if (ipos->second->getRussianName( ).decline('7').isName( name ))
                break;
    } else {
        ipos = allList.find( name );
    }

    if (ipos != allList.end( ))
        return ipos->second;
    else
        return NULL;
}

PCharacter * PCharacterManager::findPlayer( const DLString &name )
{
    PCMemoryInterface *pcm = find( name );

    return (pcm ? pcm->getPlayer( ) : NULL);
}

void PCharacterManager::quit( PCharacter *ch )
{
    PCharacterMemoryList::iterator ipos = allList.find( ch->getName( ) );
    if( ipos != allList.end( ) )
    {
        if( PCharacter* pc = dynamic_cast<PCharacter*>( ipos->second ) )
        {
            if(pc != ch) {
                LogStream::sendWarning( ) << "quit::PCharacter ignoring fake quit for '" << ch->getName() << "'" << endl;
            } else {
                allList[ch->getName()] = pc->getMemory( );
            }
        }
    }
    else
    {
        LogStream::sendWarning( ) << "quit::PCharacter '" << ch->getName() << "' not found" << endl;
    }
}

void PCharacterManager::update( PCharacter* pc )
{
        PCharacterMemoryList::iterator ipos = allList.find( pc->getName( ) );
        if( ipos != allList.end( ) )
        {
                if( PCharacterMemory* pcm = dynamic_cast<PCharacterMemory*>( ipos->second ) )
                {
                        pc->setMemory( pcm );
                        ddeallocate( pcm );
                }
        }

        allList[pc->getName( )] = pc;
}


PCharacter* PCharacterManager::getPCharacter( )
{
    PCharacter *pch;

    if( !extractList.empty( ) )
    {
        pch = *extractList.begin( );
        extractList.erase( extractList.begin( ) );
    }
    else
    {
        pch = dallocate( PCharacter );
    }

    pch->setID( dreamland->genID( ) );
    return pch;
}

void PCharacterManager::extract( PCharacter* pch )
{
    pch->extract( );
    extractList.push_back( pch );
}

