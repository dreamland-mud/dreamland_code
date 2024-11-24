/* $Id: objectwrapper.h,v 1.1.4.10.6.3 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef _OBJECTWRAPPER_H_
#define _OBJECTWRAPPER_H_

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "xmlregister.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "pluginwrapperimpl.h"

class Object;

class ObjectWrapper : public PluginWrapperImpl<ObjectWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<ObjectWrapper> Pointer;

    ObjectWrapper( );

    virtual void setSelf( Scripting::Object * );
    virtual void extract( bool );
    virtual bool targetExists() const;

    void setTarget( ::Object * );
    void checkTarget( ) const ;
    ::Object * getTarget() const;
    
private:
    ::Object *target;
};

#endif 
