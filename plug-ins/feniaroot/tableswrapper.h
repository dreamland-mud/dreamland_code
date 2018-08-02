/* $Id: tableswrapper.h,v 1.1.2.3.18.2 2007/09/11 00:09:28 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __TABLES_H__
#define __TABLES_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "flagtable.h"

#include "fenia/handler.h"

using Scripting::Register;
using Scripting::RegisterList;

class TableWrapper : public XMLVariableContainer, public virtual Scripting::Handler {
XML_OBJECT
public:
    typedef ::Pointer<TableWrapper> Pointer;
    
    TableWrapper();
    TableWrapper(const DLString &);

    virtual void setField(const Register &key, const Register &val);
    virtual Register getField(const Register &key);
    virtual Register callMethod(const Register &key, const RegisterList &args);
    void setSelf(Scripting::Object *);
    void resolveTab();

    const FlagTable *table;
    XML_VARIABLE XMLString tableName;
};

class TablesWrapper : public XMLVariableContainer, public virtual Scripting::Handler {
XML_OBJECT
public:
    typedef ::Pointer<TablesWrapper> Pointer;
    
    virtual void setField(const Register &key, const Register &val);
    virtual Register getField(const Register &key);
    virtual Register callMethod(const Register &key, const RegisterList &args);
    void setSelf(Scripting::Object *);

};


#endif /* __TABLES_H__ */
