/* $Id: idcontainerext.cpp,v 1.1.2.4.18.1 2007/09/11 00:09:28 rufina Exp $
 *
 * ruffina, 2004
 */
#include <sstream>

#include "register-impl.h"
#include "reglist.h"
#include "idcontainer.h"
#include "fenia/object.h"
#include "nativeext.h"

using namespace Scripting;

NMI_GET(IdContainer, fieldKeys, "") 
{
    RegList::Pointer rc(NEW);

    Idmap::const_iterator i;
    
    for(i = idmap.begin(); i != idmap.end(); i++)
	rc->push_back( Register( i->first ) );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE(IdContainer, size, "") 
{
    return Register( (int)idmap.size( ) );
}

NMI_INVOKE(IdContainer, api, "") 
{
    Idmap::const_iterator i;
    ostringstream buf;

    traitsAPI<IdContainer>( buf );

    buf << endl << endl << "{WRuntime fields:{x" << endl;
    for(i = idmap.begin(); i != idmap.end(); i++)
	buf << "{x" << Lex::getThis( )->getName( i->first ) << "{x" << endl;
     
    return Register( buf.str( ) );
}
