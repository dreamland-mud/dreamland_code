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
#include "wrongcommand.h"
#include "commandtemplate.h"

#include "banking.h"
#include "corder.h"
#include "configs.h"
#include "pcdeleteidle.h"
#include "run.h"
#include "whois.h"
#include "who.h"
#include "writing.h"
#include "eating.h"
#include "so.h"

extern "C"
{
        SO::PluginList initialize_comm( )
        {
                SO::PluginList ppl;
            
                Plugin::registerPlugin<RoomBehaviorRegistrator<BankRoom> >( ppl );
                Plugin::registerPlugin<ObjectBehaviorRegistrator<CreditCard> >( ppl );
                ppl.push_back( WrongCommand::Pointer( NEW, "balance" ) );
                ppl.push_back( WrongCommand::Pointer( NEW, "deposit" ) );
                ppl.push_back( WrongCommand::Pointer( NEW, "withdraw" ) );
                Plugin::registerPlugin<TaxesListener>( ppl );

                Plugin::registerPlugin<ConfigCommand>( ppl );

                Plugin::registerPlugin<SpeedWalkUpdateTask>( ppl );
                Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeSpeedWalk> >( ppl );

                Plugin::registerPlugin<COrder>( ppl );
                Plugin::registerPlugin<Whois>( ppl );
                Plugin::registerPlugin<Who>( ppl );
                Plugin::registerPlugin<CWrite>( ppl );
                
//                Plugin::registerPlugin<PCDeleteIdle>( ppl );

                Plugin::registerPlugin<CEat>( ppl );

                return ppl;
        }
}
