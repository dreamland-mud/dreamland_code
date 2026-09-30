/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANTYPES_H__
#define __CLANTYPES_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlvector.h"
#include "xmlmap.h"
#include "xmlboolean.h"
#include "xmllonglong.h"
#include "xmllong.h"
#include "xmlenumeration.h"

#include "dlxmlloader.h"
#include "clanreference.h"
#include "clanflags.h"

class Object;
class PCMemoryInterface;

/*
 * reform catalog: what a reformed clan's leader can buy with treasury qp
 */
class ClanCatalogItem : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ClanCatalogItem> Pointer;

    ClanCatalogItem( );

    /** top (unlocks an existing skill), new (a new skill) or upgrade. */
    XML_VARIABLE XMLString type;
    XML_VARIABLE XMLInteger price;
    /** Catalog id that must be owned before this one can be bought. */
    XML_VARIABLE XMLStringNoEmpty prerequisite;
    XML_VARIABLE XMLString nameEn, nameRu, nameUa;
};

/** One bought catalog item. Kept until the next epoch reset. */
class ClanPurchase : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ClanPurchase> Pointer;

    XML_VARIABLE XMLLong time;
    XML_VARIABLE XMLString buyer;
    XML_VARIABLE XMLInteger price;
};

/*
 * membership: clan induction and removal 
 */
class ClanMembership : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ClanMembership> Pointer;
    
    ClanMembership( );
    virtual ~ClanMembership( );

    XML_VARIABLE XMLEnumeration mode;
    XML_VARIABLE XMLInteger minLevel;
    
    XML_VARIABLE XMLBoolean removable;
    XML_VARIABLE XMLClanReference removeSelf;
    XML_VARIABLE XMLClanReference removeBy;

    /** Clan reform switch: automatic ranks, offices as flags. Off for clans outside the reform. */
    XML_VARIABLE XMLBooleanNoFalse reformed;
    /** Reformed clans: treasury gold cap, 0 = default. */
    XML_VARIABLE XMLIntegerNoEmpty goldCap;
    /** Reformed clans, one value per rank 0-8, empty = defaults: max skill %, extra skill levels, effect %. */
    XML_VARIABLE XMLVectorBase<XMLInteger> rankCap, rankLevelBonus, rankPower;
    /** Reformed clans: highest rank a divine class (cleric, paladin, druid) reaches, 0 = no cap. */
    XML_VARIABLE XMLIntegerNoEmpty divineRankCap;
    /** Reformed clans: catalog id -> item. */
    XML_VARIABLE XMLMapBase<ClanCatalogItem> catalog;
};

/*
 * clan bank
 */
class ClanBank : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ClanBank> Pointer;

    virtual ~ClanBank( );
    XML_VARIABLE XMLInteger questpoints;
    XML_VARIABLE XMLInteger gold, silver, diamonds;
};

/*
 * all dynamic clan data (bank, dipl, rating, item)
 */
class ClanData : public XMLVariableContainer, public DLXMLRuntimeLoader {
XML_OBJECT
public:
    typedef ::Pointer<ClanData> Pointer;
    typedef XMLMapBase<XMLInteger> Diplomacy;
    typedef XMLVectorBase<XMLInteger> PKStatus;

    ClanData( const DLString & );
    ClanData( );
    virtual ~ClanData( );
    
    inline ClanBank::Pointer getBank( ) const;
    
    inline bool hasItem( ) const;
    void setItem( Object * );
    void unsetItem( Object * );
    
    int getDiplomacy( Clan * ) const;
    int getProposition( Clan * ) const;
    void setDiplomacy( Clan *, int );
    void setProposition( Clan *, int );

    virtual DLString getTableName( ) const;
    virtual DLString getNodeName( ) const;
    
    void save( );
    void load( );

    XML_VARIABLE PKStatus victory, defeat;
    XML_VARIABLE XMLInteger rating;
    /** Reformed clans: catalog id -> purchase. */
    XML_VARIABLE XMLMapBase<ClanPurchase> purchases;

protected:
    
    XML_VARIABLE XMLPointer<ClanBank> bank;
    XML_VARIABLE Diplomacy diplomacy, proposition;

private:
    DLString name;
    long long itemID;

    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;
};

inline bool ClanData::hasItem( ) const
{
    return itemID != 0;
}
inline ClanBank::Pointer ClanData::getBank( ) const
{
    return bank;
}

#endif

