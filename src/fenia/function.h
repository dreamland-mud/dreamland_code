/* $Id: function.h,v 1.1.2.5.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: function.h,v 1.1.2.5.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <ext/rb_tree>

using namespace __gnu_cxx;

#include <list>

#include <dlstring.h>
#include <xmlvariable.h>
#include <xmlvector.h>
#include <stdint.h>

#include "codesourceref.h"
#include "lex.h"
#include "manager-impl.h"
#include "nodes.h"

#include "phase.h"

namespace Scripting {

class RegisterList;
class Scope;

class ArgNames : public vector<Lex::id_t>, public DLObject {
public:
    typedef ::Pointer<ArgNames> Pointer;
};


class Function {
public:
    typedef uint32_t id_t;

    struct selectId : public unary_function<Function, id_t> {
        const id_t &operator () (const Function &f) {
            return f.getId();
        }
    };
    typedef rb_tree<id_t, Function, selectId, less<id_t> > Map;
    
    Function(id_t i);
    virtual ~Function( );

    const id_t &getId() const {
        return id;
    }

    Register invoke(Scope &scope, Register thiz, const RegisterList &args);
    
    ArgNames::Pointer argNames;
    StmtNodeList::Pointer stmts;
    CodeSourceRef source;

    DLString toString() const;
    void reverse(ostream &os, const DLString &nextline) const;
    
    /* refcnt GC */
    int refcnt;
    
    void link() {
        refcnt++;
    }
    void unlink() {
        refcnt--;

        if(DereferenceListener::instance)
            DereferenceListener::instance->notify(this);

        if(refcnt <= 0 && Scripting::gc) {
            finalize();
        }
    }

    void finalize();

private:
    /*
     * we only have to keep fn_id, since cs_id always can be obtained from
     * the `source' field. but we keep complete id_t field for speedup the
     * lookup.
     */
    id_t id;
};

class FunctionManager : public BaseManager<Function> {
};

}

#endif
