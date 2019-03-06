/* $Id: stmt-tree.h,v 1.1.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: stmt-tree.h,v 1.1.2.6.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __STMT_TREE_H__
#define __STMT_TREE_H__

#include "nodes.h"
#include "scope.h"
#include "exceptions.h"
#include "function.h"
#include "codesource.h"

namespace Scripting {

/* Statements */

class BreakStmt : public StmtNode {
public:
    typedef ::Pointer<BreakStmt> Pointer;

    BreakStmt();
    virtual ~BreakStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
};

class CompoundStmt : public StmtNode {
public:
    typedef ::Pointer<CompoundStmt> Pointer;

    CompoundStmt();
    CompoundStmt(StmtNodeList::Pointer nl);
    virtual ~CompoundStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    StmtNodeList::Pointer nodelist;
};

class ContinueStmt : public StmtNode {
public:
    typedef ::Pointer<ContinueStmt> Pointer;

    ContinueStmt();
    virtual ~ContinueStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
};

class EmptyStmt : public StmtNode {
public:
    typedef ::Pointer<EmptyStmt> Pointer;
    
    EmptyStmt();
    virtual ~EmptyStmt();
    
    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;
};

class ExpStmt : public StmtNode {
public:
    typedef ::Pointer<ExpStmt> Pointer;

    ExpStmt();
    ExpStmt(ExpNode::Pointer e);
    virtual ~ExpStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNode::Pointer exp;
};

class TryCatchStmt : public StmtNode {
public:
    typedef ::Pointer<TryCatchStmt> Pointer;
    
    TryCatchStmt();
    TryCatchStmt( StmtNode::Pointer b, Lex::id_t v, StmtNode::Pointer h );
    virtual ~TryCatchStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

/*  try */
    StmtNode::Pointer body;
/*  catch( */
    Lex::id_t var;
/*  ) */
    StmtNode::Pointer handle;
};

class ForStmt : public StmtNode {
public:
    typedef ::Pointer<ForStmt> Pointer;
    
    ForStmt();
    ForStmt(ExpNodeList::Pointer i, 
            ExpNode::Pointer c, 
            ExpNodeList::Pointer n, 
            StmtNode::Pointer b);
    virtual ~ForStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    void evalInit();
    void evalNext();
    bool evalCond();

    void reverseInit(ostream &os, const DLString &nextline) const;
    void reverseCond(ostream &os, const DLString &nextline) const;
    void reverseNext(ostream &os, const DLString &nextline) const;

    ExpNodeList::Pointer init;
    ExpNodeList::Pointer next;
    ExpNode::Pointer cond;
    StmtNode::Pointer body;
};

class IfStmt : public StmtNode {
public:
    typedef ::Pointer<IfStmt> Pointer;
    
    IfStmt();
    IfStmt(ExpNode::Pointer c, StmtNode::Pointer t);
    IfStmt(ExpNode::Pointer c, StmtNode::Pointer t, StmtNode::Pointer e);
    virtual ~IfStmt();
    
    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNode::Pointer cond;
    StmtNode::Pointer then;
    StmtNode::Pointer elze;
};

class ReturnStmt : public StmtNode {
public:
    typedef ::Pointer<ReturnStmt> Pointer;
    
    ReturnStmt();
    ReturnStmt(ExpNode::Pointer e);
    virtual ~ReturnStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNode::Pointer exp;
};

class ThrowStmt : public StmtNode {
public:
    typedef ::Pointer<ThrowStmt> Pointer;
    
    ThrowStmt();
    ThrowStmt(ExpNode::Pointer e);
    virtual ~ThrowStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ExpNode::Pointer exp;
};

class VarStmt : public StmtNode {
public:
    typedef ::Pointer<VarStmt> Pointer;

    VarStmt();
    VarStmt(ArgNames::Pointer v);
    virtual ~VarStmt();

    virtual FlowCtl evalAux();
    virtual void reverse(ostream &os, const DLString &nextline) const;

    ArgNames::Pointer vars;
};

}

#endif /* __STMT_TREE_H__ */
