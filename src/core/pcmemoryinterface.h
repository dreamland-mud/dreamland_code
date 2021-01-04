/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcmemoryinterface.h  -  description
                             -------------------
    begin                : Tue May 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef PCMEMORYINTERFACE_H
#define PCMEMORYINTERFACE_H

#include "dlobject.h"

class DLString;
class RussianString;
class Date;
class RaceReference;
class RemortBonuses;
class Remorts;
class ClanReference;
class HometownReference;
class ProfessionReference;
class ReligionReference;
class PCharacter;
class NPCharacter;
class XMLAttributes;
class PCSkills;
class PCBonuses;
class PCMemoryInterface;

class CharacterMemoryInterface : public virtual DLObject {
public:
    virtual const DLString& getName( ) const  = 0;
    virtual void setName( const DLString& )  = 0;

    virtual short getLevel( ) const  = 0;
    virtual void setLevel( short )  = 0;

    virtual ProfessionReference & getProfession( )  = 0;
    virtual void setProfession( const ProfessionReference & )  = 0;

    virtual ReligionReference & getReligion( )  = 0;
    virtual void setReligion( const ReligionReference & ) = 0;

    virtual short getSex( ) const  = 0;
    virtual void setSex( short )  = 0;

    virtual RaceReference &getRace( )  = 0;
    virtual void setRace( const RaceReference & )  = 0;
    
    virtual ClanReference &getClan( )  = 0;
    virtual void setClan( const ClanReference & )  = 0;

    virtual int get_trust( ) const = 0;

    virtual const char * getDescription( ) const  = 0;
    virtual void setDescription( const DLString& )  = 0; 

    virtual PCharacter * getPlayer( ) = 0;
    virtual NPCharacter * getMobile( ) = 0;
    virtual PCMemoryInterface *getPCM() { return 0; }
};

/**
 * @author Igor S. Petrenko
 */
class PCMemoryInterface : public virtual CharacterMemoryInterface {
public:
    virtual const DLString& getPassword( ) const  = 0;
    virtual void setPassword( const DLString& )  = 0;

    virtual const Date& getLastAccessTime( ) const  = 0;
    virtual void setLastAccessTime( const Date& )  = 0;

    virtual const DLString& getLastAccessHost( ) const  = 0;
    virtual void setLastAccessHost( const DLString& )  = 0; 

    virtual int getTrust( ) const  = 0;
    virtual void setTrust( int )  = 0;

    virtual int getQuestPoints( ) const  = 0;
    virtual void setQuestPoints( int )  = 0;

    virtual int getSecurity( ) const  = 0;
    virtual void setSecurity( int )  = 0;
   
    virtual ClanReference &getPetition( )  = 0;
    virtual void setPetition( const ClanReference & )  = 0;

    virtual short getClanLevel( ) const  = 0;
    virtual void setClanLevel( short )  = 0;

    virtual HometownReference &getHometown( )  = 0;
    virtual void setHometown( const HometownReference & )  = 0;

    virtual XMLAttributes& getAttributes( )  = 0;
    virtual const XMLAttributes& getAttributes( ) const  = 0;
    virtual void setAttributes( const XMLAttributes& attributes )  = 0;

    virtual Remorts& getRemorts( )  = 0;
    virtual void setRemorts( const Remorts& )  = 0;

    virtual const RussianString& getRussianName( ) const  = 0;
    virtual void setRussianName( const DLString& )  = 0;

    virtual bool isOnline( ) const = 0;
    virtual PCMemoryInterface *getPCM() { return this; }

    virtual PCSkills & getSkills( ) = 0;
    virtual void setSkills(const PCSkills &) = 0;

    virtual PCBonuses & getBonuses() = 0;
    virtual void setBonuses(const PCBonuses &) = 0;
};


#endif
