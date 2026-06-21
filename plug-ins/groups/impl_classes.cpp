/* $Id: impl_classes.cpp,v 1.1.6.6 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobilebehaviorplugin.h"
#include "objectbehaviorplugin.h"
#include "xmlattributeplugin.h"
#include "skillcommandtemplate.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "commandtemplate.h"

#include "so.h"

extern "C"
{
        SO::PluginList initialize_class_skills( )
        {
            SO::PluginList ppl;

            return ppl;
        }

}
