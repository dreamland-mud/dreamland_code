/* $Id$
 *
 * ruffina, 2018
 */

#ifndef __RPCCOMMANDMANAGER_H__
#define __RPCCOMMANDMANAGER_H__

#include <map>
#include <vector>

#include "plugin.h"
#include "plugininitializer.h"

class Character;

class RpcCommand : public Plugin {
    friend class RpcCommandManager;
public:
    virtual void handle(Character *ch, const std::vector<DLString> &args) = 0;

protected:
    virtual void initialization( );
    virtual void destruction( );

    DLString name;
};

template <const char *&tn>
class RpcCommandTemplate : public RpcCommand {
public:
    RpcCommandTemplate() {
        name = tn;
    }

    virtual void handle(Character *ch, const std::vector<DLString> &args);
};

class RpcCommandManager
{
    friend class RpcCommand;
public:
    typedef std::map<DLString, RpcCommand*> Commands;

    RpcCommandManager();
    virtual ~RpcCommandManager();

    void run(Character *ch, const DLString &name, const std::vector<DLString> &args);
    
    static inline RpcCommandManager *getThis() {
        return instance;
    }

protected:
    void reg(RpcCommand *);
    void unreg(RpcCommand *);

private:
    Commands commands;
    static RpcCommandManager *instance;
};

#define RPC_DUMMY(x) dummyRpc_ ##x## _TypeName
#define RPC(x) RpcCommandTemplate<RPC_DUMMY(x)>

#define RPC_DECL(x) \
const char *RPC_DUMMY(x) = #x; \
PluginInitializer<RPC(x)> dummyRpc_ ##x## _init(52); // XXX use INITPRIO_XXX

#define RPCRUN(x) \
RPC_DECL(x) \
template <> void RPC(x)::handle(Character* ch, const std::vector<DLString> &args) 


#endif
