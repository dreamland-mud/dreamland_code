/* $Id$
 *
 * ruffina, 2004
 */
#pragma implementation
#include "playerattributes.h"

#include "xmlattributes.h"
#include "pcharacter.h"

template class EventHandler<RemortArguments>;
template class EventHandler<DeathArguments>;
template class EventHandler<PromptArguments>;
template class EventHandler<WhoisArguments>;

bool RemortAttribute::handle( const RemortArguments &args )
{
    XMLAttributes &attributes = args.pch->getAttributes( );
    XMLAttributes::iterator i;
    
    for (i = attributes.begin( ); i != attributes.end( ); i++)
	if (i->second.getPointer( ) == this) {
	    (*(args.newAttributes))[i->first].setPointer( this );
	    return false;
	}

    return false;
}
