/* $Id: function.cpp,v 1.1.2.9.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: function.cpp,v 1.1.2.9.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <integer.h>

#include <istream>
#include <sstream>

using namespace std;

#include "register-impl.h"
#include "function.h"
#include "codesource.h"
#include "exceptions.h"
#include "scope.h"
#include "stmt-tree.h"
#include "flow.h"

namespace Scripting {

Function::Function(id_t i) : refcnt(0), id(i)
{
}

Function::~Function( )
{
}

DLString 
Function::toString() const
{
    ostringstream os;

    reverse(os, DLString("\r\n"));

    return os.str();
}

void 
Function::reverse(ostream &os, const DLString &nextline) const
{
    if (!argNames || !stmts)
        throw NullPointerException();

    os << "{Gfunction{x {M" << id <<  "{x (";
    
    for(ArgNames::const_iterator i = argNames->begin();i != argNames->end(); i++) {
        if(i != argNames->begin())
            os << ", ";
            
        os << Lex::getThis()->getName(*i);
    }
    os << ") {{ ";
    
    for(StmtNodeList::const_iterator i = stmts->begin();i != stmts->end(); i++)
        (*i)->reverse(os, nextline + "    ");
    
    os << nextline << "} ";
}

Register
Function::invoke(Scope &sroot, Register thiz, RegisterList const &args)
{
    if (!argNames || !stmts)
        throw NullPointerException();

    RegisterList::const_iterator ali = args.begin();
    ArgNames::const_iterator ani = argNames->begin();
    
    sroot.addVar(ID_THIS);
    sroot.setVar(ID_THIS, thiz);

    DLString expected = argNames->toString();
    DLString actual(args.size());

    for(;ani != argNames->end();ani++, ali++) {
        if(ali == args.end())
            throw NotEnoughArgumentsException(expected, actual);
        else {
            sroot.addVar(*ani);
            sroot.setVar(*ani, *ali);
        }
    }
    
    if(ali != args.end()) {
        /* XXX - should place 'em in `argv' vector*/
        throw TooManyArgumentsException(expected, actual);
    }
    
    BTPushNode bt;

    StmtNodeList::iterator i;
    for(i=stmts->begin();i != stmts->end(); i++) {
        FlowCtl fc = (*i)->eval();
        
        if(fc.type == FlowCtl::BREAK)
            throw MissplacedBreakException();
        if(fc.type == FlowCtl::CONTINUE)
            throw MissplacedContinueException();
        if(fc.type == FlowCtl::RETURN)
            return fc.ret;
    }

    return Register();
}
    
void
Function::finalize()
{
    // A function recovered against a missing code source has no manager to be
    // removed from. It is not owned by anything, so letting it go is enough.
    if (!source.source)
        return;

    // The owning CodeSource may already be gone. A mass free (the boot fsck
    // sweep) frees objects and code sources in separate passes, so a CodeSource
    // can be destroyed before a Closure that still holds a raw pointer to one of
    // its functions; that function's `source.source` is then a non-null DANGLING
    // pointer, and the old dereference `source.source->functions.erase` walked a
    // freed function manager -- the 2026-08-08 SIGSEGV (four crash-looping
    // boots, "manager at 0x68"). Never dereference it: look the CodeSource up by
    // the stored id and confirm the live entry is the SAME object before
    // touching its function manager. (Removes the fatal deref; a full sweep must
    // still order destruction -- closures before their sources -- see the Fenia
    // GC collector plan, Trello #2857, P2a.)
    CodeSource::Manager::iterator it = CodeSource::manager->find(source.csId);
    if (it == CodeSource::manager->end())
        return;                                  // cs already collected
    if (&*it != source.source.getPointer())
        return;                                  // stored id now names a different cs (id wrap)

    it->functions.erase(id);
}

DLString ArgNames::toString() const
{
    ostringstream buf;

    for (auto &n: *this) {
        buf << Lex::getThis()->getName(n) << ", ";
    }

    buf << "total " << size();
    return buf.str();
}

}
