#include "commandtemplate.h"
#include "character.h"
#include "core/object.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

void show_list_to_char( Object *list, Character *ch, bool fShort, 
                        bool fShowNothing, DLString pocket = "", Object *container = NULL );

/*---------------------------------------------------------------------------
 * 'inventory' command 
 *--------------------------------------------------------------------------*/
CMDRUNP( inventory )
{
    ch->pecho( _("Ты несешь:") );
    show_list_to_char( ch->carrying, ch, true, true );
}
