/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          system_impl.cpp  -  description
                             -------------------
    begin                : Mon Oct 22 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "so.h"
#include "xmlattributeplugin.h"
#include "xmlvariableregistrator.h"

#include "schedulertaskattributemanager.h"
#include "commonattributes.h"
#include "xmlpcpredicates.h"
#include "xmlattributecoder.h"
#include "xmlattributetrust.h"

extern "C"
{
        SO::PluginList initialize_system( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<SchedulerTaskAttributeManager>( ppl );
                Plugin::registerPlugin<ScheduledPCMemoryAttributeManager>( ppl );

                Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLEmptyAttribute> >( ppl );
                Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLStringAttribute> >( ppl );
                Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLIntegerAttribute> >( ppl );
                Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLStringListAttribute> >( ppl );
                
                Plugin::registerPlugin<XMLVariableRegistrator<XMLPCClassPredicate> >( ppl );
                Plugin::registerPlugin<XMLVariableRegistrator<XMLPCRacePredicate> >( ppl );
                Plugin::registerPlugin<XMLVariableRegistrator<XMLPCAlignPredicate> >( ppl );
                Plugin::registerPlugin<XMLVariableRegistrator<XMLPCEthosPredicate> >( ppl );
                Plugin::registerPlugin<XMLVariableRegistrator<XMLPCSexPredicate> >( ppl );

                Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCoder> >( ppl );
                Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeTrust> >( ppl );

                return ppl;
        }
}
