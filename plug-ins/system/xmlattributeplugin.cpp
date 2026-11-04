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
#include "pcharacter.h"

static void restub( XMLAttributes &attrs, const DLString &name, bool backup )
{
    XMLAttributes::iterator ipos;

    for( ipos = attrs.begin( ); ipos != attrs.end( ); ipos++ )
        if (ipos->second->getType( ) == name) {
            if (backup)
                ipos->second.backup( );
            else
                ipos->second.recover( );
        }
}

/*
 * Players in the game that the registry does not hold under their own pointer,
 * e.g. the /api/sandbox TestBot. Their attributes must be stubbed as well,
 * otherwise they keep pointing into the unloaded plugin's code.
 */
static void restub_unregistered( const DLString &name, bool backup )
{
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();

    for (Character *wch = char_list; wch; wch = wch->next) {
        if (wch->is_npc( ))
            continue;

        PCharacter *pc = wch->getPC( );
        PCharacterMemoryList::const_iterator i = pcm.find( pc->getName( ) );

        if (i != pcm.end( ) && i->second == static_cast<PCMemoryInterface *>( pc ))
            continue;

        restub( pc->getAttributes( ), name, backup );
    }
}

void XMLAttributePlugin::initialization( )
{
    PCharacterMemoryList::const_iterator i;
    const DLString& name = getName( );
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();

    for( i = pcm.begin(); i != pcm.end(); i++ )
        restub( i->second->getAttributes( ), name, false );

    restub_unregistered( name, false );
}

void XMLAttributePlugin::destruction( )
{
    PCharacterMemoryList::const_iterator i;
    const DLString& name = getName( );
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();

    for( i = pcm.begin(); i != pcm.end(); i++ )
        restub( i->second->getAttributes( ), name, true );

    restub_unregistered( name, true );
}
