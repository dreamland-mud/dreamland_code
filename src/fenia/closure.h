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
    void toXMLFunctionRef(XMLFunctionRef &ref);
private:
    Function *function;
};

}

#endif
