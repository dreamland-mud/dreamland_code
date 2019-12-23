/* $Id: tableswrapper.cpp,v 1.1.2.4.18.4 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sstream>

using namespace std;

#include "lex.h"
#include "register-impl.h"
#include "exceptions.h"
#include "wrap_utils.h"
#include "tableswrapper.h"
#include "flagtable.h"
#include "flagtableregistry.h"

using namespace Scripting;

static IdRef ID_API("api");
static IdRef ID_VALUE("value");
static IdRef ID_VALUES("values");
static IdRef ID_NAME("name");
static IdRef ID_NAMES("names");
static IdRef ID_MESSAGE("message");
static IdRef ID_MESSAGES("messages");

    
TableWrapper::TableWrapper() : table(0)
{
}

TableWrapper::TableWrapper(const DLString &t) : table(0), tableName(t)
{
}

void 
TableWrapper::setField(const Register &key, const Register &val)
{
    throw Scripting::Exception("assignment of constant variable requested");
}

void 
TableWrapper::resolveTab()
{
    if(table == 0) {
        table = FlagTableRegistry::getTable( tableName );
        
        /* (in)sanity check. be even more paranoid. */
        if(table == 0)
            throw Scripting::Exception("no such table defined in bits.conf");
    }
}

Register 
TableWrapper::getField(const Register &key)
{
    const DLString &flag = Lex::getThis()->getName(key.toIdentifier());

    resolveTab();

    int rc;
    
    if (table->enumerated)
        rc = table->value( flag, true );
    else
        rc = table->bitstring( flag, true );

    if(rc == NO_FLAG)
        throw Scripting::Exception("no such flag defined in bits.conf");

    return rc;
}

Register 
TableWrapper::callMethod(const Register &key, const RegisterList &args)
{
    ostringstream os;

    resolveTab();

    if( (key == ID_API).toBoolean() ) {
        const FlagTable::Field * f = table->fields;
        
        if(f == NULL)
            throw Scripting::Exception("no flags in this table");

        for(int i = 0; i < table->size; i++) {
            os << "{g" << f[i].name << "{x";
            
            if(f[i].message)
                os << ": " << f[i].message;

            os << endl;
        }

        return Register( os.str() );
    }

    if (( key == ID_VALUE ).toBoolean( ) )
        return Register((int)table->value(args2string(args)));

    if (( key == ID_VALUES ).toBoolean( )) 
        return Register((int)table->bitstring(args2string(args)));

    if (( key == ID_NAME ).toBoolean( ) )
        return Register(table->name(args2number(args)));

    if (( key == ID_NAMES ).toBoolean( ) )
        return Register(table->names(args2number(args)));

    if (( key == ID_MESSAGE ).toBoolean( ) ) {
        int gcase = args.size() > 1 ? argnum2number(args, 2) : 1;
        return Register(table->message(argnum2number(args, 1), gcase + '0'));
    }

    if (( key == ID_MESSAGES ).toBoolean( ) ) {
        int gcase = args.size() > 1 ? argnum2number(args, 2) : 1;
        return Register(table->messages(argnum2number(args, 1), true, gcase + '0'));
    }

    throw Scripting::Exception("no such method in this object");
}

void 
TableWrapper::setSelf(Scripting::Object *)
{
}

////////////////////////////////////////

void 
TablesWrapper::setField(const Register &key, const Register &val)
{
    throw Scripting::Exception("assignment of constant variable requested");
}

Register 
TablesWrapper::getField(const Register &key)
{
    const DLString &table = Lex::getThis()->getName(key.toIdentifier());
    
    const FlagTable * t = FlagTableRegistry::getTable( table );
    
    if(t == 0)
        throw Scripting::Exception("no such table defined in bits.conf");

    TableWrapper::Pointer twp(NEW, table);
    Scripting::Object &obj = Scripting::Object::manager->allocate();

    obj.setHandler(twp);

    return &obj;
}

Register 
TablesWrapper::callMethod(const Register &key, const RegisterList &args)
{
    if( (key != ID_API).toBoolean() )
        throw Scripting::Exception("no such method in this object");

    ostringstream os;
    FlagTableRegistry::NamesMap::const_iterator n;
    const FlagTableRegistry::NamesMap &names = FlagTableRegistry::getNamesMap( );
    
    for (n = names.begin( ); n != names.end( ); n++)
        os << "{g" << n->first << "{x" << endl;

    return Register( os.str() );
}

void 
TablesWrapper::setSelf(Scripting::Object *)
{
}

