/* $Id$
 *
 * ruffina, 2004
 */
#include "drink_utils.h"

#include "object.h"
#include "character.h"

#include "itemflags.h"
#include "merc.h"
#include "def.h"

bool drink_is_closed( Object *obj, Character *ch )
{
    if (obj->item_type != ITEM_DRINK_CON)
        return false;

    if (!IS_SET(obj->value3(), DRINK_CLOSED))
        return false;
    
    if (IS_SET(obj->value3(), DRINK_CLOSE_CORK))
        ch->pecho( "%1$^O1 закупоре%1$Gно|н|на пробкой, для начала стоит ее вынуть.", obj );
    else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL))
        ch->pecho( "%1$^O1 закры%1$Gто|т|та крышкой, для начала стоит ее открыть.", obj );
    else if (IS_SET(obj->value3(), DRINK_CLOSE_KEY))
        ch->pecho( "%1$^O1 запер%1$Gто|т|та.", obj );
    else
        ch->println( "Тут закрыто." );
    
    return true;
}


