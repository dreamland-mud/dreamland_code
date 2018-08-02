/* $Id: scope.cpp,v 1.4.2.3.18.1 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: scope.cpp,v 1.4.2.3.18.1 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include "scope.h"
#include "reference-impl.h"
#include "function.h"
#include "register-impl.h"

using namespace std;

namespace Scripting {

void 
Scope::addVar(Lex::id_t key)
{
    (*this)[key] = Register();
}

Register 
Scope::getVar(Lex::id_t key)
{
    Scope *s, *s_parent;

    for(s = this; ; s = s_parent) {
	s_parent = s->parent;
	
	iterator i = s->find(key);
	
	if(i != s->end())
	    return i->second;

	if(!s_parent)
	    return *(*s)[ID_THIS][Register(key)];
    }
}

void 
Scope::setVar(Lex::id_t key, const Register &val)
{
    Scope *s, *s_parent;

    for(s = this; ; s = s_parent) {
	s_parent = s->parent;
	
	iterator i = s->find(key);
	
	if(i != s->end()) {
	    i->second = val;
	    return;
	}

	if(!s_parent) {
	   (*s)[ID_THIS][Register(key)] = val;
	   return;
	}
    }
}

Register 
Scope::callVar(Lex::id_t key, const RegisterList &args)
{
    Scope *s, *s_parent;

    for(s = this; ; s = s_parent) {
	s_parent = s->parent;
	
	iterator i = s->find(key);
	
	if(i != s->end())
	   return i->second.toFunction()->invoke(Register(), args);

	if(!s_parent)
	   return (*s)[ID_THIS][Register(key)](args);
    }
}

}
