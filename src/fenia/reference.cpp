/* $Id: reference.cpp,v 1.1.2.8.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: reference.cpp,v 1.1.2.8.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include "reference-impl.h"
#include "exceptions.h"
#include "context.h"
#include "scope.h"

namespace Scripting {

const Register 
Reference::operator () (const RegisterList &args) const
{
    if(id)
        return Context::current->scope->callVar(id, args);

    switch(container.type) {
        case Register::NONE:
            throw NullPointerException();

        case Register::OBJECT:
            return container.toHandler()->callMethod(key, args);
            
        case Register::STRING:
        {
            FeniaString::Traits::Invoke::Entry *e = FeniaString::Traits::Invoke::List::lookup(key);
            
            if(!e || e->method == 0)
                throw Exception("Invoke method not found: " + key.toString( ));

            /*XXX: get rid of const_cast?*/
            return (const_cast<FeniaString *>(container.strPtr( ))->*(e->method))(args);
        }
        case Register::NUMBER:
        default:
            throw NotImplementedException(DLString("Cannot call operator () on ") + container.getTypeName());
    }
}


const Register &
Reference::operator = (const Register &r) const
{
    if(id) {
        Context::current->scope->setVar(id, r);
        return r;
    }

    switch(container.type) {
        case Register::OBJECT:
            container.toHandler()->setField(key, r);
            return r;

        default:
            throw NotImplementedException(DLString("Cannot call operator = on ") + container.getTypeName());
    }
}

const Register 
Reference::operator * () const
{
    if(id)
        return Context::current->scope->getVar(id);

    switch(container.type) {
        case Register::OBJECT:
            return container.toHandler()->getField(key);

        case Register::STRING:
            try {
                DLString result;

                result.assign( container.toString().at( key.toNumber() ) );
                return result;
            
            } catch (const std::exception& e) {
                throw IndexOutOfBoundsException();
            }
            
        default:
            throw NotImplementedException(DLString("Cannot call operator * on ") + container.getTypeName());
    }
}


}
