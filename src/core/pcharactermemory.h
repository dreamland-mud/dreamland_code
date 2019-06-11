/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermemory.h  -  description
                             -------------------
    begin                : Tue May 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef PCHARACTERMEMORY_H
#define PCHARACTERMEMORY_H

#include "xmlstring.h"
#include "xmldate.h"
#include "xmlshort.h"
#include "xmllong.h"
#include "xmlvariablecontainer.h"
#include "xmlrussianstring.h"

#include "xmlattributes.h"
#include "pcmemoryinterface.h"
#include "remortdata.h"
#include "clanreference.h"
#include "profession.h"
#include "hometown.h"
#include "race.h"
#include "religion.h"

/**
 * @author Igor S. Petrenko
 */
class PCharacterMemory : public XMLVariableContainer,  
                         public PCMemoryInterface
{
XML_OBJECT
public:        
    typedef ::Pointer<PCharacterMemory> Pointer;
    
    PCharacterMemory( );
    virtual ~PCharacterMemory( );
    
    // CharacterMemoryInterface
    virtual const DLString& getName( ) const throw( );
    virtual void setName( const DLString& name ) throw( );

    virtual short getLevel( ) const throw( );
    virtual void setLevel( short level ) throw( );

    virtual ProfessionReference & getProfession( ) throw( );

    virtual ReligionReference & getReligion( ) throw( );
    virtual void setReligion( const ReligionReference & );
    
    virtual short getSex( ) const throw( );
    virtual void setSex( short sex ) throw( );

    virtual RaceReference &getRace( ) throw( );
    virtual void setRace( const RaceReference & ) throw( );

    virtual ClanReference &getClan( ) throw( );
    virtual void setClan( const ClanReference & clan ) throw( );

    virtual int get_trust( ) const;

    // PCMemoryInterface
    virtual const DLString& getPassword( ) const throw( );
    virtual void setPassword( const DLString &password ) throw( );

    virtual const Date& getLastAccessTime( ) const throw( );
    virtual void setLastAccessTime( const Date& lastAccessTime ) throw( );

    virtual const DLString& getLastAccessHost( ) const throw( );
    virtual void setLastAccessHost( const DLString& lastAccessHost ) throw( );

    virtual int getTrust( ) const throw( );
    virtual void setTrust( int trust ) throw( );

    virtual int getQuestPoints( ) const throw( );
    virtual void setQuestPoints( int ) throw( );

    virtual int getSecurity( ) const throw( );
    virtual void setSecurity( int ) throw( );

    virtual void setProfession( const ProfessionReference & ) throw( );

    virtual short getClanLevel( ) const throw( );
    virtual void setClanLevel( short clanLevel ) throw( );

    virtual ClanReference &getPetition( ) throw( );
    virtual void setPetition( const ClanReference & ) throw( );

    virtual HometownReference &getHometown( ) throw( );
    virtual void setHometown( const HometownReference & ) throw( );

    virtual XMLAttributes& getAttributes( ) throw( );
    virtual const XMLAttributes& getAttributes( ) const throw( );
    virtual void setAttributes( const XMLAttributes& attributes ) throw( );

    virtual Remorts& getRemorts( ) throw( );
    virtual void setRemorts( const Remorts& remorts ) throw( );

    virtual const RussianString& getRussianName( ) const throw( );
    virtual void setRussianName( const DLString& name ) throw( );

    virtual bool isOnline( ) const;
    virtual PCharacter * getPlayer( );

private:
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString password;
    XML_VARIABLE XMLDate lastAccessTime;
    XML_VARIABLE XMLString lastAccessHost;
    XML_VARIABLE XMLShort level;
    XML_VARIABLE XMLClanReference petition;
    XML_VARIABLE XMLClanReference clan;
    XML_VARIABLE XMLShort clanLevel;
    XML_VARIABLE XMLShort sex;
    XML_VARIABLE XMLAttributes attributes;
    XML_VARIABLE XMLHometownReference hometown;
    XML_VARIABLE XMLProfessionReference profession;
    XML_VARIABLE XMLReligionReference religion;
    XML_VARIABLE XMLRaceReference race;
    XML_VARIABLE Remorts remorts;
    XML_VARIABLE XMLInteger trust;
    XML_VARIABLE XMLInteger questpoints;
    XML_VARIABLE XMLInteger security;
    XML_VARIABLE XMLRussianString russianName;
};

#endif
