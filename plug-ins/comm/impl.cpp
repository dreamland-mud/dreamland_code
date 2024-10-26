/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          impl.cpp  -  description
                             -------------------
    begin                : Fri Apr 13 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "xmlattributeplugin.h"
#include "objectbehaviorplugin.h"
#include "roombehaviorplugin.h"
#include "commandtemplate.h"

#include "groupchannel.h"
#include "corder.h"
#include "configs.h"
#include "run.h"
#include "writing.h"
#include "eating.h"
#include "so.h"

extern "C"
{
        SO::PluginList initialize_comm( )
        {
                SO::PluginList ppl;
            
                Plugin::registerPlugin<ConfigCommand>( ppl );

                Plugin::registerPlugin<SpeedWalkUpdateTask>( ppl );
                Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeSpeedWalk> >( ppl );

                Plugin::registerPlugin<COrder>( ppl );
                Plugin::registerPlugin<CWrite>( ppl );
                
                Plugin::registerPlugin<CEat>( ppl );

        	Plugin::registerPlugin<GroupChannel>( ppl );
                return ppl;
        }
}
