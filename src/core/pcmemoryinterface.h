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
class XMLAttributes;

class CharacterMemoryInterface : public virtual DLObject {
public:
    virtual const DLString& getName( ) const throw( ) = 0;
    virtual void setName( const DLString& ) throw( ) = 0;

    virtual short getLevel( ) const throw( ) = 0;
    virtual void setLevel( short ) throw( ) = 0;

    virtual ProfessionReference & getProfession( ) throw( ) = 0;
    virtual void setProfession( const ProfessionReference & ) throw( ) = 0;

    virtual ReligionReference & getReligion( ) throw( ) = 0;
    virtual void setReligion( const ReligionReference & ) = 0;

    virtual short getSex( ) const throw( ) = 0;
    virtual void setSex( short ) throw( ) = 0;

    virtual RaceReference &getRace( ) throw( ) = 0;
    virtual void setRace( const RaceReference & ) throw( ) = 0;
    
    virtual ClanReference &getClan( ) throw( ) = 0;
    virtual void setClan( const ClanReference & ) throw( ) = 0;

    virtual int get_trust( ) const = 0;
};

/**
 * @author Igor S. Petrenko
 */
class PCMemoryInterface : public virtual CharacterMemoryInterface {
public:
    virtual const DLString& getPassword( ) const throw( ) = 0;
    virtual void setPassword( const DLString& ) throw( ) = 0;

    virtual const Date& getLastAccessTime( ) const throw( ) = 0;
    virtual void setLastAccessTime( const Date& ) throw( ) = 0;

    virtual const DLString& getLastAccessHost( ) const throw( ) = 0;
    virtual void setLastAccessHost( const DLString& ) throw( ) = 0; 

    virtual int getTrust( ) const throw( ) = 0;
    virtual void setTrust( int ) throw( ) = 0;

    virtual int getQuestPoints( ) const throw( ) = 0;
    virtual void setQuestPoints( int ) throw( ) = 0;

    virtual int getSecurity( ) const throw( ) = 0;
    virtual void setSecurity( int ) throw( ) = 0;
   
    virtual ClanReference &getPetition( ) throw( ) = 0;
    virtual void setPetition( const ClanReference & ) throw( ) = 0;

    virtual short getClanLevel( ) const throw( ) = 0;
    virtual void setClanLevel( short ) throw( ) = 0;

    virtual HometownReference &getHometown( ) throw( ) = 0;
    virtual void setHometown( const HometownReference & ) throw( ) = 0;

    virtual XMLAttributes& getAttributes( ) throw( ) = 0;
    virtual const XMLAttributes& getAttributes( ) const throw( ) = 0;
    virtual void setAttributes( const XMLAttributes& attributes ) throw( ) = 0;

    virtual Remorts& getRemorts( ) throw( ) = 0;
    virtual void setRemorts( const Remorts& ) throw( ) = 0;

    virtual const RussianString& getRussianName( ) const throw( ) = 0;
    virtual void setRussianName( const DLString& ) throw( ) = 0;

    virtual bool isOnline( ) const = 0;
    virtual PCharacter * getPlayer( ) = 0;
};


#endif
