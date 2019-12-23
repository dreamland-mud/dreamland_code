/* $Id: lex.h,v 1.1.2.3.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: lex.h,v 1.1.2.3.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

// MOC_SKIP_BEGIN
#ifndef __LEX_H__
#define __LEX_H__

#include <map>

#include <dlstring.h>
#include <oneallocate.h>
#include <xmlvariable.h>

using namespace std;

namespace Scripting {

#define ID_UNDEF    0
#define ID_THIS            1
#define ID_ORIGIN   2 // parser's "this"
#define ID_AUTO            100

class Lex : virtual public DLObject, public OneAllocate {
public:
    typedef unsigned long int id_t;

    Lex() : lastid(ID_AUTO) {
        self = this;
    }
    ~Lex() {
        self = 0;
    }
    
    const DLString &getName(id_t id);
    id_t resolve(const DLString &s);
    
    static Lex *getThis();

private:
    std::map<id_t, DLString> id2str;
    std::map<DLString, id_t> str2id;
    id_t lastid;
    
    static Lex *self;
};

class Register;

class IdRef {
public:
    IdRef(const DLString &n) : name(n), id(ID_UNDEF) { }
    
    operator Register ();

    DLString name;
    Lex::id_t id;
};

class XMLIdentifier : public XMLVariable {
public:
    XMLIdentifier() : value(0) { }
    XMLIdentifier(Lex::id_t id) : value(id) { }

    virtual void fromXML( const XMLNode::Pointer& parent ) ;
    virtual bool toXML( XMLNode::Pointer& node ) const;

    Lex::id_t getValue() const {
        return value;
    }
    void setValue(Lex::id_t id) {
        value = id;
    }
private:
    Lex::id_t value;
};

}

#endif /* __LEX_H__ */

// MOC_SKIP_END
