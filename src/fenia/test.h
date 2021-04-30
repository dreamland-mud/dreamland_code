/* $Id: test.h,v 1.1.4.6.6.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: test.h,v 1.1.4.6.6.3 2009/11/02 13:48:11 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __TEST_H__
#define __TEST_H__

#include "handler.h"
#include "context.h"
#include "xmlregister.h"
#include "codesource.h"
#include "object.h"
#include "function.h"
#include "native.h"
#include "manager-impl.h"

/*just to make moc happy*/
#include "register-impl.h"

using namespace Scripting;

class TestManager : public Object::Manager, public CodeSource::Manager {
public:
    TestManager() {
    }
    virtual ~TestManager() {
    }

    void open( ) {
        Object::Manager::open( );
        CodeSource::Manager::open( );
    }
    void close( ) {
        Object::Manager::close( );
        CodeSource::Manager::close( );
    }
    void load( ) {
        Object::Manager::load( );
        CodeSource::Manager::load( );
    }

    virtual DbEnvContext *getDbEnv( ) const;

    static DbEnvContext dbEnv;
};

class Test : public Context, public NativeImpl<Test>, public NativeHandler, public XMLVariableContainer
{
XML_OBJECT
NMI_OBJECT
public:
    Test();
    virtual ~Test();

    void load();
    void save();

    virtual void setSelf(Object *s) { 
        self = s;
    }
    virtual Scripting::Object *getSelf() const { return self; }

    Object *self;

    TestManager manager;
};

#endif

