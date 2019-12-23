/* $Id: objindexwrapper.h,v 1.1.2.6.18.2 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef _OBJINDEXWRAPPER_H_
#define _OBJINDEXWRAPPER_H_

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
// MOC_SKIP_BEGIN
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

struct obj_index_data;

class ObjIndexWrapper : public PluginWrapperImpl<ObjIndexWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<ObjIndexWrapper> Pointer;
    
    ObjIndexWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( obj_index_data * );
    void checkTarget( ) const ;
    virtual void extract( bool );
    obj_index_data *getTarget( ) const;
private:        
    obj_index_data *target;
};

#endif 
