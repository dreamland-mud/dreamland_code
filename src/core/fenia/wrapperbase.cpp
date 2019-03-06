/* $Id: wrapperbase.cpp,v 1.1.2.7.6.4 2009/11/08 17:35:28 rufina Exp $
 *
 * ruffina, 2004
 */
#include "wrapperbase.h"
#include "logstream.h"

#include "fenia/object.h"
#include "fenia/register-impl.h"
#include "fenia/context.h"
#include "feniamanager.h"
#include "schedulerwrapper.h"

WrapperBase::WrapperBase( ) : alive(false), zombie( false )
{
}

void WrapperBase::setSelf( Scripting::Object *self ) 
{
    this->self = self;

    if (self) {
        if (!zombie) /* умерла так умерла */
            WrapperManagerBase::map[id] = self;
    }
    else {
        WrapperManagerBase::WrapperMap::iterator i;
        WrapperManagerBase::WrapperMap &map = WrapperManagerBase::map;

        i = map.find( id );

        if (i != map.end( ))
            map.erase( i );
    }
}

void WrapperBase::extract( bool count )
{
    Pointer dummy(this);

    if (count) {
        WrapperManagerBase::WrapperMap::iterator i;
        WrapperManagerBase::WrapperMap &map = WrapperManagerBase::map;

        guts.clear( );

        i = map.find( id );

        if (i != map.end( ))
            map.erase( i );

        zombie = true;

        if(self)
            self->changed( );
    }
}

WrapperBase *
get_wrapper(Scripting::Object *obj)
{
    if(obj && obj->hasHandler( ))
        return obj->getHandler( ).getDynamicPointer<WrapperBase>( );
    else
        return 0;
}

void WrapperBase::postpone( Register id, const char *fmt, ... )
{
    RegisterList regList;
    Register prog;
    va_list ap;

    if (!triggerFunction(id, prog))
        return;
    
    va_start(ap, fmt);
    triggerArgs(regList, fmt, ap);
    va_end(ap);

    FeniaProcess::Pointer fp(NEW);
    fp->args.assign(regList.begin(), regList.end());
    fp->thiz = Register(self);
    fp->fun = prog;
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(fp);
    
    fp->start();
}

bool 
WrapperBase::call( Register id, const char *fmt, ... )
{
    Register rc;
    bool success;
    va_list ap;
    
    va_start(ap, fmt);
    
    success = vcall(rc, id, fmt, ap);
    
    va_end(ap);

    return success && rc.type != Register::NONE && rc.toBoolean();
}

void WrapperBase::triggerArgs( RegisterList &regList, const char *fmt, va_list ap )
{
    for (; *fmt; fmt++)
        switch (*fmt) {
        case 'C':
            regList.push_back( 
                    FeniaManager::wrapperManager->getWrapper( 
                            va_arg( ap, Character * ) ) );
            break;

        case 'O':
            regList.push_back( 
                    FeniaManager::wrapperManager->getWrapper( 
                            va_arg( ap, Object * ) ) );
            break;
        case 'R':
            regList.push_back( 
                    FeniaManager::wrapperManager->getWrapper( 
                            va_arg( ap, Room * ) ) );
            break;

        case 's':
            regList.push_back( Register( va_arg( ap, char * ) ) );
            break;
        case 'i':
            regList.push_back( Register( va_arg( ap, int ) ) );
            break;
        }
}

bool WrapperBase::hasTrigger(const DLString &name ) const
{
    Scripting::IdRef onId( DLString("on")+name ); 
    Register prog;
    return triggerFunction(onId, prog);
}

bool WrapperBase::triggerFunction(const Register &key, Register &prog) const
{    
    Lex::id_t id = key.toIdentifier( );
    
    Guts::const_iterator i = guts.find(id);

    if(i == guts.end( )) 
        return false;
    
    prog = i->second.target;

    if(prog.type != Register::FUNCTION) 
        return false;

    return true;
}

bool 
WrapperBase::vcall( Register &rc, const Register &key, const char *fmt, va_list ap )
{
    RegisterList regList;
    Register prog;

    if (!triggerFunction(key, prog))
        return false;

    triggerArgs(regList, fmt, ap);

    try {
        rc = prog.toFunction( )->invoke( Register( self ), regList );
        return true;

    } catch (const Exception &e) {
        croak(key, e);
    }

    return false;
}

void WrapperBase::croak(const Register &key, const Exception &e) const
{
    try {
        LogStream::sendError() 
            << "RT exception calling fenia prog " 
            << key.toString() << ": " << e.what();
    } catch(const Exception &x) {
        LogStream::sendError() 
            << "Exception, trying to report exception " 
            << e.what() << ": " << x.what() << endl;
    }
}

DLString
WrapperBase::stringCall( Register id, const char *fmt, ... )
{
    Register rc;
    bool success;
    va_list ap;
    
    va_start(ap, fmt);
    
    success = vcall(rc, id, fmt, ap);

    va_end(ap);

    if (!success || rc.type == Register::NONE) 
        return DLString::emptyString;
        
    return rc.toString();
}


void WrapperBase::setField(const Register &key, const Register &val)
{
    if (setNativeField(key, val))
        return;

    Lex::id_t id = key.toIdentifier();
    
    if(val.type == Register::NONE) {
        Guts::iterator i = guts.find( id );
        
        if(i != guts.end()) {
            guts.erase(i);
            self->changed();
        }
    } else {
        GutsField &f = guts[id];
        f.target = val;
        f.backref = Register( self );
        self->changed();
    }
}

Register WrapperBase::getField(const Register &key)
{
    Register retval;

    if (getNativeField(key, retval))
        return retval;

    Lex::id_t id = key.toIdentifier();
    
    Guts::iterator i = guts.find( id );

    if(i == guts.end( ))
        return Register( );
    else
        return i->second.target;
}

Register WrapperBase::callMethod(const Register &key, const RegisterList &args )
{
    Register retval;

    {
        Scripting::BTPushNative dummy(this, key.toIdentifier());

        if (callNativeMethod(key, args, retval))
            return retval;
    }

    Lex::id_t id = key.toIdentifier();
    
    Guts::iterator i = guts.find( id );

    if(i == guts.end( ))
        throw Scripting::UnknownNativeMethodException();

    return i->second.target.toFunction()->invoke(i->second.backref, args);
}

