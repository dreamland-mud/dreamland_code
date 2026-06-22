/* $Id: impl.cpp,v 1.1.2.3.18.4 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"

#include "cmlt.h"
#include "remortnanny.h"
#include "so.h"

extern "C"
{
    SO::PluginList initialize_remort( ) {
        SO::PluginList ppl;

        Plugin::registerPlugin<CMlt>( ppl );
        Plugin::registerPlugin<RemortNanny>( ppl );

        return ppl;
    }
}
