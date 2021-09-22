/* $Id: plugin.h,v 1.12.2.2.18.4 2010-09-01 21:20:47 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 * based on idea by NoFate, 2002
 */
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "dlobject.h"

class Plugin : public virtual DLObject {
friend class SharedObject;    
public:

    typedef ::Pointer<Plugin> Pointer;    
    
    template<typename PlugType, typename PlugList>
    static inline void registerPlugin( PlugList& ppl )
    {
        ppl.push_back( static_cast<Plugin*>( new PlugType( ) ) );
    }

protected:
    virtual void initialization( ) = 0;
    virtual void destruction( ) = 0;

    /** 
      * Return 'true' for plugins that have serious side-effects when reloaded,
      * e.g. [descriptor] plugin that disconnects all players. 
      */
    virtual bool isCritical() const;
};

#endif
