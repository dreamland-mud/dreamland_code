/* $Id$
 *
 * ruffina, 2004
 */
#pragma implementation
#include "playerattributes.h"

#include "xmlattributes.h"
#include "pcharacter.h"

template class AttributeEventHandler<ScoreArguments>;
template class AttributeEventHandler<RemortArguments>;
template class AttributeEventHandler<DeathArguments>;
template class AttributeEventHandler<PromptArguments>;
template class AttributeEventHandler<WhoisArguments>;
template class AttributeEventHandler<StopFightArguments>;
template class AttributeEventHandler<AfkArguments>;

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
