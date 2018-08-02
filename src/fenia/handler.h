/* $Id: handler.h,v 1.1.2.3.18.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: handler.h,v 1.1.2.3.18.3 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __HANDLER_H__
#define __HANDLER_H__


#include <xmlpolymorphvariable.h>

namespace Scripting {

class Object;
class Register;
class RegisterList;

class Handler : public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<Handler> Pointer;

    virtual ~Handler( );

    virtual void setField(const Register &key, const Register &val) = 0;
    virtual Register getField(const Register &key) = 0;
    virtual Register callMethod(const Register &key, const RegisterList &args) = 0;

    virtual void setSelf(Object *) = 0;
    virtual void backup(); // called when 'object -> handler' link destroyed
};

}

#endif
