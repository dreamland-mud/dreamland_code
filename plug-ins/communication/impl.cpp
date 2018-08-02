/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributeplugin.h"
#include "commandtemplate.h"

#include "worldchannel.h"
#include "areachannel.h"
#include "racechannel.h"
#include "roomchannel.h"
#include "personalchannel.h"
#include "twitlist.h"
#include "channels.h"
#include "so.h"

class RegMocPlugin : public Plugin {
public:
    typedef ::Pointer<RegMocPlugin> Pointer;

    virtual void initialization( )
    {
	Class::regMoc<WorldChannel>( );
	Class::regMoc<AreaChannel>( );
	Class::regMoc<RaceChannel>( );
	Class::regMoc<RoomChannel>( );
	Class::regMoc<PersonalChannel>( );
    }

    virtual void destruction( )
    {
	Class::unregMoc<WorldChannel>( );
	Class::unregMoc<AreaChannel>( );
	Class::unregMoc<RaceChannel>( );
	Class::unregMoc<RoomChannel>( );
	Class::unregMoc<PersonalChannel>( );
    }
};

extern "C"
{
    SO::PluginList initialize_communication( )
    {
	SO::PluginList ppl;
    
	Plugin::registerPlugin<RegMocPlugin>( ppl );
	Plugin::registerPlugin<CTwit>( ppl );
	Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeTwitList> >( ppl );
        Plugin::registerPlugin<ChannelsCommand>( ppl );
	
	return ppl;
    }
}
