/* $Id$
 *
 * ruffina, 2004
 */
#include "clantypes.h"
#include "logstream.h"

#include "object.h"
#include "objectmanager.h"
#include "clanreference.h"

/*-----------------------------------------------------------------
 * ClanData 
 *----------------------------------------------------------------*/
const DLString ClanData::TABLE_NAME = "clan-data";
const DLString ClanData::NODE_NAME = "ClanData";

ClanData::ClanData( )  : itemID(0)
{
    defeat.resize( 5 );
    victory.resize( 5 );
}

ClanData::ClanData( const DLString &n ) : name( n ), itemID(0)
{
    defeat.resize( 5 );
    victory.resize( 5 );
}

ClanData::~ClanData( )
{
}

int ClanData::getDiplomacy( Clan *clan ) const
{
    Diplomacy::const_iterator i;
    
    i = diplomacy.find( clan->getName( ) );

    if (i == diplomacy.end( ))
        return 0;
    else
        return i->second.getValue( );
}

int ClanData::getProposition( Clan *clan ) const
{
    Diplomacy::const_iterator i;
    
    i = proposition.find( clan->getName( ) );

    if (i == proposition.end( ))
        return 0;
    else
        return i->second.getValue( );
}

void ClanData::setDiplomacy( Clan *clan, int dip )
{
    diplomacy[clan->getName( )] = dip;
}

void ClanData::setProposition( Clan *clan, int dip )
{
    proposition[clan->getName( )] = dip;
}

DLString ClanData::getTableName( ) const
{
    return TABLE_NAME;
}

DLString ClanData::getNodeName( ) const
{
    return NODE_NAME;
}

void ClanData::save( )
{
    saveXML( this, name, true );
}

void ClanData::load( )
{
    loadXML( this, name, true );
}

void ClanData::setItem( Object *obj )
{
    itemID = obj->getID( );
    save( );
}

void ClanData::unsetItem( Object *obj )
{
    if (obj->getID( ) == itemID) {
        itemID = 0;
        save( );
    }
}


/*-----------------------------------------------------------------
 * ClanMembership 
 *----------------------------------------------------------------*/
ClanMembership::ClanMembership( )
                : mode( 0, &petition_table ),
                  minLevel( 999 ),
                  removable( false )
{
}

ClanMembership::~ClanMembership( )
{
}

/*-----------------------------------------------------------------
 * ClanBank
 *----------------------------------------------------------------*/
ClanBank::~ClanBank( )
{
}

