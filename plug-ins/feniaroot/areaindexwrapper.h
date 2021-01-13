#ifndef AREAINDEXWRAPPER_H
#define AREAINDEXWRAPPER_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
// MOC_SKIP_BEGIN
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

struct AreaIndexData;

class AreaIndexWrapper : public PluginWrapperImpl<AreaIndexWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<AreaIndexWrapper> Pointer;
    
    AreaIndexWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( AreaIndexData * );
    void checkTarget( ) const ;
    virtual void extract( bool );
    AreaIndexData *getTarget( ) const;
private:        
    AreaIndexData *target;
};

#endif 
