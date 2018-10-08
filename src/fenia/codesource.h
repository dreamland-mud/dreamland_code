/* $Id: codesource.h,v 1.1.2.5.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: codesource.h,v 1.1.2.5.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */
#ifndef __CODESOURCE_H__
#define __CODESOURCE_H__

#include <ext/rb_tree>
#include <stdint.h>

using namespace __gnu_cxx;

#include "txncontext.h"
#include "manager-decl.h"
#include "phase.h"
#include "function.h"

namespace Scripting {

class Register;
class FunctionManager;

class CodeSource {
public:
    typedef ::Pointer<CodeSource> Pointer;

    typedef uint32_t id_t;
    struct selectId : public unary_function<CodeSource, id_t> {
        const id_t &operator () (const CodeSource &f) {
            return f.getId();
        }
    };
    typedef rb_tree<id_t, CodeSource, selectId, less<id_t> > Map;

    class Manager;

    CodeSource(id_t i) : refcnt(0), id(i), evaled(false) { }

    Register eval(Register thiz);
    Register eval();

    const id_t &getId() const 
    {
        return id;
    }
    
    /* refcnt GC */
    int refcnt;

    void link() {
        refcnt++;
    }
    void unlink() {
        refcnt--;
        if(refcnt <= 0 && Scripting::gc) {
            finalize();
        }
    }

    void finalize();

    static Manager *manager;
    
    DLString name, author, content;

    FunctionManager functions;
private:
    id_t id;
    bool evaled;
};

class CodeSource::Manager : public BaseManager<CodeSource>, public DbContext { 
public:
    Manager();
    virtual ~Manager();

    void open( );
    void close( );

    void put( id_t, CodeSource & );
    virtual void seq( id_t, Data & );
};

}

#endif
