/* $Id: xmlattributequestreward.cpp,v 1.1.4.1.10.1 2007/06/26 07:19:46 rufina Exp $
 *
 * ruffina, 2003
 */

#include "xmlattributequestreward.h"

const DLString XMLAttributeQuestReward::TYPE = "XMLAttributeQuestReward";

int XMLAttributeQuestReward::getCount( int vnum ) const
{
    DLString vnumStr( vnum );
    const_iterator i = find( vnumStr );

    if (i == end( ))
        return 0;
    else
        return i->second.getValue( );
}

void XMLAttributeQuestReward::setCount( int vnum, int count )
{
    DLString vnumStr( vnum );
    iterator i = find( vnumStr );

    if (i == end( ))
        (*this)[vnumStr] = count;
    else
        i->second = count;
}

int XMLAttributeQuestReward::getTier( int vnum ) const
{
    DLString key = DLString( "tier:" ) + DLString( vnum );
    const_iterator i = find( key );

    if (i == end( ))
        return 0;
    else
        return i->second.getValue( );
}

void XMLAttributeQuestReward::setTier( int vnum, int tier )
{
    // The mirror means "highest tier ever paid for this vnum" -- buyObject reads
    // it to restore a lost item. Never rewind it: a player upgrading a second,
    // lower-tier copy of the same vnum must not overwrite the tier paid for the
    // first (which a later recovery would then restamp too low).
    if (tier <= getTier( vnum ))
        return;

    DLString key = DLString( "tier:" ) + DLString( vnum );
    iterator i = find( key );

    if (i == end( ))
        (*this)[key] = tier;
    else
        i->second = tier;
}

