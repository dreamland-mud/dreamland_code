/* $Id: impl.cpp,v 1.1.2.5.6.3 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "so.h"
#include "xmlattributeplugin.h"
#include "mobilebehaviorplugin.h"
#include "commandtemplate.h"

#include "train.h"
#include "practice.h"
#include "teach.h"

extern "C"
{
    SO::PluginList initialize_learn( )
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<MobileBehaviorRegistrator<Trainer> >( ppl );
        
        Plugin::registerPlugin<CPractice>( ppl );

        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeTeacher> >( ppl );
        
        return ppl;
    }
}
