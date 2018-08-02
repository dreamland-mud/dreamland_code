/* $Id$
 *
 * ruffina, Dream Land, 2018
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2018
 */


#include <sstream>

#include "logstream.h"
#include "register-impl.h"
#include "xmlregister.h"
#include "codesource.h"
#include "closure.h"
#include "scope.h"

using namespace Scripting;

Closure::Closure(Scope *start, Function *f) : function(f)
{
    copyScope(start);
    function->link();
}

Closure::Closure(XMLFunctionRef &ref)
{
    function = &CodeSource::manager->at(ref.codesource).functions.at(ref.function);
    function->link();

    clear();
    for(XMLMapBase<XMLRegister>::iterator i=ref.environment.begin();i != ref.environment.end();i++) {
        Lex::id_t id = Lex::getThis()->resolve(i->first);
        (*this)[id] = i->second;
    }
}


Closure::~Closure()
{
    function->unlink();
}

void
Closure::copyScope(Scope *scope)
{
    if(!scope)
        return;

    // parent first, so that the most inner scope takes priority
    copyScope(scope->parent);
    insert(scope->begin(), scope->end());
}

Register
Closure::invoke(Register thiz, const RegisterList &args)
{
    Register dummy(this);

    CppScopeClobberRoot root;

    root.insert(begin(), end());

    return function->invoke(root, thiz, args);
}

void 
Closure::reverse(ostream &os, const DLString &nextline) const
{
    os << "[";
    for(const_iterator i = begin();i != end();i++) {
        if(i != begin()) 
            os << ", ";

        DLString name = Lex::getThis()->getName(i->first);
        os << name << "=" << i->second.repr();
    }
    os << "] ";
    function->reverse(os, nextline);
}

DLString 
Closure::toString() const
{
    ostringstream os;

    reverse(os, DLString("\r\n"));

    return os.str();
}

void
Closure::toXMLFunctionRef(XMLFunctionRef &ref)
{
    ref.codesource = function->source.source->getId();
    ref.function = function->getId();

    ref.environment.clear();

    for(iterator i = begin();i != end();i++) {
        DLString name = Lex::getThis()->getName(i->first);
        ref.environment[name] = i->second;
    }
}
