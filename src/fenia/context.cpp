/* $Id: context.cpp,v 1.1.2.5.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: context.cpp,v 1.1.2.5.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <iostream>

#include "context.h"
#include "handler.h"
#include "codesource.h"
#include "register-impl.h"

using namespace std;

namespace Scripting {

Context *Context::current = 0;
Register Context::root;

BackTrace::BackTrace()
{
    BackTrace *&top = Context::current->backTrace;
    parent = top;
    top = this;
}

BackTrace::~BackTrace()
{
    Context::current->backTrace = parent;
}

void 
BackTrace::report(ostream &os) 
{
    BackTrace *p;
    
    for(p = Context::current->backTrace; p; p = p->parent)
	os << "    called from " << *p << endl;
    
    os << "    Ya-ma-ta!" << endl;
}

void 
BTPushNative::print(ostream &os) const 
{
    os << handler->getType() << "::" << Lex::getThis()->getName(id);
}
    
void 
BTPushNode::print(ostream &os) const 
{
    if(!node) {
	os << "<native>";
    } else {
	os << node->source;
    }
}

}
