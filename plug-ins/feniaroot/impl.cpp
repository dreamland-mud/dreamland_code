/* $Id: impl.cpp,v 1.1.4.6.6.3 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "wrappermanager.h"
#include "wrappersplugin.h"
#include "feniacroaker.h"
#include "xmlattributecodesource.h"
#include "xmlvariableregistrator.h"
#include "commandtableloader.h"
#include "behaviorloader.h"
#include "mocregistrator.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_feniaroot( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<WrapperManager>( ppl );
        Plugin::registerPlugin<WrappersPlugin>( ppl );
        Plugin::registerPlugin<FeniaCroaker>(ppl);
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCodeSource> >( ppl );
        Plugin::registerPlugin<CommandTableLoader>(ppl);
        Plugin::registerPlugin<XMLVariableRegistrator<BehaviorHelp> >( ppl );
        Plugin::registerPlugin<MocRegistrator<DefaultBehavior> >(ppl);
        Plugin::registerPlugin<BehaviorLoader>(ppl);
        
        return ppl;
    }
}


