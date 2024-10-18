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
#include "nounholder.h"

#include "xmlattributes.h"
#include "pcmemoryinterface.h"
#include "remortdata.h"
#include "clanreference.h"
#include "profession.h"
#include "hometown.h"
#include "race.h"
#include "religion.h"
#include "pcskilldata.h"
#include "bonus.h"

/**
 * @author Igor S. Petrenko
 */
class PCharacterMemory : public Grammar::NounHolder,
                         public XMLVariableContainer,  
                         public PCMemoryInterface
{
XML_OBJECT
public:        
    typedef ::Pointer<PCharacterMemory> Pointer;
    
    PCharacterMemory( );
    virtual ~PCharacterMemory( );
    
    // CharacterMemoryInterface
    virtual const DLString& getName( ) const ;
    virtual DLString getNameP(char gram_case) const;
    virtual const char * getNameC( ) const;
    virtual void setName( const DLString& name ) ;

    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;

    virtual short getLevel( ) const ;
    virtual void setLevel( short level ) ;

    virtual ProfessionReference & getProfession( ) ;

    virtual ReligionReference & getReligion( ) ;
    virtual void setReligion( const ReligionReference & );
    
    virtual short getSex( ) const ;
    virtual void setSex( short sex ) ;

    virtual int getAlignment( ) const;
    virtual void setAlignment( int );

    virtual int getEthos( ) const;
    virtual void setEthos( int );

    virtual RaceReference &getRace( ) ;
    virtual void setRace( const RaceReference & ) ;

    virtual ClanReference &getClan( ) ;
    virtual void setClan( const ClanReference & clan ) ;

    virtual int get_trust( ) const;

    // PCMemoryInterface
    virtual const DLString& getPassword( ) const ;
    virtual void setPassword( const DLString &password ) ;

    virtual const Date& getLastAccessTime( ) const ;
    virtual void setLastAccessTime( const Date& lastAccessTime ) ;

    virtual const DLString& getLastAccessHost( ) const ;
    virtual void setLastAccessHost( const DLString& lastAccessHost ) ;

    virtual void setDescription( const DLString&, lang_t lang );
    virtual const char * getDescription( lang_t lang ) const;
    virtual const XMLMultiString & getDescription( ) const;
    virtual void setDescription( const XMLMultiString & ); 

    virtual const DLString& getRussianPretitle( ) const ;
    virtual void setRussianPretitle( const DLString& ) ;
    
    virtual const DLString& getPretitle( ) const ;
    virtual void setPretitle( const DLString& ) ;

    virtual void setTitle( const DLString& );
    virtual const DLString & getTitle( ) const;

    virtual int getTrust( ) const ;
    virtual void setTrust( int trust ) ;

    virtual int getQuestPoints( ) const ;
    virtual void setQuestPoints( int ) ;

    virtual int getSecurity( ) const ;
    virtual void setSecurity( int ) ;

    virtual void setProfession( const ProfessionReference & ) ;

    virtual short getClanLevel( ) const ;
    virtual void setClanLevel( short clanLevel ) ;

    virtual ClanReference &getPetition( ) ;
    virtual void setPetition( const ClanReference & ) ;

    virtual HometownReference &getHometown( ) ;
    virtual void setHometown( const HometownReference & ) ;

    virtual XMLAttributes& getAttributes( ) ;
    virtual const XMLAttributes& getAttributes( ) const ;
    virtual void setAttributes( const XMLAttributes& attributes ) ;

    virtual Remorts& getRemorts( ) ;
    virtual void setRemorts( const Remorts& remorts ) ;

    virtual const RussianString& getRussianName( ) const ;
    virtual void setRussianName( const DLString& name ) ;

    virtual bool isOnline( ) const;
    virtual PCharacter * getPlayer( );
    virtual NPCharacter * getMobile( );

    virtual PCSkills & getSkills();
    virtual void setSkills(const PCSkills &);

    virtual PCBonuses & getBonuses();
    virtual void setBonuses(const PCBonuses &);

    virtual int getStartRoom() const;
    virtual void setStartRoom(int vnum);

    virtual int getLoyalty() const;
    virtual void setLoyalty(int value);


private:
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString password;
    XML_VARIABLE XMLDate lastAccessTime;
    XML_VARIABLE XMLString lastAccessHost;
    XML_VARIABLE XMLStringNoEmpty title;
    XML_VARIABLE XMLStringNoEmpty pretitle;
    XML_VARIABLE XMLStringNoEmpty russianPretitle;
    XML_VARIABLE XMLMultiString description;
    XML_VARIABLE XMLShort level;
    XML_VARIABLE XMLClanReference petition;
    XML_VARIABLE XMLClanReference clan;
    XML_VARIABLE XMLShort clanLevel;
    XML_VARIABLE XMLShort sex;
    XML_VARIABLE XMLInteger alignment;
    XML_VARIABLE XMLEnumeration ethos;
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
    XML_VARIABLE PCSkills skills;
    XML_VARIABLE PCBonuses bonuses;
    XML_VARIABLE XMLInteger start_room;
    XML_VARIABLE XMLInteger loyalty;
};

#endif
