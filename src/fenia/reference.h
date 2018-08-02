/* $Id: reference.h,v 1.1.2.5.18.1 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: reference.h,v 1.1.2.5.18.1 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __REFERENCE_H__
#define __REFERENCE_H__

#include "register-decl.h"

namespace Scripting {

struct Reference {
    inline Reference();
    inline Reference(Lex::id_t k);
    inline Reference(const Register &c, const Register &k);

    const Register operator () (const RegisterList &) const;
    const Register &operator = (const Register &) const;
    const Register operator * () const;
    
    Lex::id_t id;
    Register container;
    Register key;
};

}

#endif

