/* $Id$
 *
 * ruffina, 2004
 */
#include "tattoo.h"

#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "merc.h"

#include "interp.h"
#include "handler.h"
#include "act.h"
#include "def.h"

/*----------------------------------------------------------------------
 * Religion Tattoo 
 *---------------------------------------------------------------------*/
void ReligionTattoo::fight( Character *ch )
{
    if (obj->wear_loc == wear_tattoo)
        ch->getReligion( )->tattooFight( obj, ch );
}

