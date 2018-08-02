/* $Id: mtalk.cpp,v 1.1.2.4.6.1 2008/02/23 13:41:33 rufina Exp $
 *
 * ruffina, 2003
 */
#include "commandtemplate.h"
#include "xmlattributemarriage.h"

#include "pcharacter.h"
#include "pcharactermanager.h"

CMDRUN( mtalk )
{
    std::basic_ostringstream<char> buf;
    std::basic_ostringstream<char> buf0;
    XMLAttributeMarriage::Pointer attr;
    PCharacter *victim;
    
    if (ch->is_npc( )) {
	ch->send_to( "Тебе нельзя.\r\n" );
	return;
    }

    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );
    
    if (!attr || attr->spouse.getValue( ).empty( )) {
	ch->send_to( "Сначала женись, потом поговорим.\r\n" );
	return;
    }

    if (constArguments.empty( )) {
	ch->send_to( "Сказать что?" );
	return;
    }
    
    victim = dynamic_cast<PCharacter *>( PCharacterManager::find( attr->spouse.getValue( ) ) );

    if (!victim) {
	if (attr->wife.getValue( ))
	    ch->send_to( "Твой муж отсутствует в мире.\r\n" );
	else
	    ch->send_to( "Твоя жена отсутствует в мире.\r\n" );
	
	return;
    }
    
    if (attr->wife.getValue( )) {
	buf << "Твоя жена говорит тебе '{G";
	buf0 << "Ты говоришь мужу '{G";
    }
    else {
	buf << "Твой муж говорит тебе  '{G";
	buf0 << "Ты говоришь жене '{G";
    }

    buf << constArguments << "{x'" << endl;
    buf0 << constArguments << "{x'" << endl;

    victim->send_to( buf );
    ch->send_to( buf0 );
}
