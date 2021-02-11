#ifndef AFFECTHANDLERWRAPPER_H
#define AFFECTHANDLERWRAPPER_H

#include "xmlvariablecontainer.h"
// MOC_SKIP_BEGIN
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"

struct AffectHandler;

class AffectHandlerWrapper : public PluginWrapperImpl<AffectHandlerWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<AffectHandlerWrapper> Pointer;
    
    AffectHandlerWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( AffectHandler* );
    void checkTarget( ) const ;
    virtual void extract( bool );
    AffectHandler *getTarget( ) const;
private:        
    AffectHandler *target;
};

#endif 
