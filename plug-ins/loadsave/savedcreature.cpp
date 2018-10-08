/* $Id$
 *
 * ruffina, 2004
 */
#include "savedcreature.h"
#include "save.h"

void SavedCreature::save( )
{
    save_creature( ch );
    saved = true;
}

bool SavedCreature::extract( bool fCount )
{
    if (saved) {
        unsave_creature( ch );
        saved = false;
    }

    return MobileBehavior::extract( fCount );
}

void SavedCreature::stopfol( Character *master )
{
    if (saved) {
        unsave_creature( ch );
        saved = false;
    }

    MobileBehavior::stopfol( master );
}

