/* $Id: regcontainerext.cpp,v 1.1.2.4.6.4 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */
#include <sstream>

#include "regcontainer.h"
#include "wrap_utils.h"
#include "nativeext.h"
#include "fenia/object.h"
#include "register-impl.h"
#include "reglist.h"

using namespace Scripting;

NMI_GET(RegContainer, keys, "список ключей") 
{
    RegList::Pointer rc(NEW);

    Map::const_iterator i;
    
    for(i = map.begin(); i != map.end(); i++)
        rc->push_back( i->first );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_GET(RegContainer, values, "список значений") 
{
    RegList::Pointer rc(NEW);

    Map::const_iterator i;
    
    for(i = map.begin(); i != map.end(); i++)
        rc->push_back( i->second );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE(RegContainer, size, "(): размер массива") 
{
    return Register( (int)map.size( ) );
}


NMI_INVOKE( RegContainer, clone , "(): создать дубликат массива")
{
    ::Pointer<RegContainer> rc(NEW);
    Map::const_iterator i;
    
    for(i = map.begin(); i != map.end(); i++)
        (*rc)->map[i->first] = i->second;

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE( RegContainer, api, "(): печатает этот api" )
{
    ostringstream buf;
    Scripting::traitsAPI<RegContainer>( buf );
    return Register( buf.str( ) );
}


NMI_INVOKE( RegContainer, clear, "(): очистка массива" )
{
    map.clear( );
    self->changed();
    return Register( );
}

NMI_INVOKE( RegContainer, add , "(args...): попарно добавляет в массив все элементы, перечисленные в параметрах")
{
    if (args.size() % 2 != 0) 
        throw Scripting::Exception("add() method takes even number of arguments");

    RegisterList::const_iterator a;
    for (a = args.begin(); a != args.end(); a++) {
        Register key = *a;
        Register value = *(++a);
        map[key] = value;
    }

    self->changed();
    return Register( self );
}

NMI_INVOKE( RegContainer, addAll , "(array): добавляет в этот массив все элементы из массива array")
{
    Scripting::Object *otherObj = get_unique_arg(args).toObject();
    if (otherObj && otherObj->hasHandler()) {
        RegContainer *otherArray = otherObj->getHandler().getDynamicPointer<RegContainer>();
        if (otherArray) {
            map.insert(otherArray->map.begin(), otherArray->map.end());
            self->changed();
            return Register(self);
        }
    }

    throw Scripting::Exception("Invalid array passed to addAll() method");
}