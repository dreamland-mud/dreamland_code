/* $Id: register-decl.h,v 1.1.2.5.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: register-decl.h,v 1.1.2.5.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __REGISTER_DECL_H__
#define __REGISTER_DECL_H__

#include <list>

#include <dlstring.h>
#include <pointer.h>

#include "lex.h"
#include "handler.h"

namespace Scripting {

class Reference;
class Object;
class Closure;
class FeniaString;

class Register {
public:
    inline Register();
    inline Register(int i);
    inline Register(Lex::id_t i);
    inline Register(const DLString &s);
    inline Register(const string &s);
    inline Register(const char *c);
    inline Register(Closure *c);
    inline Register(Object *o);
    inline Register(const Register &r);
    template <typename T> 
        static inline Register handler();
    inline ~Register();

    inline const Register &operator = (int r);
    inline const Register &operator = (Lex::id_t r);
    inline const Register &operator = (const DLString &r);
    inline const Register &operator = (const char *r);
    inline const Register &operator = (Closure *c);
    inline const Register &operator = (Object *r);
    inline const Register &operator = (const Register &r);

    inline const Reference operator [] (const Register &r) const;
    
    inline const Register operator + (const Register &r) const;
    inline const Register operator * (const Register &r) const;
    inline const Register operator - (const Register &r) const;
    inline const Register operator / (const Register &r) const;
    inline const Register operator & (const Register &r) const;
    inline const Register operator | (const Register &r) const;
    inline const Register operator % (const Register &r) const;
    inline const Register operator ^ (const Register &r) const;
    inline const Register operator == (const Register &r) const;
    inline const Register operator != (const Register &r) const;
    inline const Register operator > (const Register &r) const;
    inline const Register operator < (const Register &r) const;
    inline const Register operator >= (const Register &r) const;
    inline const Register operator <= (const Register &r) const;
    
    inline const Register operator ! () const;
    inline const Register operator ~ () const;
    inline const Register operator - () const;
    
    inline const Register binminus (const Register &r) const;
    inline const Register unminus () const;

    inline int toNumber() const;
    inline Lex::id_t toIdentifier() const;
    inline DLString toString() const;
    inline Object *toObject() const;
    inline Closure *toFunction() const;
    inline Handler::Pointer toHandler() const ;
    inline bool toBoolean() const ;

    inline DLString repr() const;

    enum /*Type*/ {
        NONE,
        NUMBER,
        IDENTIFIER,
        STRING,
        OBJECT,
        FUNCTION,
    } type;

    inline const char *getTypeName() const {
        switch(type) {
            case NONE:                return "NONE";                break;
            case NUMBER:        return "NUMBER";        break;
            case IDENTIFIER:        return "IDENTIFIER";        break;
            case STRING:        return "STRING";        break;
            case OBJECT:        return "OBJECT";        break;
            case FUNCTION:        return "FUNCTION";        break;
            default:                return "<unknown>";        break;
        }
    }
    
    union {
        int number;
        Lex::id_t identifier;
        Closure *function;
        Object *object;
        char aux[sizeof(DLString)];   /*XXX: actualy, FeniaString*/
    } value;


    inline FeniaString *strPtr( );
    inline const FeniaString *strPtr( ) const;
    
private:
    inline void strNew( );

protected:
    inline void release();
    inline void safeSet(Closure *a);
    inline void safeSet(Object *a);
    inline void safeSet(const FeniaString &a);
    inline void safeSet(int a);
    inline void safeSet(Lex::id_t a);

    inline void set(int a);
    inline void set(Lex::id_t a);
    inline void set(const FeniaString &a);
    inline void set(Object *a);
    inline void set(Closure *a);
};

class RegisterList : public std::list<Register> { };


}

#endif
