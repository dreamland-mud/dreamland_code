/* $Id: areabehaviorplugin.h,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef AREABEHAVIORPLUGIN_H
#define AREABEHAVIORPLUGIN_H

#include "plugin.h"
#include "class.h"

struct area_data;

class AreaBehaviorPlugin : public Plugin {
public:
    typedef ::Pointer<AreaBehaviorPlugin> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual const DLString& getName( ) const = 0;
};

template<typename C>
class AreaBehaviorRegistrator: public AreaBehaviorPlugin {
public:
    typedef ::Pointer< AreaBehaviorRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
        Class::regMoc<C>( );
        AreaBehaviorPlugin::initialization( );
    }
    virtual void destruction( ) 
    {
        AreaBehaviorPlugin::destruction( );
        Class::unregMoc<C>( );
    }
    virtual const DLString& getName( ) const 
    {
        return C::MOC_TYPE;
    }
};

/** Returns true if area is a suburb for private mansions. */
bool area_is_mansion(area_data *);

/** Returns true if area belongs to a clan. */
bool area_is_clan(area_data *);

/** Returns true if areas is a hometown. */
bool area_is_hometown(area_data *);

/** Returns true if area has a meaningful level range. */
bool area_has_levels(area_data *area);

/** Describe area danger level, with colors. */
DLString area_danger_long(area_data *area);

/** Describe area danger level as a single word, with colors. */
DLString area_danger_short(area_data *area);

#endif
