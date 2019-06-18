/* $Id: reglist.cpp,v 1.1.2.4.6.2 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "reglist.h"
#include "subr.h"

using namespace Scripting;

template class Scripting::NativeImpl<RegList>;

const DLString RegList::TYPE = "RegList";

NMI_INIT(RegList, "список")

void
RegList::setSelf(Scripting::Object *s) 
{
    self = s;
}

const DLString &
RegList::getType() const 
{
    return TYPE;
}

DLObject::Pointer 
RegList::set(DLObject::Pointer o1, DLObject::Pointer o2) 
{
    return DLObject::Pointer();
}

bool
XMLRegisterList::toXML( XMLNode::Pointer& parent ) const
{
    const_iterator i;
    
    for(i = begin(); i != end(); i++) {
        XMLNode::Pointer node(NEW);
        
        i->toXML(node);
        node->setName("li");
        
        parent->appendChild(node);
    }

    return true;
}

void 
XMLRegisterList::fromXML( const XMLNode::Pointer& parent ) throw ( ExceptionBadType )
{
    clear();
    XMLContainer::fromXML(parent);
}

bool
XMLRegisterList::nodeFromXML( const XMLNode::Pointer& child)
{
    const DLString &name = child->getName();
    
    if(child->getType( ) != XMLNode::XML_NODE || name != "li")
        return false;

    insert(end( ), XMLRegister( ));
    back( ).fromXML(child);

    return true;
}

/* methods */

NMI_INVOKE( RegList, iterator, "")
{
    RegListIterator::Pointer it(NEW);

    it->list = Register(self);
    it->position = 0;

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );

    obj->setHandler(it);
    return Register(obj);
}

NMI_INVOKE( RegList, forEach , "(func[,args]): выполняет для каждого элемента списка функцию с аргументами")
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

        if (reg.type != Register::NONE)
            rc->push_back( reg );
    }

    Object *obj = &Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

/* fields */

NMI_GET( RegList, call , "(): вызовет следующий после этого метод для каждого элемента списка")
{
    RegListCall::Pointer rlc(NEW);
    
    rlc->list = Register( self );
    
    Object *obj = &Object::manager->allocate();
    obj->setHandler(rlc);

    return Register( obj );
}


/* ----------------------- reg list call ---------------------- */

Register 
RegListCall::getField(const Register &key)
{
    RegList *rl = wrapper_cast<RegList>(list);

    RegList::Pointer rc( NEW );
    RegList::iterator i;
    
    for(i = rl->begin(); i != rl->end(); i++)
        rc->push_back( *(*i)[key] );

    Object *obj = &Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

void 
RegListCall::setField(const Register &key, const Register &val)
{
    RegList *rl = wrapper_cast<RegList>(list);
    
    RegList::iterator i;
    
    for(i = rl->begin(); i != rl->end(); i++)
        (*i)[key] = val;
}

Register 
RegListCall::callMethod(const Register &key, const RegisterList &args)
{
    RegList *rl = wrapper_cast<RegList>(list);
    
    RegList::Pointer rc( NEW );
    RegList::iterator i;
    
    for(i = rl->begin(); i != rl->end(); i++)
        rc->push_back( (*i)[key](args) );

    Object *obj = &Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

/* ----------------------- reg list iterator ---------------------- */

NMI_INIT(RegListIterator, "Итератор списка")

RegListIterator::RegListIterator() : cached(NULL)
{
}

void RegListIterator::setSelf(Scripting::Object *s)
{
    cached = NULL;
    self = s;
}

void RegListIterator::restoreIterator()
{
    RegList *rl = wrapper_cast<RegList>(list);

    if(rl == cached)
        return;

    cached = rl;
    
    int i = 0;

    for(it=rl->begin();it != rl->end();it++) 
        if(i++ == position)
            return;
}

NMI_INVOKE(RegListIterator, hasNext, "")
{
    restoreIterator();   
    return Register(it != cached->end());
}

NMI_INVOKE(RegListIterator, next, "")
{
    restoreIterator();   
    
    if(it != cached->end()) {
        self->changed();
        position++;
        return Register(*it++);
    } else
        return Register();
}

