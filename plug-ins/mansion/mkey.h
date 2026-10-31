/* $Id: mkey.h,v 1.1.2.4.6.3 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef MKEY_H
#define MKEY_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlvariablecontainer.h"

#include "xmlattribute.h"
#include "playerattributes.h"

#include "commandplugin.h"

class MKey : public CommandPlugin {
XML_OBJECT
public:
    typedef ::Pointer<MKey> Pointer;

    MKey( );

    virtual void run( Character*, const DLString& constArguments );
    
private:
    void doShow( Character *, DLString& );
    void doGrant( Character *, DLString& );
    void doRemove( Character *, DLString& );
    void usage( Character * );
    
    static const DLString COMMAND_NAME;
};

class XMLAttributeMansionKey : public RemortAttribute, 
                               public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeMansionKey> Pointer;

    // Key vnums as a Fenia .List(): the Fenia key maker reads them.
    virtual Scripting::Register toRegister() const;

    XML_VARIABLE XMLVectorBase<XMLInteger> keys;

};

#endif
