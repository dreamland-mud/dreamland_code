#include "commandtemplate.h"
#include "character.h"
#include "core/object.h"
#include "merc.h"
#include "def.h"

bool show_char_equip( Character *ch, Character *victim, ostringstream &buf, bool fShowEmpty );

/*---------------------------------------------------------------------------
 * 'equipment' command 
 *--------------------------------------------------------------------------*/
CMDRUNP( equipment )
{
    ostringstream buf;
    bool naked;
    
    naked = show_char_equip( ch, ch, buf, true );

    ch->pecho( "Ты используешь:" );
    ch->send_to( buf );

    if (naked)
        ch->pecho( "На тебе совсем ничего нет -- не мешало бы одеться!" );
}

