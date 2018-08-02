/* $Id: nodes.cpp,v 1.4.6.3.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: nodes.cpp,v 1.4.6.3.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include "nodes.h"
#include "flow.h"
#include "context.h"
#include "reference-impl.h"

namespace Scripting {

Node::Node()
{
}

Node::~Node()
{
}

StmtNode::~StmtNode( )
{
}

ExpNode::~ExpNode( )
{
}

ReferenceNode::~ReferenceNode( )
{
}

ExpNodeList::ExpNodeList()
{
}

ExpNodeList::~ExpNodeList()
{
}

StmtNodeList::StmtNodeList()
{
}

StmtNodeList::~StmtNodeList()
{
}

FlowCtl 
StmtNode::eval() 
{
    NodeTrace dummy(this);
    return evalAux();
}

Register 
ExpNode::eval()
{
    NodeTrace dummy(this);
    return evalAux();
}

Reference 
ReferenceNode::eval()
{
    NodeTrace dummy(this);
    return evalAux();
}


}
