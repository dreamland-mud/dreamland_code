/* $Id: mtalk.cpp,v 1.1.2.4.6.1 2008/02/23 13:41:33 rufina Exp $
 *
 * ruffina, 2003
 */
#include "commandtemplate.h"
#include "xmlattributemarriage.h"
#include "replay.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "l10n.h"
#include "mudtags.h"
#include "chatframe.h"

CMDRUN( mtalk )
{
    std::basic_ostringstream<char> buf;
    std::basic_ostringstream<char> buf0;
    XMLAttributeMarriage::Pointer attr;
    PCharacter *victim;
    
    if (ch->is_npc( )) {
        ch->pecho(_("Тебе нельзя."));
        return;
    }

    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );
    
    if (!attr || attr->spouse.getValue( ).empty( )) {
        ch->pecho(_("Сначала женись, потом поговорим."));
        return;
    }

    if (constArguments.empty( )) {
        ch->pecho( _("Сказать что?") );
        return;
    }
    
    victim = dynamic_cast<PCharacter *>( PCharacterManager::find( attr->spouse.getValue( ) ) );

    if (!victim) {
        if (attr->wife.getValue( ))
            ch->pecho(_("Твой муж отсутствует в мире."));
        else
            ch->pecho(_("Твоя жена отсутствует в мире."));
        
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

    // Drop player-injected {h command tags before the text reaches the spouse's screen.
    // This channel is off the CommunicationChannel framework, so it strips directly.
    DLString msg = constArguments;
    mudtags_strip_web( msg, ch );

    buf << msg << "{x'";
    remember_history_private(victim, buf.str());

    // The two lines as the consoles will show them, kept before the newline
    // goes on: a frame carries the line, not the line feed.
    DLString lineVict = buf.str( );
    buf << endl;

    buf0 << msg << "{x'";
    DLString lineChar = buf0.str( );
    buf0 << endl;

    victim->send_to( buf );
    ch->send_to( buf0 );

    // Off the framework as well, and with one listener it checks nothing at
    // all: a spouse who is in the world hears this, and that is the whole rule.
    if (chat_subscribed( victim ))
        chat_emit( victim, ch, false, "mtalk", "personal", lineVict );
    if (chat_subscribed( ch ))
        chat_emit( ch, victim, true, "mtalk", "personal", lineChar );
}
