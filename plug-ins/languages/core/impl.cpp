/* $Id: impl.cpp,v 1.1.2.1 2007/05/02 02:52:33 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "xmlattributeplugin.h"
#include "mobilebehaviorplugin.h"
#include "xmlvariableregistrator.h"

#include "language.h"
#include "languagemanager.h"
#include "xmlattributelanguage.h"
#include "poliglot.h"


extern "C"
{
    SO::PluginList initialize_languages_core( ) 
    {
        SO::PluginList ppl;
                
        Plugin::registerPlugin<LanguageManager>( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLanguage> >( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLanguageHints> >( ppl );
        Plugin::registerPlugin<XMLVariableRegistrator<LanguageHelp> >( ppl );
        
        Plugin::registerPlugin<MobileBehaviorRegistrator<Poliglot> >( ppl );

        return ppl;
    }
}

