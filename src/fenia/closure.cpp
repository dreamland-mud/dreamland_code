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
#include "exceptions.h"
#include "scope.h"

using namespace Scripting;

/**
 * Find a function without creating one.
 *
 * BaseManager::at() inserts a default-constructed entry for an unknown id, so
 * asking it for a function that is gone quietly hands back an empty Function
 * with a null CodeSourceRef. That stand-in cannot be told apart from a real
 * function afterwards, and the first attempt to serialize it dereferences the
 * null source. Look the ids up instead, and report a miss as a miss.
 */
static Function * findFunction(CodeSource::id_t csId, Function::id_t fnId)
{
    CodeSource::Manager::iterator cs = CodeSource::manager->find(csId);

    if (cs == CodeSource::manager->end())
        return 0;

    FunctionManager::iterator fn = cs->functions.find(fnId);

    if (fn == cs->functions.end())
        return 0;

    return &*fn;
}

Closure::Closure(Scope *start, Function *f) : function(f)
{
    copyScope(start);
    function->link();
}

/**
 * Recover a closure from the database.
 *
 * The code source it points at can legitimately be missing: every 'cs post'
 * replaces a file's CodeSource, and a closure stored in a .tmp.* map keeps
 * naming the id that was current when it was saved. Such a closure is loaded
 * as broken rather than as a fabricated stand-in, so that it fails loudly at
 * the point of use and gets written back as null on the next save.
 */
Closure::Closure(XMLFunctionRef &ref) : function(0)
{
    function = findFunction(ref.codesource.getValue(), ref.function.getValue());

    if (function)
        function->link();
    else
        LogStream::sendError()
            << "fenia: closure points at a function that is gone: cs "
            << ref.codesource.getValue() << " fn " << ref.function.getValue()
            << " -- loading it as broken, it will be saved as null" << endl;

    clear();
    for(XMLMapBase<XMLRegister>::iterator i=ref.environment.begin();i != ref.environment.end();i++) {
        Lex::id_t id = Lex::getThis()->resolve(i->first);
        (*this)[id] = i->second;
    }
}


Closure::~Closure()
{
    if (function)
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
    if (!function)
        throw FunctionNotDefinedException();

    Register dummy(this);

    CppScopeClobberRoot root;

    root.insert(begin(), end());

    return function->invoke(root, thiz, args);
}

void
Closure::reverse(ostream &os, const DLString &nextline) const
{
    if (!function) {
        os << "{Rbroken function{x ";
        return;
    }

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

bool
Closure::toXMLFunctionRef(XMLFunctionRef &ref)
{
    if (isBroken())
        return false;

    ref.codesource = function->source.source->getId();
    ref.function = function->getId();

    ref.environment.clear();

    for(iterator i = begin();i != end();i++) {
        DLString name = Lex::getThis()->getName(i->first);
        ref.environment[name] = i->second;
    }

    return true;
}
