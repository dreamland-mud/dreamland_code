/* $Id: xmlattributeplugin.cpp,v 1.1.2.2 2008/02/24 17:26:57 rufina Exp $
 *
 * ruffina, 2003
 */

#include "xmlattribute.h"
#include "xmlattributes.h"
#include "xmlattributeplugin.h"
#include "pcmemoryinterface.h"
#include "pcharactermemorylist.h"
#include "pcharactermanager.h"


void XMLAttributePlugin::initialization( )
{
    PCharacterMemoryList::const_iterator i;
    const DLString& name = getName( );
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();
	 
    for( i = pcm.begin(); i != pcm.end(); i++ )
    { 
	XMLAttributes::iterator ipos;
	XMLAttributes &attrs = i->second->getAttributes( );

	for( ipos = attrs.begin( ); ipos != attrs.end( ); ipos++ )
	    if (ipos->second->getType( ) == name)
		ipos->second.recover( );
    }
}

void XMLAttributePlugin::destruction( )
{
    PCharacterMemoryList::const_iterator i;
    const DLString& name = getName( );
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();

    for( i = pcm.begin(); i != pcm.end(); i++ )
    {
	XMLAttributes::iterator ipos;
	XMLAttributes &attrs = i->second->getAttributes( );

	for( ipos = attrs.begin( ); ipos != attrs.end( ); ipos++ )
	    if (ipos->second->getType( ) == name)
		ipos->second.backup( );
    }
}

