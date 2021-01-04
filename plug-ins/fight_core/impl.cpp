/* $Id: impl.cpp,v 1.1.2.2 2008/04/04 21:29:02 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "plugin.h"
#include "weaponrandomizer.h"

extern "C"
{
    SO::PluginList initialize_fight_core( )
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<WeaponRandomizer>( ppl );
        return ppl;
    }
}
