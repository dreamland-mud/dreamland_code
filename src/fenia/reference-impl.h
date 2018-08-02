/* $Id: reference-impl.h,v 1.1.2.2.18.1 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: reference-impl.h,v 1.1.2.2.18.1 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __REFERENCE_IMPL_H__
#define __REFERENCE_IMPL_H__

#include "reference.h"
#include "register-impl.h"


namespace Scripting {

    
Reference::Reference() : id(0)
{
}

Reference::Reference(Lex::id_t k) : id(k)
{
}

Reference::Reference(const Register &c, const Register &k) 
	: id(0), container(c), key(k) 
{ 
}

}

#endif

