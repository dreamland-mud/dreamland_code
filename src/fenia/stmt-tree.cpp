/* $Id: stmt-tree.cpp,v 1.1.2.7.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: stmt-tree.cpp,v 1.1.2.7.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include "flow.h"
#include "stmt-tree.h"
#include "register-impl.h"

namespace Scripting {

/* Statements */
    
BreakStmt::BreakStmt() 
{
}

BreakStmt::~BreakStmt() 
{
}

FlowCtl
BreakStmt::evalAux()
{
    return FlowCtl(FlowCtl::BREAK);
}

void 
BreakStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << "{Ybreak{x; ";
}

CompoundStmt::CompoundStmt()
{
}

CompoundStmt::~CompoundStmt()
{
}

CompoundStmt::CompoundStmt(StmtNodeList::Pointer nl) : nodelist(nl) 
{
}

FlowCtl
CompoundStmt::evalAux() 
{
    CppScopeClobber scope;

    for(StmtNodeList::iterator i = nodelist->begin();i != nodelist->end(); i++) {
	FlowCtl fc = (*i)->eval();

	if(fc.type != FlowCtl::NEXT)
	    return fc;
    }
    return FlowCtl(FlowCtl::NEXT);
}

void 
CompoundStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << "{{";

    for(StmtNodeList::const_iterator i = nodelist->begin();i != nodelist->end(); i++)
	(*i)->reverse(os, nextline);
    
    if(nextline.length() > 4)
	os << string(nextline.c_str( ), nextline.length() - 4);
    
    os << "} ";
}
           
ContinueStmt::ContinueStmt()
{
}

ContinueStmt::~ContinueStmt()
{
}

FlowCtl
ContinueStmt::evalAux() 
{
    return FlowCtl(FlowCtl::CONTINUE);
}

void 
ContinueStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << "{Ycontinue{x; ";
}

EmptyStmt::EmptyStmt() 
{
}
    
EmptyStmt::~EmptyStmt() 
{
}
    
FlowCtl
EmptyStmt::evalAux() 
{
    return FlowCtl(FlowCtl::NEXT);
}

void 
EmptyStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << "; ";
}


ExpStmt::ExpStmt( )
{
}

ExpStmt::~ExpStmt( )
{
}

ExpStmt::ExpStmt(ExpNode::Pointer e) : exp(e) 
{
}

FlowCtl
ExpStmt::evalAux() 
{
    exp->eval();			    /*ignore return value*/
    return FlowCtl(FlowCtl::NEXT);
}

void 
ExpStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline;
    exp->reverse(os, nextline);
    os << "; ";
}

ForStmt::ForStmt( ) 
{
}

ForStmt::~ForStmt( ) 
{
}

ForStmt::ForStmt(ExpNodeList::Pointer i, 
	ExpNode::Pointer c, 
	ExpNodeList::Pointer n,
	StmtNode::Pointer b) : init(i), next(n), cond(c), body(b) 
{ 
}

void
ForStmt::evalInit()
{
    if(init) {
	ExpNodeList::iterator i;

	for(i = init->begin();i != init->end(); i++)
	    (*i)->eval(); // ignore return values
    }
}

bool
ForStmt::evalCond()
{
    if(cond && !cond->eval().toBoolean())
	return false;

    return true;
}

void
ForStmt::evalNext()
{
    if(next) {
	ExpNodeList::iterator i;

	for(i = next->begin();i != next->end(); i++)
	    (*i)->eval(); // ignore return values
    }
}

FlowCtl
ForStmt::evalAux() 
{
    CppScopeClobber scope;
    
    for(evalInit();evalCond();evalNext()) {
	FlowCtl fc = body->eval();
	
	if(fc.type == FlowCtl::RETURN)
	    return fc;

	if(fc.type == FlowCtl::BREAK)
	    break;
	
	/* next & continue flow controll is ok */
    }

    return FlowCtl(FlowCtl::NEXT);
}

void 
ForStmt::reverseInit(ostream &os, const DLString &nextline) const
{
    if(init) {
	ExpNodeList::const_iterator i;

	for(i = init->begin();i != init->end(); i++) {
	    if(i != init->begin())
		os << ", ";
	    (*i)->reverse(os, nextline);
	}
    }
}

void 
ForStmt::reverseCond(ostream &os, const DLString &nextline) const
{
    if(cond)
	cond->reverse(os, nextline);
}

void 
ForStmt::reverseNext(ostream &os, const DLString &nextline) const
{
    if(next) {
	ExpNodeList::const_iterator i;

	for(i = next->begin();i != next->end(); i++) {
	    if(i != next->begin())
		os << ", ";
		
	    (*i)->reverse(os, nextline);
	}
    }
}

void 
ForStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << nextline << "{Yfor{x (";

    reverseInit(os, nextline);
    os << "; ";
    reverseCond(os, nextline);
    os << "; ";
    reverseNext(os, nextline);
    os << ") ";
    body->reverse(os, nextline + "    ");
}


IfStmt::IfStmt() 
{
}

IfStmt::~IfStmt() 
{
}

IfStmt::IfStmt(ExpNode::Pointer c, StmtNode::Pointer t) 
    : cond(c), then(t), elze() 
{
}

IfStmt::IfStmt(ExpNode::Pointer c, StmtNode::Pointer t, StmtNode::Pointer e) 
    : cond(c), then(t), elze(e) 
{
}

FlowCtl
IfStmt::evalAux() 
{
    CppScopeClobber scope;
    
    if(cond->eval().toBoolean())
	return then->eval();
    else if(elze)
	return elze->eval();
    else
	return FlowCtl(FlowCtl(FlowCtl::NEXT));
}

void 
IfStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << nextline << "{Yif{x (";
    cond->reverse(os, nextline);
    os << ") ";
    then->reverse(os, nextline + "    ");
    
    if(elze) {
	os << nextline << "{Yelse{x ";
	elze->reverse(os, nextline + "    ");
    }
}


ThrowStmt::ThrowStmt() : exp() 
{
}

ThrowStmt::~ThrowStmt()
{
}

ThrowStmt::ThrowStmt(ExpNode::Pointer e) : exp(e) 
{
}

FlowCtl
ThrowStmt::evalAux() 
{
    throw CustomException( exp->eval().toString( ) );
}

void 
ThrowStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << "{Ythrow{x ";
    exp->reverse(os, nextline);
    os << ";";
}

ReturnStmt::ReturnStmt() : exp() 
{
}

ReturnStmt::~ReturnStmt()
{
}

ReturnStmt::ReturnStmt(ExpNode::Pointer e) : exp(e) 
{
}

FlowCtl
ReturnStmt::evalAux() 
{
    return FlowCtl(FlowCtl::RETURN, exp ? exp->eval() : Register());
}

void 
ReturnStmt::reverse(ostream &os, const DLString &nextline) const
{
    if(exp) {
	os << nextline << "{Yreturn{x ";
	exp->reverse(os, nextline);
	os << ";";
    } else
	os << nextline << "{Yreturn{x;";
}

VarStmt::VarStmt()
{
}

VarStmt::~VarStmt()
{
}

VarStmt::VarStmt(ArgNames::Pointer v) : vars(v)
{
}

FlowCtl
VarStmt::evalAux()
{
    ArgNames::const_iterator i;
    
    for(i = vars->begin();i != vars->end(); i++)
	Context::current->scope->addVar(*i);

    return FlowCtl(FlowCtl::NEXT);
}

void 
VarStmt::reverse(ostream &os, const DLString &nextline) const
{
    ArgNames::const_iterator i;
    os << nextline << "{Gvar{x ";
    
    for(i = vars->begin();i != vars->end(); i++) {
	if(i != vars->begin())
	    os << ", ";

	os << Lex::getThis()->getName(*i);
    }

    os << ';' << nextline;
}


TryCatchStmt::TryCatchStmt( )
{
}

TryCatchStmt::~TryCatchStmt( )
{
}

TryCatchStmt::TryCatchStmt( StmtNode::Pointer b, Lex::id_t v, StmtNode::Pointer h )
		: body(b), var(v), handle(h)
{
}

FlowCtl
TryCatchStmt::evalAux( )
{    
    try {
	return body->eval( );

    } catch(CustomException ce) {
	CppScopeClobber scope;
	
	scope.addVar(var);
	scope.setVar(var,  Register( ce.message ) );

	return handle->eval( );
    } catch(::Exception e) {
	CppScopeClobber scope;
	
	scope.addVar(var);
	scope.setVar(var,  Register( e.what( ) ) );

	return handle->eval( );
    }
}

void 
TryCatchStmt::reverse(ostream &os, const DLString &nextline) const
{
    os << nextline << nextline << "{Ytry{x ";
    
    body->reverse(os, nextline + "    ");
    os << nextline << "{Ycatch{x (" << Lex::getThis()->getName(var) << ") ";
    handle->reverse(os, nextline + "    ");
}

}
