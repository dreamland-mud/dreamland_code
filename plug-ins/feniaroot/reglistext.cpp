/* $Id: reglistext.cpp,v 1.1.2.7.6.5 2009/02/08 14:22:03 rufina Exp $
 *
 * ruffina, 2004
 */
#include <sstream>

#include "reglist.h"
#include "register-impl.h"
#include "fenia/object.h"
#include "nativeext.h"
#include "subr.h"

#include "wrap_utils.h"
#include "dl_math.h"

using namespace Scripting;

NMI_INVOKE( RegList, at, "(n): возвращает элемент списка под номером n")
{
    size_t n = args2number(args);
    const_iterator i;
    
    if (n < 0 || n >= size())
        return Register();

    for (i = begin( ); i != end( ) && n > 0; i++, n--)
        ;

    return *i;
}

NMI_INVOKE( RegList, random, "(): возвращает случайный элемент списка")
{
    size_t n;
    const_iterator i;
    
    if (size() == 0)
        return Register();

    n = number_range( 0, size() - 1 );
    for (i = begin( ); i != end( ) && n > 0; i++, n--)
        ;
    
    return *i;
}

NMI_INVOKE( RegList, front , "(): возвращает первый элемент списка")
{
    if (size() == 0)
        return Register();
    return front();
}

NMI_INVOKE( RegList, pop_front, "(): удаляет первый элемент списка" )
{
    if (empty( ))
        throw Scripting::Exception("list is already empty");

    pop_front( );
    self->changed();

    return Register( );
}

NMI_INVOKE( RegList, back , "(): возвращает последний элемент списка")
{
    if (size() == 0)
        return Register();
    return back();
}

NMI_INVOKE( RegList, pop_back, "(): удаляет последний элемент списка" )
{
    if (empty( ))
        throw Scripting::Exception("list is already empty");

    pop_back( );
    self->changed();

    return Register( );
}


NMI_INVOKE( RegList, add , "(args...): добавляет в конец списка все элементы, перечисленные в параметрах")
{
    insert(end( ), args.begin(), args.end());

    self->changed();

    return Register( self );
}

NMI_INVOKE( RegList, push_front, "(elem): добавляет элемент в начало списка" )
{
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
        
    push_front( args.front( ) );
    self->changed();

    return Register( );
}

NMI_INVOKE( RegList, push_back, "(elem): добавляет элемент в конец списка" )
{
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
        
    push_back( args.front( ) );
    self->changed();

    return Register( );
}

struct RemIfEq {
    RemIfEq(const Register &r) : reg(r) { }
    
    bool operator () (const Register &r) {
        return (reg == r).toBoolean( );
    }

    const Register &reg;
};

NMI_INVOKE( RegList, join, "(разделитель): соединяет все элементы списка в строку через указанный разделитель")
{
    ostringstream buf;
    DLString sep = args2string(args);

    for(const_iterator i = begin(); i != end(); i++) {
        if (i != begin())
            buf << sep;
        buf << i->toString();
    }

    return buf.str();
}

NMI_INVOKE( RegList, sub , "(args...): удаляет из списка все вхождения элементов, перечисленных в параметрах")
{
    for(RegisterList::const_iterator i = args.begin(); i != args.end(); i++)
        remove_if( RemIfEq(*i) );

    self->changed();

    return Register( self );
}

NMI_INVOKE( RegList, has, "(elem): true если указанный элемент содержится в списке")
{
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    const Register &arg = args.front( );

    for (const_iterator i = begin( ); i != end( ); i++)
        if ((arg == *i).toBoolean( ))
            return true;

    return false;
}

NMI_INVOKE( RegList, size , "(): размер списка")
{
    return (int)size( );
}

struct RegisterWeakOrder {
    bool operator () ( const Register &k1, const Register &k2 ) {
        return (k1 < k2).toBoolean( );
    }
};

NMI_INVOKE( RegList, sort , "(): сортирует список по возрастанию")
{
    sort( RegisterWeakOrder( ) );
    self->changed();

    return Register( self );
}

struct RegisterBinPred {
    bool operator () ( const Register &k1, const Register &k2 ) {
        return (k1 == k2).toBoolean( );
    }
};

NMI_INVOKE( RegList, unique , "(): удаляет идущие подряд дублирующиеся элементы")
{
    unique( RegisterBinPred( ) );
    self->changed();

    return Register( self );
}

NMI_INVOKE( RegList, clone , "(): создает новый список, аналог этого")
{
    RegList::Pointer rl(NEW);

    rl->assign(begin( ), end( ));

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rl);

    return Register( obj );
}

NMI_INVOKE( RegList, api, "(): печатает этот api")
{
    ostringstream buf;
    
    traitsAPI<RegList>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RegList, filter, "(func[,args]): возвращает новый список из элементов, для которых функция func вернула true")
{
    RegisterList::const_iterator ai = args.begin();
    if(ai == args.end())
        throw Scripting::NotEnoughArgumentsException( );

    Register rfun = *ai++;
    Closure *fun = rfun.toFunction( );
    
    RegisterList av;
    av.assign(ai, args.end( ));
    
    RegList::Pointer rc( NEW );

    for(iterator i = begin(); i != end(); i++) {
        Register reg = fun->invoke(*i, av);

        if (reg.type == Register::NUMBER && reg.toNumber() != 0)
            rc->push_back( *i );
    }

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc); 

    return Register( obj );
}
