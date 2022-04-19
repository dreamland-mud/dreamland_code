/* $Id: scope.h,v 1.4.2.4.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: scope.h,v 1.4.2.4.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "register-decl.h"
#include "context.h"

namespace Scripting {
    
class Scope : public map<Lex::id_t, Register> {
public:
    Scope() : parent(0) { }
    Scope(Scope *p) : parent(p) { }

    void addVar(Lex::id_t);
    Register getVar(Lex::id_t);
    void setVar(Lex::id_t, const Register &);
    Register callVar(Lex::id_t, const RegisterList &);

    Scope *parent;
};

class CppScopeClobber : public Scope {
public:
    CppScopeClobber() : Scope(Context::current->scope) {
        Context::current->scope = this;
    }
    ~CppScopeClobber() {
        if (Context::current)
            Context::current->scope = parent;
    }
};

class CppScopeClobberRoot : public Scope {
public:
    CppScopeClobberRoot() : save(Context::current->scope) { 
        Context::current->scope = this;
    }
    ~CppScopeClobberRoot() {
        if (Context::current)
            Context::current->scope = save;
    }

    Scope *save;
};


}

#endif /* __SCOPE_H__ */
