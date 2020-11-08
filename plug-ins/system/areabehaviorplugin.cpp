/* $Id: areabehaviorplugin.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "areabehaviorplugin.h"
#include "character.h"
#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

void AreaBehaviorPlugin::initialization( ) {
    AreaIndexData *area;

    for (area = area_first; area; area = area->next) {
        if (!area->behavior)
            continue;
        
        if (area->behavior->getType( ) == getName( )) {
            area->behavior.recover( );
            area->behavior->setArea( area );        
        }
    }
}

void AreaBehaviorPlugin::destruction( ) {
    AreaIndexData *area;

    /* XXX */
    if (dreamland->isShutdown( ))
        return;

    for (area = area_first; area; area = area->next) {
        if (!area->behavior)
            continue;
        
        if (area->behavior->getType( ) == getName( )) {
            area->behavior->unsetArea( );
            area->behavior.backup( );
        }
    }
}

bool area_is_mansion(AreaIndexData *area)
{
    // TODO remove obsolete check after flag is set everywhere.
    if (!str_prefix("ht", area->area_file->file_name))
        return true;

    return IS_SET(area->area_flag, AREA_MANSION);
}

bool area_is_clan(AreaIndexData *area)
{
    // TODO remove obsolete check after flag is set everywhere.
    static const DLString CLAN_AREA_TYPE = DLString("ClanArea");
    if (area->behavior && CLAN_AREA_TYPE.strPrefix(area->behavior->getType()))
        return true;

    return IS_SET(area->area_flag, AREA_CLAN);
}

bool area_is_hometown(AreaIndexData *area)
{
    return IS_SET(area->area_flag, AREA_HOMETOWN);
}

bool area_has_levels(AreaIndexData *area)
{
    return area->high_range > 0 || area->low_range > 0;
}

DLString area_danger_long(AreaIndexData *area)
{
    bitstring_t flags = area->area_flag;

    if (IS_SET(flags, AREA_SAFE))
        return "{Cбезопасно{x";
    
    if (IS_SET(flags, AREA_EASY))
        return "{Gлёгкие противники{x";

    if (IS_SET(flags, AREA_HARD))
        return "{Yсложные противники{x";

    if (IS_SET(flags, AREA_DEADLY))
        return "{Rсмертельно опасно{x";

    return DLString::emptyString;
}

DLString area_danger_short(AreaIndexData *area)
{
    bitstring_t flags = area->area_flag;

    if (IS_SET(flags, AREA_SAFE))
        return "{Cмирная{x";
    
    if (IS_SET(flags, AREA_EASY))
        return "{Gлегко{x";

    if (IS_SET(flags, AREA_HARD))
        return "{Yсложно{x";

    if (IS_SET(flags, AREA_DEADLY))
        return "{Rопасно{x";

    return DLString::emptyString;
}
