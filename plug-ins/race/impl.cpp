/* $Id: impl.cpp,v 1.1.2.2.18.5 2008/05/27 21:30:05 rufina Exp $
 *
 * ruffina, 2006
 */

#include "so.h"
#include "mocregistrator.h"
#include "xmlvariableregistrator.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "defaultrace.h"
#include "defaultpcrace.h"
#include "defaultracelanguage.h"

TABLE_LOADER_IMPL(RaceLoader, "races", "Race");
TABLE_LOADER(RaceLanguageLoader, "race-languages", "RaceLanguage");

extern "C"
{
        SO::PluginList initialize_race( )
        {
            SO::PluginList ppl;

            Plugin::registerPlugin<XMLVariableRegistrator<RaceHelp> >( ppl );
            Plugin::registerPlugin<MocRegistrator<DefaultRace> >( ppl );
            Plugin::registerPlugin<MocRegistrator<DefaultPCRace> >( ppl );
            Plugin::registerPlugin<RaceLoader>( ppl );
            Plugin::registerPlugin<MocRegistrator<DefaultRaceLanguage> >( ppl );
            Plugin::registerPlugin<RaceLanguageLoader>( ppl );

            return ppl;
        }
        
}

