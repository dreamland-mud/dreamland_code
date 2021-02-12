/* $Id: impl_raceaptitude.cpp,v 1.1.2.2 2008/02/24 17:25:48 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "mocregistrator.h"

#include "raceaptitude.h"


extern "C"
{
        SO::PluginList initialize_race_aptitude( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<MocRegistrator<RaceAptitude> >( ppl );
                
                return ppl;
        }
        
}
