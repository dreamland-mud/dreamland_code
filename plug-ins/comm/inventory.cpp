#include "commandtemplate.h"
#include "character.h"
#include "core/object.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"
#include "loadsave.h"

void show_list_to_char( Object *list, Character *ch, bool fShort, 
                        bool fShowNothing, DLString pocket = "", Object *container = NULL );

/*---------------------------------------------------------------------------
 * 'inventory' command 
 *--------------------------------------------------------------------------*/
CMDRUNP( inventory )
{
    // Blind characters can't see their pack -- say so instead of printing an
    // empty "Ты несешь: ничего", which reads like the items vanished.
    if (eyes_blinded( ch )) {
        eyes_blinded_msg( ch );
        return;
    }

    ch->pecho( _("Ты несешь:") );
    show_list_to_char( ch->carrying, ch, true, true );
}
