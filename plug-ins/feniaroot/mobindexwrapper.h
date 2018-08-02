/* $Id: mobindexwrapper.h,v 1.1.2.5.18.2 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef _MOBINDEXWRAPPER_H_
#define _MOBINDEXWRAPPER_H_

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
// MOC_SKIP_BEGIN
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

struct mob_index_data;

class MobIndexWrapper : public PluginWrapperImpl<MobIndexWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<MobIndexWrapper> Pointer;
    
    MobIndexWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( mob_index_data * );
    void checkTarget( ) const throw( Scripting::Exception );
    virtual void extract( bool );
    mob_index_data *getTarget( ) const;
private:        
    mob_index_data *target;
};

#endif 
