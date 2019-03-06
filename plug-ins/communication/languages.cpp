/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>

#include "commandtemplate.h"
#include "pcharacter.h"
#include "race.h"
#include "merc.h"
#include "def.h"

CMDRUN( speak )
{
    RaceLanguage *lang;
    DLString arg = constArguments;
    
    if (ch->is_npc( ))
        return;
    
    arg = arg.getOneArgument( );

    if (arg.empty( )) {
        ostringstream buf, mybuf;
        int mylangs = 0;
        
        buf << "Сейчас ты разговариваешь на "
            << ch->language->getShortDescr( ).ruscase( '6' ) << " языке "
            << "(" << ch->language->getName( ) << ")." << endl;
        
        for (int i = 0; i < raceLanguageManager->size( ); i++) {
            lang = raceLanguageManager->find( i );

            if (!lang->available( ch ))
                continue;
            
            if (!mybuf.str( ).empty( ))
                mybuf << ", ";

            mybuf << lang->getShortDescr( ).ruscase( '1' ) << " (" << lang->getName( ) << ")";
            mylangs++;
        }
        
        if (mylangs == 0) {
        }
        else if (mylangs == 1) {
            buf <<  "Ты знаешь только " << mybuf.str( ) << " язык." << endl;
        }
        else {
            buf << "Ты знаешь такие языки: " << mybuf.str( ) << endl;
        }
    
        ch->send_to( buf );
        return;
    }
    
    lang = raceLanguageManager->findUnstrict( arg );

    if (!lang) {
        ch->pecho( "Ты никогда не слыша%1$Gло|л|ла об этом языке.", ch );
        return;
    }

    if (!lang->available( ch )) {
        ch->printf( "Ты не умеешь разговаривать на %s языке.\r\n",
                     lang->getShortDescr( ).ruscase( '6' ).c_str( ) );
        return;
    }
    
    ch->printf( "Теперь ты разговариваешь на %s языке.\r\n",
                 lang->getShortDescr( ).ruscase( '6' ).c_str( ) );
    ch->language.assign( lang );
}

