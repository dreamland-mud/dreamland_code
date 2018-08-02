/* $Id$
 *
 * ruffina, Dream Land, 2004
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include "native.h"
#include "register-impl.h"

namespace Scripting {

void NativeHandler::setField( const Register &key, const Register &val ) 
{
    if (!setNativeField( key, val ))
        throw Exception("Set method not found: " + key.toString( ));

}

Register NativeHandler::getField( const Register &key ) 
{
    Register retval;

    if (!getNativeField( key, retval ))
        throw Exception("Get method not found: " + key.toString( ));

    return retval;

}

Register NativeHandler::callMethod( const Register &key, const RegisterList &args ) 
{
    Register retval;
    BTPushNative dummy(this, key.toIdentifier());

    if (!callNativeMethod( key, args, retval ))
        throw Exception("Invoke method not found: " + key.toString( ));

    return retval;
}

}

