/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLAN_H__
#define __CLAN_H__

#include "globalregistryelement.h"

class Character;
class PCharacter;
class Object;
class PCMemoryInterface; 

class ClanData;
class ClanMembership;
class ClanTitles;
class ClanOrgs;

class Clan : public GlobalRegistryElement {
public:
    typedef ::Pointer<Clan> Pointer;
    
    Clan( );
    Clan( const DLString & );
    virtual ~Clan( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;

    virtual const DLString &getShortName( ) const;
    virtual const DLString &getLongName( ) const;
    virtual const DLString &getColor( ) const;
    virtual const DLString &getPaddedName( ) const;
    virtual const DLString &getChannelPattern( ) const;
    
    virtual const DLString & getTitle( PCMemoryInterface * ) const;
    virtual bool isLeader( PCMemoryInterface * ) const;
    virtual bool isRecruiter( PCMemoryInterface * ) const;
    virtual bool canInduct( PCharacter * ) const;
    virtual void onInduct(PCharacter *) const;
    
    virtual void makeMonument( Character *, Character * ) const;
    virtual void handleVictory( PCharacter *, PCharacter * );
    virtual void handleDefeat( PCharacter *, PCharacter * );

    virtual bool isEnemy( const Clan & );
    virtual bool isDispersed( ) const;
    virtual int getRecallVnum( ) const;
    virtual bool hasChannel( ) const;
    virtual bool hasDiplomacy( ) const;
    virtual bool isHidden( ) const;

    virtual ClanData * getData( );
    virtual const ClanMembership * getMembership( ) const;
    virtual ClanMembership * getMembership( );
    virtual const ClanTitles * getTitles( ) const;
    virtual const ClanOrgs * getOrgs( ) const;

protected:
    DLString name;
};

#endif
