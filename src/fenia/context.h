/* $Id: context.h,v 1.1.2.5.18.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: context.h,v 1.1.2.5.18.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "register-decl.h"

namespace Scripting {

class Scope;
class Node;

class BackTrace;
class NodeTrace;
class Node;

class Context {
public:
    Context() : scope(0), backTrace(0), nodeTrace(0) { }
    
    Scope *scope;
    BackTrace *backTrace;
    NodeTrace *nodeTrace;

    static Context *current;
    static Register root;
};

struct NodeTrace {
    NodeTrace(Node *n) : node(n) {
        NodeTrace *&top = Context::current->nodeTrace;
        parent = top;
        top = this;
    }
    ~NodeTrace() {
        if (Context::current)
            Context::current->nodeTrace = parent;
    }
    
    Node *node;
    NodeTrace *parent;
};


struct BackTrace {
    BackTrace();
    virtual ~BackTrace();
    
    virtual void print(ostream &os) const = 0;
 
    static void report(ostream &os);
    
    BackTrace *parent;
};

inline ostream &
operator << (ostream &os, const BackTrace &bt) 
{
    bt.print(os);
    return os;
}

struct Handler;

struct BTPushNative : public BackTrace {
    BTPushNative(Handler *h, Lex::id_t i) : handler(h), id(i) { }
    
    virtual void print(ostream &os) const;

    Handler *handler;
    Lex::id_t id;
};

struct BTPushNode : public BackTrace {
    BTPushNode() {
        NodeTrace *nt = Context::current->nodeTrace;

        if(nt)
            node = nt->node;
        else
            node = 0;
    }
    
    virtual void print(ostream &os) const;

    Node *node;
};


}
#endif
