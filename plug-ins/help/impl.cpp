/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "plugin.h"
#include "mocregistrator.h"
#include "xmlvariableregistrator.h"
#include "bugtracker.h"
#include "helpcontainer.h"
#include "markuphelparticle.h"
#include "xmltableloaderplugin.h"
#include "json/json.h"
#include "iconvmap.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "dreamland.h"
#include "autoflags.h"
#include "def.h"

TABLE_LOADER_IMPL(HelpLoader, "helps", "Help");

static IconvMap koi2utf("koi8-r", "utf-8");


/**
 * Save a JSON file with all keywords and unique ID, to be used inside hedit.
 */
void help_save_ids() 
{
    if (dreamland->hasOption(DL_BUILDPLOT))
        return;

    Json::Value typeahead;

    for (auto &a: helpManager->getArticles()) {
        if (a->labels.all.count("social") > 0)
            continue;
            
        Json::Value b;
        b["kw"] = koi2utf((*a)->getAllKeywordsString());
        b["id"] = DLString((*a)->getID());
        typeahead.append(b);
    }

    Json::FastWriter writer;
    DLFileStream("/tmp", "hedit", ".json").fromString(
        writer.write(typeahead)
    );
}

extern "C" {
    
    SO::PluginList initialize_help( ) 
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<BugTracker>( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<GenericHelp> >( ppl );
        Plugin::registerPlugin<MocRegistrator<HelpContainer> >( ppl );                
        Plugin::registerPlugin<HelpLoader>( ppl );

        return ppl;
    }
}


