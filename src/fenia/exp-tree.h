/* $Id: exp-tree.h,v 1.1.2.6.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: exp-tree.h,v 1.1.2.6.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __EXP_TREE_H__
#define __EXP_TREE_H__

#include "nodes.h"
#include "scope.h"
#include "xmlregister.h"
#include "codesource.h"

namespace Scripting {


/* Expressions */
class ListExp : public ExpNode {
public:
    typedef ::Pointer<ListExp> Pointer;

    ListExp();
    ListExp(ExpNodeList::Pointer nl);
    virtual ~ListExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNodeList::Pointer nodelist;
};

class DerefExp : public ExpNode {
public:
    typedef ::Pointer<DerefExp> Pointer;
    
    DerefExp();
    DerefExp(ReferenceNode::Pointer r);
    virtual ~DerefExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ReferenceNode::Pointer ref;
};

class ConstantExp : public ExpNode {
public:
    typedef ::Pointer<ConstantExp> Pointer;
    
    ConstantExp();
    ConstantExp(Register o);
    virtual ~ConstantExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    Register object;
};

class ClosureExp : public ExpNode {
public:
    typedef ::Pointer<ClosureExp> Pointer;
    
    ClosureExp(Function *f);
    virtual ~ClosureExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    Function *function;
};

class LambdaExp : public ClosureExp {
public:
    typedef ::Pointer<LambdaExp> Pointer;

    LambdaExp(Function *f);

    virtual Register evalAux();
};


class CallExp : public ExpNode {
public:
    typedef ::Pointer<CallExp> Pointer;
    
    CallExp();
    CallExp(ReferenceNode::Pointer r, ExpNodeList::Pointer a );
    virtual ~CallExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
    
    ReferenceNode::Pointer ref;
    ExpNodeList::Pointer args;
};

class OpExp : public ExpNode {
public:
    typedef ::Pointer<OpExp> Pointer;

    typedef const Register (Register::*BinOp)(const Register &) const;
    typedef const Register (Register::*UnOp)() const;
    
    OpExp( );
    OpExp(const char *n, ExpNode::Pointer l, ExpNode::Pointer r);
    OpExp(const char *n, ExpNode::Pointer e);
    virtual ~OpExp( );

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    void lookup();

    ExpNode::Pointer left;
    ExpNode::Pointer right;

    DLString name;
    
    union {
        BinOp bin;
        UnOp un;
    } op;
};

class OrExp : public ExpNode {
public:
    typedef ::Pointer<OrExp> Pointer;
    
    OrExp();
    OrExp(ExpNode::Pointer l, ExpNode::Pointer r);
    virtual ~OrExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNode::Pointer left;
    ExpNode::Pointer right;
};

class AndExp : public ExpNode {
public:
    typedef ::Pointer<AndExp> Pointer;
    
    AndExp();
    AndExp(ExpNode::Pointer l, ExpNode::Pointer r);
    virtual ~AndExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNode::Pointer left;
    ExpNode::Pointer right;
};

class AssignExp : public ExpNode {
public:
    typedef ::Pointer<AssignExp> Pointer;
    
    AssignExp();
    AssignExp(ReferenceNode::Pointer l, ExpNode::Pointer r);
    virtual ~AssignExp();

    virtual Register evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ReferenceNode::Pointer left;
    ExpNode::Pointer right;
};

}

#endif /* __EXP_TREE_H__ */
