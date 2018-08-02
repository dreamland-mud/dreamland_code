/* $Id: ref-tree.cpp,v 1.1.2.6.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: ref-tree.cpp,v 1.1.2.6.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include "register-impl.h"
#include "reference-impl.h"
#include "ref-tree.h"
#include "context.h"

namespace Scripting {

/* References */

DefaultRef::DefaultRef( ) 
{
}

DefaultRef::~DefaultRef( ) 
{
}

/* <identifier> */
DefaultRef::DefaultRef(Lex::id_t i) : key(i) 
{
}

Reference
DefaultRef::evalAux() 
{
    return Reference(key);
}

void 
DefaultRef::reverse(ostream &os, const DLString &nextline) const
{
    os << Lex::getThis()->getName(key);
}


HashRef::HashRef( )
{
}

HashRef::~HashRef( )
{
}

/* <exp>[<exp>] */    
HashRef::HashRef(ExpNode::Pointer e, ExpNode::Pointer k) : exp(e), key(k) 
{
}

Reference
HashRef::evalAux() 
{
    return exp->eval() [key->eval()];
}

void 
HashRef::reverse(ostream &os, const DLString &nextline) const
{
    exp->reverse(os, nextline);
    os << "[ ";
    key->reverse(os, nextline);
    os << " ]";
}


MemberRef::MemberRef( )
{
}

MemberRef::~MemberRef( )
{
}

/* <exp>.<identifier> */
MemberRef::MemberRef(ExpNode::Pointer e, Lex::id_t k) : exp(e), key(k) 
{
}

Reference
MemberRef::evalAux() 
{
    return exp->eval() [ key ] ;
}

void 
MemberRef::reverse(ostream &os, const DLString &nextline) const
{
    exp->reverse(os, nextline);
    os << '.' << key.toString();
}


RootRef::~RootRef( )
{
}

RootRef::RootRef( )
{
}

/* .<identifier> */
RootRef::RootRef(Lex::id_t k) : key(k) 
{
}

Reference
RootRef::evalAux() 
{
    return Context::root [ key ];
}

void 
RootRef::reverse(ostream &os, const DLString &nextline) const
{
    os << '.' << key.toString();
}

}
