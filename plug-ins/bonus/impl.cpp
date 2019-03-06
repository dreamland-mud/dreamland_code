/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "xmlvariableregistrator.h"
#include "mobilebehaviorplugin.h"
#include "mocregistrator.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "xmlattributeplugin.h"

#include "defaultbonus.h"

TABLE_LOADER(BonusLoader, "bonuses", "Bonus");

extern "C"
{
    SO::PluginList initialize_bonus( ) 
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<MocRegistrator<DefaultBonus> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ExperienceBonus> >( ppl );
        
        Plugin::registerPlugin<BonusLoader>( ppl );

        return ppl;
    }
}

