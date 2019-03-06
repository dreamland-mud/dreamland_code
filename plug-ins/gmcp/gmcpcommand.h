/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBITEMMANIP_H
#define WEBITEMMANIP_H

#include "plugininitializer.h"
#include "outofband.h"
#include "classselfregistratorplugin.h"

#define INITPRIO_NORMAL 100


struct GMCPCommand : public OutOfBandCommand {
    bool isSupported(Descriptor *d) const;
    void checkSupport(Descriptor *d, const char *proto) const;

    static void send(Descriptor *d, const string &package, const string &message, const string &data);
};

template <const char *&tn>
class GMCPCommandTemplate : public GMCPCommand, public ClassSelfRegistratorPlugin<tn> {
public:
    typedef ::Pointer<GMCPCommandTemplate> Pointer;
    
    GMCPCommandTemplate( ) {
        name = cmdName;
    }

    virtual const DLString &getCommandType( ) const {
        return name;
    }

    virtual void run( const OutOfBandArgs &args ) const {
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType ) {
    }
    virtual bool toXML( XMLNode::Pointer& node ) const {
        return false;
    }
protected:
    virtual void initialization( ) 
    {
        ClassSelfRegistratorPlugin<tn>::initialization( );
        GMCPCommand::initialization( );
    }
    virtual void destruction( ) 
    {
        GMCPCommand::destruction( );
        ClassSelfRegistratorPlugin<tn>::destruction( );
    }
    
private:
    static const char *cmdName;
    DLString name;
};

#define GMCPCOMMAND_DUMMY(x)         dummy_ ##x## _TypeName
#define GMCPCOMMAND(x) GMCPCommandTemplate<GMCPCOMMAND_DUMMY(x)>

#define GMCPCOMMAND_DECL(x) \
const char * GMCPCOMMAND_DUMMY(x) =  #x; \
template<> const char *GMCPCOMMAND(x)::cmdName = #x; \
PluginInitializer<GMCPCOMMAND(x)> dummy_ ##x## _init(INITPRIO_NORMAL);

#define GMCPCOMMAND_RUN(x) \
GMCPCOMMAND_DECL(x) \
template <> void GMCPCOMMAND(x)::run( const OutOfBandArgs &args ) const 


#endif

