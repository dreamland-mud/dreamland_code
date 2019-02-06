/* $Id$
 *
 * ruffina, 2018
 */

#include "rpccommandmanager.h"
#include "logstream.h"

RpcCommandManager *RpcCommandManager::instance;

RpcCommandManager themanager; // XXX should the instance be created by Initializer?


void 
RpcCommand::initialization( )
{
    RpcCommandManager::getThis()->reg(this);
}

void 
RpcCommand::destruction( )
{
    RpcCommandManager::getThis()->unreg(this);
}


//-------------------------------------------------
// RpcCommandManager
//-------------------------------------------------

RpcCommandManager::RpcCommandManager()
{
    instance = this;
}

RpcCommandManager::~RpcCommandManager()
{
    instance = NULL;
}

void 
RpcCommandManager::run(Character *ch, const DLString &name, const std::vector<DLString> &args)
{
    RpcCommand *cmd = commands[name];
    if(cmd) {
        cmd->handle(ch, args);
    } else {
        LogStream::sendError() << "Unregistered RPC command: " << name << endl;
    }
}

void 
RpcCommandManager::reg(RpcCommand *cmd)
{
    commands[cmd->name] = cmd;
}

void 
RpcCommandManager::unreg(RpcCommand *cmd)
{
    commands[cmd->name] = NULL;
}

