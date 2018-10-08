/* $Id: impl.cpp,v 1.1.6.2.6.3 2009/08/10 01:06:51 rufina Exp $
 *
 * ruffina, 2004
 */
#include "class.h"
#include "so.h"
#include "plugin.h"

#include "clantypes.h"
#include "clantitles.h"
#include "clanorg.h"

class ClanCoreRegistrator : public Plugin {
public:
    typedef ::Pointer<ClanCoreRegistrator> Pointer;

    virtual void initialization( )
    {
        Class::regMoc<ClanData>( );
        Class::regMoc<ClanBank>( );
        Class::regMoc<ClanMembership>( );
        Class::regMoc<ClanOrder>( );
        Class::regMoc<ClanOrgs>( );
        Class::regXMLVar<ClanTitlesByClass>( );
        Class::regXMLVar<ClanTitlesByLevel>( );
    }

    virtual void destruction( )
    {
        Class::unregXMLVar<ClanTitlesByClass>( );
        Class::unregXMLVar<ClanTitlesByLevel>( );
        Class::unregMoc<ClanOrgs>( );
        Class::unregMoc<ClanOrder>( );
        Class::unregMoc<ClanMembership>( );
        Class::unregMoc<ClanBank>( );
        Class::unregMoc<ClanData>( );
    }
};

extern "C"
{
    SO::PluginList initialize_clan_core( )
    {
        SO::PluginList ppl;
    
        Plugin::registerPlugin<ClanCoreRegistrator>( ppl );

        return ppl;
    }
}
