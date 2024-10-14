/* $Id$
 *
 * ruffina, 2004
 */
#include "tattoo.h"

#include "core/object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "merc.h"
#include "wearloc_utils.h"
#include "interp.h"
#include "loadsave.h"
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

