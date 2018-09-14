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

#include "commandtemplate.h"

#include "nochannel.h"
#include "nopost.h"
#include "confirm.h"
#include "deny.h"
#include "cban.h"
#include "reward.h"
#include "so.h"

extern "C"
{
	SO::PluginList initialize_admin( )
	{
		SO::PluginList ppl;

		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeNoChannel> >( ppl );

		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeNoPost> >( ppl );

		Plugin::registerPlugin<Confirm>( ppl );
		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeConfirm> >( ppl );
		Plugin::registerPlugin<XMLAttributeConfirmListenerPlugin>( ppl );
		
		Plugin::registerPlugin<Deny>( ppl );
		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeDeny> >( ppl );
		
		Plugin::registerPlugin<CBan>( ppl );
		
		
		Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeGodReward> >( ppl );
		Plugin::registerPlugin<XMLAttributeGodRewardListenerPlugin>( ppl );
		
		return ppl;
	}
}
