/* $Id: xmlattributeinduct.cpp,v 1.1.6.4.6.2 2008/03/05 21:42:49 rufina Exp $
 *
 * ruffina, 2004
 */

#include "skillmanager.h"
#include "skill.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "gsn_plugin.h"
#include "clan.h"
#include "act.h"
#include "descriptor.h"
#include "xmlattributeinduct.h"

const DLString XMLAttributeInduct::TYPE = "XMLAttributeInduct";

void XMLAttributeInduct::run( PCharacter *ch )
{   
    for (iterator entry = begin( ); entry != end( ); entry++) {
        act_p( entry->message.getValue( ).c_str( ), ch, 0, 0, TO_CHAR, POS_DEAD );
    }
    
    ch->getAttributes( ).eraseAttribute( "induct" );
    ch->updateSkills( );
    ch->save( );
}

void XMLAttributeInduct::addEntry( DLString message )
{
    XMLInductEntry entry;
    
    entry.message = message;
    push_back( entry );
}

void 
XMLAttributeInductListenerPlugin::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;
    XMLAttributeInduct::Pointer attr;
    
    if (newState != CON_PLAYING)
        return;
    if (!d->character)
        return;
    if (!( ch = d->character->getPC( ) ))
        return;
    
    attr = ch->getAttributes( ).findAttr<XMLAttributeInduct>( "induct" );
    if (attr)
        attr->run( ch );

    if (ch->getClan( )->isRecruiter( ch )) {
        PCharacterMemoryList::const_iterator pos;
        const PCharacterMemoryList& list = PCharacterManager::getPCM( );
        int cnt = 0;

        for (pos = list.begin( ); pos != list.end( ); pos++) 
            if (pos->second->getPetition( ) == ch->getClan( ))
                cnt++;
        
        if (cnt > 0)
            ch->pecho( "%1$d петиц%1$Iия|ии|ии ожида%1$Iет|ют|ют твоего рассмотрения.\n", cnt );
    }
}

