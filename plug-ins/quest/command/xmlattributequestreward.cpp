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

