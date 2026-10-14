/* $Id: impl.cpp,v 1.1.2.1 2007/05/02 02:52:33 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "xmlattributeplugin.h"
#include "mobilebehaviorplugin.h"
#include "xmlvariableregistrator.h"
#include "mocregistrator.h"

#include "language.h"
#include "languagemanager.h"
#include "xmlattributelanguage.h"
#include "poliglot.h"
#include "wordeffect.h"


extern "C"
{
    SO::PluginList initialize_languages_core( ) 
    {
        SO::PluginList ppl;
                
        Plugin::registerPlugin<LanguageManager>( ppl );
        Plugin::registerPlugin<MocRegistrator<LanguageCommand> >( ppl );                
        // The plain base is the class every Fenia-owned effect node instantiates:
        // its run() is a no-op, .WordEffect(lang,name).runVict/runObj does the work.
        Plugin::registerPlugin<MocRegistrator<WordEffect> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLanguage> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLanguageHints> >( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<LanguageHelp> >( ppl );
        
        Plugin::registerPlugin<MobileBehaviorRegistrator<Poliglot> >( ppl );

        return ppl;
    }
}

