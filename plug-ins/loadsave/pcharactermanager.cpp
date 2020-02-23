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

#include <sstream>

#include "char.h"
#include "logstream.h"
#include "dbio.h"
#include "dlfile.h"
#include "dlfileop.h"
#include "crypto.h"

#include "pcharactermanager.h"
#include "pcharactermemorylist.h"
#include "pcharactermemory.h"
#include "pcharacter.h"

#include "fread_utils.h"
#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

void fread_to_end_string( FILE *pfile );
void password_set( PCMemoryInterface *pci, const DLString &plainText );
void limit_count_on_boot( OBJ_INDEX_DATA *pObjIndex, time_t ts, const DLString &playerName );

static void load_PET( FILE *pfile ) 
{
    const char *word;
    MOB_INDEX_DATA *pFidoIndex = get_mob_index( MOB_VNUM_FIDO );

    word = feof(pfile) ? "End" : fread_word(pfile);
    if (!str_cmp(word,"Vnum"))
    {
            int vnum;
        MOB_INDEX_DATA *pPetIndex;
            
            vnum = fread_number(pfile);
        pPetIndex = get_mob_index( vnum );
        
        if (pPetIndex)
            pPetIndex->count++;
        else
            pFidoIndex->count++; 
    }
    else
       pFidoIndex->count++; 
    
    for ( ; ; ) {
        word = feof( pfile ) ? "End" : fread_word( pfile );
        switch( Char::upper( word[0] ) ) {
        case '*':
            fread_to_eol( pfile );
            break;
        case 'E':
            if( !strcmp( word, "End" ) ) {
                return;
            }
        }
    }
}

static void load_MLT( FILE *pfile ) 
{
  const char *word;

  for ( ; ; ) {
    word = feof( pfile ) ? "End" : fread_word( pfile );
    switch( Char::upper( word[0] ) ) {
      case '*':
        fread_to_eol( pfile );
        break;
      case 'E':
        if( !strcmp( word, "End" ) ) {
            return;
        }
    }
  }
}

static void load_OBJECT( FILE *pfile, const DLString &playerName )
{
  const char *word;
  int  vnum = 0;
  int bVnum;
  OBJ_INDEX_DATA *pObjIndex;
  time_t ts = -1;

  for ( ; ; ) {
    word = feof( pfile ) ? "End" : fread_word( pfile );
    bVnum = false;
    
    if( !strcmp( word, "Vnum" ) ) {
      vnum = fread_number( pfile );
      bVnum = true;
    }
    
    switch( Char::upper( word[0] ) ) {
      case '*':
        fread_to_eol( pfile );
        break;
      
      case 'E':
        if( !strcmp( word, "End" ) ) {
            if (!feof( pfile )) {
                char letter = fread_letter( pfile );
                
                if (letter != '<')
                    ungetc( letter, pfile );
                else {
                    char * xml = fread_string( pfile );
                    free_string( xml );
                }
            }

            if (vnum == 0) {
                LogStream::sendWarning( ) << "load_OBJECT: no vnum found in player profile " << playerName << endl;
                return;
            }

            pObjIndex =  get_obj_index( vnum );
            if (!pObjIndex) {
                LogStream::sendWarning( ) << "load_OBJECT: invalid vnum " << vnum << " for player profile " << playerName << endl;
                return;
            }

            if (pObjIndex->limit < 0) {
                pObjIndex->count++;
                return;
            }

            limit_count_on_boot( pObjIndex, ts, playerName );
            return;
        }
        break;

      case 'T':
        if (!strcmp( word, "TS")) {
            ts = fread_number64( pfile );
        }
        break;

      case 'V':
        if( !strcmp( word, "Vnum" ) && !bVnum ) {
          vnum = fread_number( pfile );
          break;
        }
        break;
    }
  }
}

static void load_PLAYER( PCharacter *pc, FILE *pfile )
{
    char *word;
    bool exit = false;

    while( !exit )
    {
        char letter = fread_letter( pfile );

        if( letter == '*' )
        {
            fread_to_eol( pfile );
            continue;
        }

        if( letter != '#' )
        {
            LogStream::sendError( ) << "load_players_date: # not found." << endl;
            break;
        }
        
        word = fread_word( pfile );
        if( !strcmp( word, "PLAYER" ) )
        {
            while( !exit )
            {
                const char *word0 = feof( pfile ) ? "End" : fread_word( pfile );
                switch( Char::upper( word0[0] ) )
                {
                case '*':
                    fread_to_eol( pfile );
                    break;
                                                
                case 'D':
                    if( !strcmp( word0, "Desc" ) )
                        fread_to_end_string( pfile );
                    break;

                case 'E':
                    if( !strcmp( word0, "End" ) ) {
                        exit = true;
                    }
                    break;
                case 'N':
                    if ( !strcmp( word0, "Name" ) )
                    {
                        char *nm = fread_string( pfile ) ;
                        DLString name( nm );
                        free_string(nm);
                        pc->setName( name );
                    }
                    break;
                case 'P':
                    if (!strcmp( word0, "Pass" )) {
                        char *pwd = fread_string( pfile );
                        password_set( pc, pwd );
                        free_string( pwd );
                    }
                    break;
                }
            }
            
            exit = false;
        }
        else if( !strcmp( word, "OBJECT" ) ||
                 !strcmp( word, "O" ) )   
        {
            load_OBJECT( pfile, pc->getName( ) );
        }
        else if( !strcmp( word, "PET" ) ) 
            load_PET( pfile );
        else if( !strcmp( word, "END" ) ) 
            exit = true;
        else if (!strcmp( word, "MLT" )) {
            load_MLT( pfile );
        } else {
            LogStream::sendError( ) << "load_players_date: bad section." << endl;
            exit = true;
        }
    }
}

void PCharacterManager::loadPlayers( )
{
    DLString buf;
    PCharacter* pc;

    LogStream::sendNotice( ) <<  "Players loading...." << endl;

    DLDirectory dir( dreamland->getPlayerDir( ) );
    dir.open( );

    try {
        while (true) {
            DLFileRead profile( dir, dir.nextTypedEntry( ext ) );

            LogStream::sendNotice( ) << "Try " << profile.getPath( ) << ", " << profile.getFileName( ) << endl;
            
            if (!profile.open( )) 
                continue;
            
            pc = getPCharacter( );
            pc->setName( profile.getFileName( ).capitalize( ) );

            load_PLAYER( pc, profile.getFP( ) );
            load( pc );

            LogStream::sendNotice( ) << pc->getName( ) << " has race " << pc->getRace( )->getName( ) << " and level " << pc->getLevel( ) << endl;

            if( !pc->getName( ).empty( ) ) {
                if( allList.find( pc->getName( ) ) == allList.end( ) )
                {
                    allList[pc->getName( )] = pc->getMemory( );
                }
                else {
                    LogStream::sendWarning( ) << "Player name: '" << pc->getName( ) << "' is duplicated." << endl;
                }
            }
            else {
                LogStream::sendError( ) << "Bad loaded player file " << buf << endl;
            }

            ddeallocate( pc );
        }

    } catch (const ExceptionDBIOEOF &exeof) {
    }

    LogStream::sendNotice( ) << "Total players: " << allList.size( ) << endl;
}


void PCharacterManager::remove( const DLString& name )
{
    PCharacterMemoryList::iterator ipos = allList.find( name );

    if( ipos != allList.end( ) )
    {
        XMLAttributes& attributes = ipos->second->getAttributes( );
        
        while (!attributes.empty( )) 
            attributes.eraseAttribute( attributes.begin( )->first );
        
        allList.erase( ipos );
    }
    else
    {
        LogStream::sendWarning( ) << "remove::PC " << name << " not found" << endl;
    }
}

void PCharacterManager::rename( const DLString& oldName, const DLString& newName )
{
    PCharacterMemoryList::iterator ipos = allList.find( oldName );

    if( ipos != allList.end( ) )
    {
        if( PCharacter* pc = dynamic_cast<PCharacter*>( ipos->second ) )
        {
            DLString newNameLower = newName.toLower( );
            DLString oldNameLower = oldName.toLower( );

            allList.erase( ipos );
            allList[newName] = pc;

            try
            {
                DBIO dbio( dreamland->getDbDir( ), PLAYER_TABLE );
                dbio.renameID( oldNameLower, newNameLower );
            }
            catch( const ExceptionDBIO& ex )
            {
                LogStream::sendWarning( ) << ex << endl;
            }
        }
    }
    else
    {
            LogStream::sendWarning( ) << "rename::PCharacter '" << oldName << "' not found" << endl;
    }
}


void PCharacterManager::saveMemory( PCMemoryInterface *pci )
{
    PCharacter *pc;
    PCharacterMemory *pcm;

    if( (pc = dynamic_cast<PCharacter *>(pci)) ) {
        save( pc );        
        return;
    }
    
    pcm = (PCharacterMemory *)pci;

    pc = getPCharacter( );
    pc->setName( pcm->getName( ) );
    
    load(pc);
    pc->setMemory( pcm );
    save(pc);
    extract(pc);
}

bool PCharacterManager::pfDelete ( const DLString& playerName ) 
{
    DLString name = playerName.toLower( );
    DLFile oldProfile( dreamland->getPlayerDir( ), name, ext );
    DLFile newProfile( dreamland->getPlayerDeleteDir( ), name, ".delete" );

    DLFile oldXml( DLFile( dreamland->getDbDir( ), PLAYER_TABLE ), name, ".xml" );
    DLFile newXml( DLFile( dreamland->getDbDir( ), DELETED_TABLE ), name, ".xml" );

    if (!oldProfile.rename( newProfile )) {
        LogStream::sendSystem( ) << "pfDelete failed for " << playerName << endl;
        return false;
    }
    
    if (!oldXml.copy( newXml )) {
        LogStream::sendSystem( ) << "pfDelete:: " << name << " : backup failed!" << endl;
        return false;
    }

    try {
        DBIO dbio( dreamland->getDbDir( ), PLAYER_TABLE );
        dbio.remove( name );
    }
    catch( const ExceptionDBIO& ex ) {
        LogStream::sendWarning( ) << ex << endl;
    }
                
    PCharacterManager::remove( name );
    return true;
}

bool PCharacterManager::pfRemort ( PCharacter* pch ) 
{
    DLString name = pch->getName( ).toLower( );
    DLString remortExt;
    remortExt << "." << pch->getRemorts( ).size( );

    DLFile oldProfile( dreamland->getPlayerDir( ), name, ext );
    DLFile newProfile( dreamland->getPlayerRemortDir( ), name, remortExt );

    DLFile oldXml( DLFile( dreamland->getDbDir( ), PLAYER_TABLE ), name, ".xml" );
    DLFile newXml( DLFile( dreamland->getDbDir( ), REMORTS_TABLE ), name, remortExt );
    
    pch->save( );
    
    if (!oldXml.copy( newXml )) 
        return false;

    if (!oldProfile.copy( newProfile )) 
        return false;

    return true;
}

bool PCharacterManager::pfBackup ( const DLString &playerName ) 
{
    DLString name = playerName.toLower( );

    DLFile oldProfile( dreamland->getPlayerDir( ), name, ext );
    DLFile newProfile( dreamland->getPlayerBackupDir( ), name, ext );

    DLFile oldXml( DLFile( dreamland->getDbDir( ), PLAYER_TABLE ), name, ".xml" );
    DLFile newXml( DLFile( dreamland->getDbDir( ), BACKUP_TABLE ), name, ".xml" );

    if (!oldXml.copy( newXml )) 
        return false;

    if (!oldProfile.copy( newProfile )) 
        return false;

    return true;
}

bool PCharacterManager::pfRecover ( const DLString &playerName, const DLString &arg, int n ) 
{
    PCharacter *pc;
    DLString name = playerName.toLower( );
    DLFile oldProfile, oldXml; 
    DLFile newProfile( dreamland->getPlayerDir( ), name, ext );
    DLFile newXml( DLFile( dreamland->getDbDir( ), PLAYER_TABLE ), name, ".xml" );


    if (arg=="delete") {
        oldProfile = DLFile( dreamland->getPlayerDeleteDir( ), name, ".delete" );
             oldXml = DLFile( DLFile( dreamland->getDbDir( ), DELETED_TABLE ), name, ".xml" );
    }
    else if (arg=="remort") {
        if (n<0)
            return false;

        DLString remortExt;
        remortExt << "."  << n;

        oldProfile = DLFile( dreamland->getPlayerRemortDir( ), name, remortExt );
        oldXml = DLFile( DLFile( dreamland->getDbDir( ), REMORTS_TABLE ), name, remortExt );
    }
    else {
        oldProfile = DLFile( dreamland->getPlayerBackupDir( ), name, ext );
        oldXml = DLFile( DLFile( dreamland->getDbDir( ), BACKUP_TABLE ), name, ".xml" );
    }
                
    if (!oldXml.copy( newXml )) {
        LogStream::sendError() << "pfRecover: cannot copy " << oldXml.getPath() << " to " << newXml.getPath() << endl;
        return false;
    }
    
    if (!oldProfile.copy( newProfile )) {
        LogStream::sendError() << "pfRecover: cannot copy " << oldProfile.getPath() << " to " << newProfile.getPath() << endl;
        return false;
    }
    
    pc = getPCharacter();
    pc->setName(name);
    load( pc );
        
    allList[pc->getName()] = pc->getMemory();
    ddeallocate(pc);
        
    return true;
}


bool PCharacterManager::save( PCharacter* pc )
{
    DLString name = pc->getName( );
    
    name.toLower( );
    return thisClass->saveXML( pc, name, true );
}

bool PCharacterManager::load( PCharacter* pc )
{
    DLString name= pc->getName( );
    
    name.toLower( );
    return thisClass->loadXML( pc, name );
}

PCharacter * PCharacterManager::create( const DLString &name )
{
    PCharacter *pc = getPCharacter( );

    pc->setName( name );
    pc->load( );
    return pc;
}

bool password_check( PCMemoryInterface *target, const DLString &input )
{
    // Retrieved valid memory interface instance.
    PCMemoryInterface *pci = PCharacterManager::find( target->getName( ) );
    if (!pci) {
        warn("No memory interface found for player %s.", target->getName( ).c_str( ));
        return false;
    }

    DLString hashAndSalt = pci->getPassword( );
    // Support un-encoded logins as a fallback.
    if (hashAndSalt == input) {
        warn( "Player %s still uses plain-text password.", pci->getName( ).c_str( ) );
        return true;
    }
   
    // Split out salt prefix from saved password.
    DLString::size_type pos = hashAndSalt.find( '$' );
    if (pos == DLString::npos || pos >= hashAndSalt.size( ) - 1) {
          warn("Invalid salt separator position for hashed password for %s.", pci->getName( ).c_str( ));
          return false;
    }
  
    DLString hexSalt = hashAndSalt.substr(0, pos);
    
    // Encode user's input with stored salt, to receive hash&salt combination.
    DLString generated = digestWithHexSalt( input, hexSalt );
    // Check stored hash&salt combination against generated one.
    return generated == hashAndSalt;
}

void password_set( PCMemoryInterface *pci, const DLString &password )
{
    // Generate hash&salt combination from provided plain text and unique random salt.
    DLString hashAndSalt = digestWithRandomSalt( password );
    // Store generated combo as user's password.
    pci->setPassword( hashAndSalt );
    // Save user's profile to disk.
    PCharacterManager::saveMemory( pci );
}

