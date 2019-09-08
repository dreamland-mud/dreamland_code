/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermanager.h  -  description
                             -------------------
    begin                : Mon May 14 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef PCHARACTERMANAGER_H
#define PCHARACTERMANAGER_H

#include "dllist.h"
#include "oneallocate.h"
#include "pcharactermemorylist.h"
#include "dlxmlloader.h"

class PCharacter;
class PCMemoryInterface;


/**
 * @author Igor S. Petrenko
 */
class PCharacterManager : public OneAllocate, public DLXMLRuntimeLoader {
public:        
        typedef ::Pointer<PCharacterManager> Pointer;
        typedef DLList<PCharacter>        ExtractList;

public:
        static const DLString ext;

public:
        PCharacterManager( );
        virtual ~PCharacterManager( );

        virtual DLString getTableName( ) const;
        virtual DLString getNodeName( ) const;
        
        static void quit( PCharacter* );
        static void update( PCharacter* );
        static void extract( PCharacter* );
        static PCMemoryInterface* find( const DLString& );
        static PCharacter * findPlayer( const DLString& );
        static inline const PCharacterMemoryList& getPCM( );
        static PCharacter* getPCharacter( );


public: 
        // implemented in anatolia_core
        static void loadPlayers( );
        static void saveMemory( PCMemoryInterface *pci );
        static void remove( const DLString& );
        static void rename( const DLString&, const DLString& );
        static bool pfDelete( const DLString& );
        static bool pfRemort( PCharacter* );
        static bool pfBackup( const DLString& );
        static bool pfRecover( const DLString&, const DLString&, int );
        
        // implemented in loadsave
        static bool save( PCharacter* );
        static bool load( PCharacter* );
        static PCharacter * create( const DLString & );

private:
        static const DLString PLAYER_TABLE;
        static const DLString REMORTS_TABLE;
        static const DLString BACKUP_TABLE;
        static const DLString DELETED_TABLE;
        static const DLString NODE_NAME;

        static PCharacterManager* thisClass;
        
        static ExtractList extractList;
        static PCharacterMemoryList allList;
};

inline const PCharacterMemoryList& PCharacterManager::getPCM( )
{
        return allList;
}

#endif
