/* $Id: register-impl.h,v 1.1.2.7.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: register-impl.h,v 1.1.2.7.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __REGISTER_IMPL_H__
#define __REGISTER_IMPL_H__

#include <integer.h>

#include "function.h"
#include "object.h"

#include "register-decl.h"
#include "feniastring.h"
#include "reference.h"
#include "closure.h"

namespace Scripting {
    
inline 
Register::Register() : type(NONE) 
{
}

inline 
Register::Register(int i) 
{
    safeSet(i);
}

inline
Register::Register(Lex::id_t i) 
{
    safeSet(i);
}

inline
Register::Register(const DLString &s) 
{
    safeSet(s);
}

inline
Register::Register(const string &s)
{
    safeSet(DLString(s));
}

inline
Register::Register(const char *c) 
{
    if(c)
        safeSet(DLString(c));
    else
        type = NONE;
}

inline
Register::Register(Closure *c) 
{
    if(c) {
        value.function = c;
        value.function->link();
        type = FUNCTION;
    } else
        type = NONE;
}

inline
Register::Register(Object *o) 
{
    if(o) {
        value.object = o;
        value.object->link();
        type = OBJECT;
    } else
        type = NONE;
}

inline
Register::Register(const Register &r) : type(r.type) 
{
    switch(type) {
        case NUMBER:
            value.number = r.value.number;
            break;
        case IDENTIFIER:
            value.identifier = r.value.identifier;
            break;
        case STRING:
            strNew( );
            *strPtr( ) = *r.strPtr( );
            break;
        case OBJECT:
            value.object = r.value.object;
            value.object->link();
            break;
        case FUNCTION:
            value.function = r.value.function;
            value.function->link();
            break;
        default:
            type = r.type;
    }
}

template <typename T> 
inline Register 
Register::handler()
{
    Object &o = Object::manager->allocate();

    o.setHandler(::Pointer<T>(NEW));
//    o.setHandler(new T);

    return Register(&o);
}

template <typename T>
inline Register 
Register::handler(const DLString &argument)
{
    Object &o = Object::manager->allocate();

    o.setHandler(::Pointer<T>(NEW, argument));

    return Register(&o);
}



inline
Register::~Register() 
{
    release();
}

inline
const Register &
Register::operator = (int r) 
{
    set(r);
    return *this;
}

inline
const Register &
Register::operator = (Lex::id_t r) 
{
    set(r);
    return *this;
}

inline
const Register &
Register::operator = (const DLString &r) 
{
    set(r);
    return *this;
}

inline
const Register &
Register::operator = (const char *r) 
{
    set(DLString(r));
    return *this;
}

inline
const Register &
Register::operator = (Closure *r) 
{
    set(r);
    return *this;
}

inline
const Register &
Register::operator = (Object *r) 
{
    set(r);
    return *this;
}

inline
const Register &
Register::operator = (const Register &r) 
{
    switch(r.type) {
        case NUMBER:
            set(r.value.number);
            break;
        case IDENTIFIER:
            set(r.value.identifier);
            break;
        case STRING:
            set(*r.strPtr( ));
            break;
        case OBJECT:
            set(r.value.object);
            break;
        case FUNCTION:
            set(r.value.function);
            break;
        default:
            release();
            type = r.type;
    }
    return *this;
}

inline
FeniaString *
Register::strPtr( )
{
    return (FeniaString *)value.aux;
}

inline
const FeniaString *
Register::strPtr( ) const
{
    return (const FeniaString *)value.aux;
}

inline
void
Register::strNew( )
{
    new (value.aux) FeniaString( );
}

inline
void 
Register::release() 
{
    switch(type) {
        case STRING:
            strPtr( )->~FeniaString( );
            break;
        case OBJECT:
            value.object->unlink();
            break;
        case FUNCTION:
            value.function->unlink();
            break;
        default:
            ;
    }
    type = NONE;
}

inline
void 
Register::safeSet(Closure *a) 
{
    value.function = a;
    type = FUNCTION; 
}

inline
void 
Register::safeSet(Object *a) 
{ 
    value.object = a;
    type = OBJECT;
}

inline
void 
Register::safeSet(const FeniaString &a) 
{
    strNew( );
    *strPtr( ) = a;

    type = STRING;
}

inline
void 
Register::safeSet(int a) 
{ 
    value.number = a;
    type = NUMBER;
}

inline
void 
Register::safeSet(Lex::id_t a) 
{ 
    value.identifier = a;
    type = IDENTIFIER;
}

inline
void 
Register::set(int a) 
{ 
    release();
    safeSet(a);
}

inline
void 
Register::set(Lex::id_t a) 
{ 
    release();
    safeSet(a);
}

inline
void 
Register::set(const FeniaString &a) { 
    release();
    safeSet(a);
}
    
inline
void 
Register::set(Closure *a) { 
    if(a) {
        a->link();
        release();
        safeSet(a);
    } else {
        release();
        type = NONE;
    }
}

inline
void 
Register::set(Object *a) {
    if(a) {
        a->link();
        release();
        safeSet(a);
    } else {
        release();
        type = NONE;
    }
}

///////////// from register.cpp

inline const Reference 
Register::operator [] (const Register &r) const
{
    return Reference(*this, r);
}

/* mixer operators */
inline const Register 
Register::operator + (const Register &r) const
{
    switch(type) {
        case NUMBER:
            return value.number + r.toNumber();
        case STRING:
            return DLString(*strPtr( ) + r.toString());
        default:
            throw NotImplementedException();
    }
}

/*arithmetics operators*/
inline const Register 
Register::operator * (const Register &r) const
{
    if(type == NUMBER)
        return value.number * r.toNumber();
    
    throw NotImplementedException();
}

inline const Register 
Register::operator - (const Register &r) const
{
    if(type == NUMBER)
        return value.number - r.toNumber();
    
    throw NotImplementedException();
}

inline const Register 
Register::operator / (const Register &r) const
{
    if(type == NUMBER) {
        int d = r.toNumber();

        if(d == 0)
            throw DivisionByZero();
            
        return value.number / d;
    }
    
    throw NotImplementedException();
}

inline const Register 
Register::operator & (const Register &r) const
{
    if(type == NUMBER)
        return value.number & r.toNumber();
    
    throw NotImplementedException();
}

inline const Register
Register::operator | (const Register &r) const
{
    if(type == NUMBER)
        return value.number | r.toNumber();
    
    throw NotImplementedException();
}

inline const Register
Register::operator % (const Register &r) const
{
    if(type == NUMBER) {
        int d = r.toNumber();

        if(d == 0)
            throw DivisionByZero();
            
        return value.number % d;
    }
    
    throw NotImplementedException();
}

inline const Register
Register::operator ^ (const Register &r) const
{
    if(type == NUMBER)
        return value.number ^ r.toNumber();
    
    throw NotImplementedException();
}

/*comparition operators*/
inline const Register
Register::operator == (const Register &r) const
{
    if(type == NONE || r.type == NONE)
        return type == r.type;

    switch(type) {
        case NUMBER:
            return value.number == r.toNumber();
        case IDENTIFIER:
            return value.identifier == r.toIdentifier();
        case STRING:
            return *strPtr( ) == r.toString();
        case OBJECT:
            return value.object == r.toObject();
        case FUNCTION:
            return value.function == r.toFunction();
        default:
            throw NotImplementedException();
    }
}

inline const Register
Register::operator != (const Register &r) const
{
    if(type == NONE || r.type == NONE)
        return type != r.type;

    switch(type) {
        case NONE:
            return r.type != NONE;
        case NUMBER:
            return value.number != r.toNumber();
        case IDENTIFIER:
            return value.identifier != r.toIdentifier();
        case STRING:
            return *strPtr( ) != r.toString();
        case OBJECT:
            return value.object != r.toObject();
        case FUNCTION:
            return value.function != r.toFunction();
        default:
            throw NotImplementedException();
    }
}

inline const Register
Register::operator > (const Register &r) const
{
    switch(type) {
        case NUMBER:
            return value.number > r.toNumber();
        case IDENTIFIER:
            return value.identifier > r.toIdentifier();
        case STRING:
            return *strPtr( ) > r.toString();
        default:
            throw NotImplementedException();
    }
}

inline const Register
Register::operator < (const Register &r) const
{
    switch(type) {
        case NUMBER:
            return value.number < r.toNumber();
        case IDENTIFIER:
            return value.identifier < r.toIdentifier();
        case STRING:
            return *strPtr( ) < r.toString();
        default:
            throw NotImplementedException();
    }
}

inline const Register
Register::operator >= (const Register &r) const
{
    switch(type) {
        case NUMBER:
            return value.number >= r.toNumber();
        case IDENTIFIER:
            return value.identifier >= r.toIdentifier();
        case STRING:
            return *strPtr( ) >= r.toString();
        default:
            throw NotImplementedException();
    }
}

inline const Register
Register::operator <= (const Register &r) const
{
    switch(type) {
        case NUMBER:
            return value.number <= r.toNumber();
        case IDENTIFIER:
            return value.identifier <= r.toIdentifier();
        case STRING:
            return *strPtr( ) <= r.toString();
        default:
            throw NotImplementedException();
    }
}


inline const Register
Register::operator ! () const
{
    return !toBoolean();
}

inline const Register
Register::operator ~ () const
{
    return ~toNumber();
}

inline const Register
Register::operator - () const
{
    return -toNumber();
}

inline const Register 
Register::binminus (const Register &r) const 
{
    return operator - (r);
}

inline const Register 
Register::unminus () const 
{
    return operator - ();
}

inline int 
Register::toNumber() const
{
    switch(type) {
        case NUMBER:
            return value.number;
        case STRING:
            return strPtr( )->toInt();
        default:
            throw InvalidCastException("number", getTypeName());
    }
}

inline Lex::id_t
Register::toIdentifier() const 
{
    if(type == IDENTIFIER)
        return value.identifier;
    else
        throw InvalidCastException("identifier", getTypeName());
}

inline DLString
Register::toString() const
{
    switch(type) {
        case NUMBER:
            return Integer(value.number).toString();
        case IDENTIFIER:
            return Lex::getThis()->getName(value.identifier);
        case STRING:
            return *strPtr( );
        case FUNCTION:
            return value.function->toString();
        default:
            throw InvalidCastException("string", getTypeName());
    }
}

inline DLString
Register::repr() const
{
    switch(type) {
        case NONE:
            return DLString("null");
        case OBJECT:
            {
                Handler::Pointer h = value.object->getHandler();
                DLString htype = h ? h->getType() : "<offline>";
                return DLString("OBJECT #") + DLString(value.object->getId()) + ": " + htype;
            }
        default:
            return toString();
    }
}

inline Object *
Register::toObject() const
{
    if(type == OBJECT)
        return value.object;
    else
        throw InvalidCastException("object", getTypeName());
}

inline Closure *
Register::toFunction() const
{
    if(type == FUNCTION)
        return value.function;
    else
        throw InvalidCastException("function", getTypeName());
}

inline Handler::Pointer
Register::toHandler() const
{
    return toObject()->getHandler();
}

inline bool
Register::toBoolean() const
{
    switch(type) {
        case NUMBER:
            return value.number != 0;
        case STRING:
            return !strPtr( )->empty();
        default:
            throw InvalidCastException("boolean", getTypeName());
    }
}
}

#endif

#include "reference-impl.h"
