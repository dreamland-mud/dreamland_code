/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBITEMMANIP_H
#define WEBITEMMANIP_H

#include "plugininitializer.h"
#include "webmanip.h"
#include "classselfregistratorplugin.h"

#define INITPRIO_NORMAL 100

template <const char *&tn>
class WebManipCommandTemplate : public WebManipCommand, public ClassSelfRegistratorPlugin<tn> {
public:
    typedef ::Pointer<WebManipCommandTemplate> Pointer;
    
    WebManipCommandTemplate( ) {
        name = cmdName;
    }

    virtual const DLString &getName( ) const {
        return name;
    }

    virtual bool run( ostringstream &buf, const ManipCommandArgs &args ) {
        return false;
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
    virtual void fromXML( const XMLNode::Pointer& node )  {
    }
    virtual bool toXML( XMLNode::Pointer& node ) const {
        return false;
    }
protected:
    virtual void initialization( ) 
    {
        ClassSelfRegistratorPlugin<tn>::initialization( );
        WebManipCommand::initialization( );
    }
    virtual void destruction( ) 
    {
        WebManipCommand::destruction( );
        ClassSelfRegistratorPlugin<tn>::destruction( );
    }
    
private:
    static const char *cmdName;
    DLString name;
};

#define WEBMANIP_DUMMY(x)         dummy_ ##x## _TypeName
#define WEBMANIP(x) WebManipCommandTemplate<WEBMANIP_DUMMY(x)>

#define WEBMANIP_DECL(x) \
const char * WEBMANIP_DUMMY(x) =  #x; \
template<> const char *WEBMANIP(x)::cmdName = #x; \
PluginInitializer<WEBMANIP(x)> dummy_ ##x## _init(INITPRIO_NORMAL);

#define WEBMANIP_RUN(x) \
WEBMANIP_DECL(x) \
template <> bool WEBMANIP(x)::run( ostringstream &buf, const ManipCommandArgs &args ) 


#endif
