/* $Id: nodes.h,v 1.4.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: nodes.h,v 1.4.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __NODES_H__
#define __NODES_H__

#include <list>

#include <dlstring.h>
#include <xmlvariablecontainer.h>
#include <xmlvector.h>
#include <xmlpointer.h>

#include "codesourceref.h"

namespace Scripting {

class Scope;
class Register;
class Reference;
class FlowCtl;

/* abtract semantic node  */
class Node : public virtual DLObject {
public:
    Node();
    virtual ~Node( );

    virtual void reverse(ostream &os, const DLString &nextline) const = 0;
    CodeSourceRef source;
};

/* semantinc nodes by evaluation type */
class StmtNode : public Node {
public:
    typedef ::Pointer<StmtNode> Pointer;
    
    virtual ~StmtNode( );
    
    FlowCtl eval();
    virtual FlowCtl evalAux() = 0;
};

class ExpNode : public Node {
public:
    typedef ::Pointer<ExpNode> Pointer;
    
    virtual ~ExpNode( );
    
    Register eval();
    virtual Register evalAux() = 0;
};

class ReferenceNode : public Node {
public:
    typedef ::Pointer<ReferenceNode> Pointer;
    
    virtual ~ReferenceNode( );

    Reference eval();
    virtual Reference evalAux() = 0;
};

/* node lists */
// MOC_SKIP_BEGIN
class ExpNodeList : public vector<ExpNode::Pointer>, public virtual DLObject {
public:
    typedef ::Pointer<ExpNodeList> Pointer;

    ExpNodeList( );
    virtual ~ExpNodeList( );
};

class StmtNodeList : public vector<StmtNode::Pointer>, public virtual DLObject {
public:
    typedef ::Pointer<StmtNodeList> Pointer;

    StmtNodeList( );
    virtual ~StmtNodeList( );
};
// MOC_SKIP_END

}

#endif /* __NODES_H__ */
