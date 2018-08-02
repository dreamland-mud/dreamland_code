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
#include "xmlenumeration.h"

#include "dlxmlloader.h"
#include "clanreference.h"
#include "clanflags.h"

class Object;
class PCMemoryInterface;

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

protected:
    
    XML_VARIABLE XMLPointer<ClanBank> bank;
    XML_VARIABLE Diplomacy diplomacy, proposition;
    XML_VARIABLE XMLLongLong itemID;

private:
    DLString name;

    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;
};

inline bool ClanData::hasItem( ) const
{
    return itemID.getValue( ) != 0;
}
inline ClanBank::Pointer ClanData::getBank( ) const
{
    return bank;
}

#endif

