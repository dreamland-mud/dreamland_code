/* $Id$
 *
 * ruffina, Dream Land, 2018
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2018
 */


#include <sstream>
#include <map>
#include <utility>

#include "logstream.h"
#include "register-impl.h"
#include "xmlregister.h"
#include "codesource.h"
#include "closure.h"
#include "exceptions.h"
#include "scope.h"

using namespace Scripting;

// --- Fenia GC duplicate collapse (Trello #2857, P3.5) -----------------------
// Redirect table plus the lookup the restore constructor consults. See closure.h
// for the rationale. feniaBuildDupRedirect() (ccodesource.cpp, sharing the same
// plan as `cs gc`) fills this before boot object recovery, and it is cleared
// right after -- empty and inactive at every other time, so runtime closures
// pay only one guarded bool check.
namespace {
    // (duplicate csId, fnId) -> (canonical csId, fnId).
    std::map<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t> > g_dupRedirect;
    bool g_dupRedirectActive = false;
}

void feniaDupRedirectAdd(uint32_t dupCs, uint32_t dupFn, uint32_t canonCs, uint32_t canonFn)
{
    g_dupRedirect[std::make_pair(dupCs, dupFn)] = std::make_pair(canonCs, canonFn);
}

void feniaDupRedirectActivate()
{
    g_dupRedirectActive = true;
}

void feniaDupRedirectClear()
{
    g_dupRedirect.clear();
    g_dupRedirectActive = false;
}

bool feniaDupRedirectLookup(uint32_t &csId, uint32_t &fnId)
{
    if (!g_dupRedirectActive)
        return false;

    std::map<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t> >::iterator i
        = g_dupRedirect.find(std::make_pair(csId, fnId));
    if (i == g_dupRedirect.end())
        return false;

    csId = i->second.first;
    fnId = i->second.second;
    return true;
}

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
    // The global manager is null once CodeSource::Manager::~Manager has run.
    // ~Closure now calls this at teardown, so guard the pointer here rather than
    // lean on destruction order keeping every closure death ahead of it.
    if (!CodeSource::manager)
        return 0;

    CodeSource::Manager::iterator cs = CodeSource::manager->find(csId);

    if (cs == CodeSource::manager->end())
        return 0;

    FunctionManager::iterator fn = cs->functions.find(fnId);

    if (fn == cs->functions.end())
        return 0;

    return &*fn;
}

Closure::Closure(Scope *start, Function *f)
    : function(f), csId(f->source.csId), fnId(f->getId())
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
Closure::Closure(XMLFunctionRef &ref)
    : function(0), csId(ref.codesource.getValue()), fnId(ref.function.getValue())
{
    // Fenia GC (Trello #2857, P3.5): during boot recovery, reroute a closure that
    // names a non-canonical duplicate CodeSource to the canonical copy's matching
    // function, so the duplicate ends up unreferenced and the boot fsck reaps it.
    // Rewriting csId/fnId (not just the resolved function) keeps ~Closure and
    // toXMLFunctionRef consistent -- teardown re-resolves by the ids and the next
    // save writes the canonical. A no-op outside recovery (table inactive).
    feniaDupRedirectLookup(csId, fnId);

    function = findFunction(csId, fnId);

    if (function)
        function->link();
    else
        LogStream::sendError()
            << "fenia: closure points at a function that is gone: cs "
            << csId << " fn " << fnId
            << " -- loading it as broken, it will be saved as null" << endl;

    clear();
    for(XMLMapBase<XMLRegister>::iterator i=ref.environment.begin();i != ref.environment.end();i++) {
        Lex::id_t id = Lex::getThis()->resolve(i->first);
        (*this)[id] = i->second;
    }
}


Closure::~Closure()
{
    // Do NOT dereference the raw `function` pointer here. A mass free (the boot
    // fsck sweep, ValidateTask) can destroy the owning CodeSource -- and, with
    // it, this function -- before this closure is freed, so `function` may
    // dangle; the old unconditional `function->unlink()` then wrote refcnt-- to
    // freed memory (part of the 2026-08-08 crash-loop). Re-resolve by the stored
    // (csId, fnId) and unlink only when the live function is still the exact
    // object we linked. If it is gone, or its id was reused by another code
    // source, there is nothing of ours left to unlink. See Trello #2857 (P2a).
    if (!function)
        return;

    if (findFunction(csId, fnId) == function)
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

    // Use the stored ids rather than dereferencing source.source (its raw
    // CodeSource* is the pointer P1 stopped trusting). csId/fnId are the same
    // values isBroken() has just confirmed resolve to a live function.
    ref.codesource = csId;
    ref.function = fnId;

    ref.environment.clear();

    for(iterator i = begin();i != end();i++) {
        DLString name = Lex::getThis()->getName(i->first);
        ref.environment[name] = i->second;
    }

    return true;
}
