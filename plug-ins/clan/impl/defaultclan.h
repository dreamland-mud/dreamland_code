/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DEFAULTCLAN_H__
#define __DEFAULTCLAN_H__

#include "xmlvector.h"
#include "xmllimits.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "dlxmlloader.h"

#include "clan.h"
#include "clanreference.h"
#include "clantypes.h"
#include "clantitles.h"
#include "clanorg.h"

/*
 * DefaultClan
 */
class DefaultClan : public Clan, 
                    public XMLTableElement, 
                    public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultClan> Pointer;
    
    DefaultClan( );
    
    virtual const DLString & getName( ) const;
    virtual void setName( const DLString & );
    virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );

    virtual const DLString &getRussianName( ) const;
    virtual const DLString &getShortName( ) const;
    virtual const DLString &getLongName( ) const;
    virtual const DLString &getColor( ) const;
    virtual const DLString &getPaddedName( ) const;
    virtual const DLString &getChannelPattern( ) const;
    
    virtual ClanData * getData( );
    virtual const ClanMembership * getMembership( ) const;
    virtual ClanMembership * getMembership( );
    virtual const ClanTitles * getTitles( ) const;
    virtual const ClanOrgs * getOrgs( ) const;

    virtual bool isDispersed( ) const;
    virtual int  getRecallVnum( ) const;
    virtual bool hasChannel( ) const;
    virtual bool hasDiplomacy( ) const;
    virtual bool isHidden( ) const;
    virtual bool isLeader( PCMemoryInterface * ) const;
    virtual bool isRecruiter( PCMemoryInterface * ) const;
    virtual bool canInduct( PCharacter * ) const;
    virtual const DLString & getTitle( PCMemoryInterface * ) const;
    
    virtual void handleVictory( PCharacter *, PCharacter * );
    virtual void handleDefeat( PCharacter *, PCharacter * );
    virtual void makeMonument( Character *, Character * ) const;
    virtual bool isEnemy( const Clan & );

protected:
    XML_VARIABLE XMLString shortName, longName, padName, nameRus;
    XML_VARIABLE XMLString color;
    XML_VARIABLE XMLString channelPattern;

    XML_VARIABLE XMLBoolean channel, dispersed, diplomacy, hidden;
    XML_VARIABLE XMLInteger recallVnum;

    XML_VARIABLE XMLVectorBase<XMLString> deities;
    XML_VARIABLE XMLVectorBase<XMLClanReference> enemies;
    XML_VARIABLE XMLString monument;
    
    XML_VARIABLE XMLInteger leader, recruiter;
    XML_VARIABLE XMLPointer<ClanMembership> membership;
    XML_VARIABLE XMLPointer<ClanTitles> titles;
    XML_VARIABLE XMLPointer<ClanOrgs> orgs;
    
    XML_VARIABLE XMLLimits induct;

    ClanData::Pointer data;
};

#endif
