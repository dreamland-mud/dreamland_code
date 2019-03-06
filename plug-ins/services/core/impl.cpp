/* $Id: impl.cpp,v 1.1.4.1 2005/07/30 14:50:09 rufina Exp $
 *
 * ruffina, 2005
 */

#include "so.h"
#include "plugin.h"
#include "class.h"

#include "price.h"

class RegMocPlugin : public Plugin {
public:
    typedef ::Pointer<RegMocPlugin> Pointer;

    virtual void initialization( )
    {
        Class::regMoc<CoinPrice>( );
        Class::regMoc<LevelPrice>( );
        Class::regMoc<QuestPointPrice>( );
    }

    virtual void destruction( )
    {
        Class::unregMoc<CoinPrice>( );
        Class::unregMoc<LevelPrice>( );
        Class::unregMoc<QuestPointPrice>( );
    }
};

extern "C"
{
    SO::PluginList initialize_services_core( )
    {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<RegMocPlugin>( ppl );
            
        return ppl;
    }
}
