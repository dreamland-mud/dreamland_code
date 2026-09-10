/* $Id$
 *
 * ruffina, Dream Land, 2018
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2018
 */


#ifndef __CLOSURE_H__
#define __CLOSURE_H__

#include "dlobject.h"
#include "function.h"

namespace Scripting {

class RegisterList;
class Scope;
class XMLFunctionRef;

class Closure : public map<Lex::id_t, Register>, public DLObject {
public:
    Closure(XMLFunctionRef &ref);
    Closure(Scope *start, Function *f);
    virtual ~Closure();

    inline Function *getFunction() {
        return function;
    }

    void copyScope(Scope *s);
    Register invoke(Register thiz, const RegisterList &args);

    void reverse(ostream &os, const DLString &nextline) const;
    DLString toString() const;

    /**
     * Fill in a serializable reference to this closure.
     * @return false when the closure is broken (see isBroken), i.e. there is
     *         nothing valid to write out.
     */
    bool toXMLFunctionRef(XMLFunctionRef &ref);

    /**
     * A closure loaded from a reference to a code source that no longer exists.
     * It cannot be invoked, printed or saved -- see the constructor for why one
     * can exist at all.
     */
    inline bool isBroken() const {
        return function == 0 || !function->source.source;
    }
private:
    Function *function;

    // Identity of `function`, captured when the closure is built. Teardown
    // (~Closure) must NOT dereference the raw `function` pointer: a mass free
    // (the boot fsck sweep) can destroy the owning CodeSource, and with it this
    // function, before this closure is freed -- leaving `function` dangling.
    // These ids let ~Closure re-resolve the function through the managers and
    // unlink it only while it is genuinely still alive. See Trello #2857 (P2a).
    uint32_t csId = 0;
    Function::id_t fnId = 0;
};

}

// Fenia GC duplicate collapse (Trello #2857, P3.5). Pre-P2b hot-reloads minted a
// fresh same-name CodeSource each time; the old copies persist in the DB, still
// referenced, so the boot fsck cannot reap them. This redirect makes boot
// recovery route a closure that names a non-canonical duplicate to the canonical
// copy's matching function, so every duplicate falls to refcnt 0 and the boot
// fsck (ValidateTask) reaps it in the same boot. feniaBuildDupRedirect() builds
// the plan (shared with `cs gc`) before object recovery; Closure's restore
// constructor consults feniaDupRedirectLookup(); feniaDupRedirectClear() runs
// right after recovery. The map is empty and inactive at every other time.
void feniaDupRedirectAdd(uint32_t dupCs, uint32_t dupFn, uint32_t canonCs, uint32_t canonFn);
void feniaDupRedirectActivate();
void feniaDupRedirectClear();
bool feniaDupRedirectLookup(uint32_t &csId, uint32_t &fnId);
// True exactly once per process -- the real boot's first WrappersPlugin
// initialization -- and false on every later plug-reload re-initialization, so
// the collapse arms boot-only. The flag lives in the fenia core, which plug
// reload does not re-load, so it survives even if the plugin .so is re-dlopened.
bool feniaDupRedirectFirstUse();
// Build the boot-time collapse plan and arm the redirect. Returns the number of
// duplicate copies armed (0 when not the first boot, or nothing to collapse); a
// positive count tells the caller to force-save objects afterwards (durability).
int feniaBuildDupRedirect();

#endif
