/* Dream Land, 2026 */
#include "so.h"
#include "charmquest.h"

extern "C"
{
        SO::PluginList initialize_quest_charm( )
        {
                SO::PluginList ppl;

                // No per-type behavior registrators: the generic MobQuestTarget
                // from quest_core carries the mark, like every Fenia-era type.
                Plugin::registerPlugin<CharmQuestRegistrator>( ppl );

                return ppl;
        }
}
