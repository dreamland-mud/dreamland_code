/* $Id: exp-tree.cpp,v 1.1.2.6.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: exp-tree.cpp,v 1.1.2.6.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <iostream>

using namespace std;

#include "exp-tree.h"
#include "reference-impl.h"
#include "register-impl.h"
#include "logstream.h"

namespace Scripting {

/* Expressions */

ListExp::ListExp() 
{
}

ListExp::~ListExp() 
{
}

/* (<exp>, <exp>, ...) */
ListExp::ListExp(ExpNodeList::Pointer nl) : nodelist(nl) 
{
}

Register
ListExp::evalAux() 
{
    Register rc;

    for(ExpNodeList::iterator i = nodelist->begin();i != nodelist->end(); i++)
        rc = (*i)->eval();

    return rc;
}

void 
ListExp::reverse(ostream &os, const DLString &nextline) const
{
    os << "(";
    
    for(ExpNodeList::const_iterator i = nodelist->begin();i != nodelist->end(); i++) {
        if(i != nodelist->begin())
            os << ", ";

        (*i)->reverse(os, nextline + "    ");
    }

    os << ")";
}


DerefExp::DerefExp( ) 
{
}

DerefExp::~DerefExp( ) 
{
}

/* <reference> */
DerefExp::DerefExp(ReferenceNode::Pointer r) : ref(r) 
{
}

Register
DerefExp::evalAux() 
{
    return *ref->eval();
}

void 
DerefExp::reverse(ostream &os, const DLString &nextline) const
{
    ref->reverse(os, nextline);
}



ConstantExp::ConstantExp( )
{
}
    
ConstantExp::~ConstantExp( )
{
}
    
/* <constant> */
ConstantExp::ConstantExp(Register o) : object(o) 
{
}

Register
ConstantExp::evalAux() 
{
    return object;
}

void 
ConstantExp::reverse(ostream &os, const DLString &nextline) const
{
    os << "{M";
    
    switch (object.type) {
    case Register::NONE:
        os << "null";
        break;
    case Register::NUMBER:
        os << object.value.number;
        break;
    case Register::STRING:
        os << '"' 
           << (*object.strPtr( ))
                    .substitute('{', "{{")
                    .substitute('"', "{R\\\"{M")
                    .substitute('\n', "{R\\n{M")
                    .substitute('\r', "{R\\r{M")
                    .substitute('\t', "{R\\t{M")
           << '"';
        break;
    case Register::FUNCTION:
        object.value.function->reverse(os, nextline);
        break;
    default:
        os << object.toString( );
    }

    os << "{x";    
}


ClosureExp::ClosureExp(Function *f) : function(f) 
{
    function->link();
}

ClosureExp::~ClosureExp( )
{
    function->unlink();
}

Register
ClosureExp::evalAux() 
{
    return Register(new Closure(NULL, function));
}

void 
ClosureExp::reverse(ostream &os, const DLString &nextline) const
{
    os << "{M";
    
    function->reverse(os, nextline);

    os << "{x";    
}

LambdaExp::LambdaExp(Function *f) : ClosureExp(f)
{
}

Register
LambdaExp::evalAux() 
{
    return Register(new Closure(Context::current->scope, function));
}

CallExp::CallExp( ) 
{
}

CallExp::~CallExp( ) 
{
}

/* <reference>(<exp>, <exp>, ...) */
CallExp::CallExp(ReferenceNode::Pointer r, ExpNodeList::Pointer a ) 
    : ref(r), args(a) 
{
}

Register
CallExp::evalAux() 
{
    RegisterList rl;

    for(ExpNodeList::iterator i = args->begin();i != args->end(); i++)
        rl.push_back( (*i)->eval() );
    
    return ref->eval()(rl);
}

void 
CallExp::reverse(ostream &os, const DLString &nextline) const
{
    ref->reverse(os, nextline);
    
    os << "(";
    
    for(ExpNodeList::const_iterator i = args->begin();i != args->end(); i++) {
        if(i != args->begin())
            os << ", ";
        
        (*i)->reverse(os, nextline);
    }

    os << ")";
}



OpExp::OpExp( ) 
{
}

OpExp::~OpExp( ) 
{
}

/* <exp> binop <exp> */
OpExp::OpExp(const char *n, ExpNode::Pointer l, ExpNode::Pointer r) 
{
    name = n;
    left = l;
    right = r;
    lookup();
}

/* unop <exp> */
OpExp::OpExp( const char *n, ExpNode::Pointer e) 
{
    name = n;
    left.clear();
    right = e;
    lookup();
}

Register
OpExp::evalAux() 
{
    if(left)
        return ( (left->eval()) .* (op.bin) ) (right->eval()) ;
    else
        return ( (right->eval()) .* (op.un) ) ();
}

void 
OpExp::reverse(ostream &os, const DLString &nextline) const
{
    if(left) {
        left->reverse(os, nextline);
    
        os << " ";
    }
    
    os << "{C" << name << "{x ";

    right->reverse(os, nextline);
}

void
OpExp::lookup()
{
    if(left) {
#define DEFOP(x) if(name == #x) { op.bin = &Register::operator x; return; }
        if(name == "-") { op.bin = &Register::binminus; return; }
        DEFOP(+)
        DEFOP(*)
        DEFOP(/)
        DEFOP(&)
        DEFOP(|)
        DEFOP(%)
        DEFOP(^)
        DEFOP(==)
        DEFOP(!=)
        DEFOP(>)
        DEFOP(>=)
        DEFOP(<)
        DEFOP(<=)
#undef DEFOP
    } else {
#define DEFOP(x) if(name == #x) { op.un = &Register::operator x; return; }
        if(name == "-") { op.un = &Register::unminus; return; }
        DEFOP(!)
        DEFOP(~)
#undef DEFOP
    }
}


OrExp::OrExp( ) 
{
}

OrExp::~OrExp( ) 
{
}

/* <exp> || <exp> */
OrExp::OrExp(ExpNode::Pointer l, ExpNode::Pointer r) : left(l), right(r) 
{
}

Register
OrExp::evalAux() 
{
    return left->eval().toBoolean() || right->eval().toBoolean();
}

void 
OrExp::reverse(ostream &os, const DLString &nextline) const
{
    left->reverse(os, nextline);
    os << " {C||{x ";
    right->reverse(os, nextline);
}


AndExp::AndExp( ) 
{
}

AndExp::~AndExp( ) 
{
}

/* <exp> && <exp> */
AndExp::AndExp(ExpNode::Pointer l, ExpNode::Pointer r) : left(l), right(r) 
{
}

Register
AndExp::evalAux() 
{
    return left->eval().toBoolean() && right->eval().toBoolean();
}

void 
AndExp::reverse(ostream &os, const DLString &nextline) const
{
    left->reverse(os, nextline);
    os << " {C&&{x ";
    right->reverse(os, nextline);
}


AssignExp::AssignExp() 
{
}

AssignExp::~AssignExp() 
{
}

/* <reference> = <exp> */
AssignExp::AssignExp(ReferenceNode::Pointer l, ExpNode::Pointer r) 
        : left(l), right(r) 
{
}

Register
AssignExp::evalAux() 
{
    return left->eval() = right->eval();
}

void 
AssignExp::reverse(ostream &os, const DLString &nextline) const
{
    left->reverse(os, nextline);
    os << " {C={x ";
    right->reverse(os, nextline);
}

}

