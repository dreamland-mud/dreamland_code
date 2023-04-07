/* $Id: mtalk.cpp,v 1.1.2.4.6.1 2008/02/23 13:41:33 rufina Exp $
 *
 * ruffina, 2003
 */
#include "commandtemplate.h"
#include "xmlattributemarriage.h"
#include "replay.h"

#include "pcharacter.h"
#include "pcharactermanager.h"

CMDRUN( mtalk )
{
    std::basic_ostringstream<char> buf;
    std::basic_ostringstream<char> buf0;
    XMLAttributeMarriage::Pointer attr;
    PCharacter *victim;
    
    if (ch->is_npc( )) {
        ch->pecho("Тебе нельзя.");
        return;
    }

    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );
    
    if (!attr || attr->spouse.getValue( ).empty( )) {
        ch->pecho("Сначала женись, потом поговорим.");
        return;
    }

    if (constArguments.empty( )) {
        ch->pecho( "Сказать что?" );
        return;
    }
    
    victim = dynamic_cast<PCharacter *>( PCharacterManager::find( attr->spouse.getValue( ) ) );

    if (!victim) {
        if (attr->wife.getValue( ))
            ch->pecho("Твой муж отсутствует в мире.");
        else
            ch->pecho("Твоя жена отсутствует в мире.");
        
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

    buf << constArguments << "{x'";
    remember_history_private(victim, buf.str());
    buf << endl;
    buf0 << constArguments << "{x'" << endl;

    victim->send_to( buf );
    ch->send_to( buf0 );
}
