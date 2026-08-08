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
};

}

#endif
