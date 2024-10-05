#ifndef XMLKILLSTATATTRIBUTE_H

#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "xmlcounter.h"
#include "xmlinteger.h"
#include "xmlenumeration.h"

/** Keeps track of killed mobs of each vnum, as well as kill counters by mob align. */
class XMLKillingAttribute : public virtual XMLAttribute, public XMLVariableContainer
{
XML_OBJECT    
public:        
    typedef ::Pointer<XMLKillingAttribute> Pointer;

    XMLKillingAttribute();
    virtual ~XMLKillingAttribute();

    virtual Scripting::Register toRegister() const;

    static const DLString TYPE;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }


    XML_VARIABLE XMLCounter vnum;
    XML_VARIABLE XMLEnumerationArray align;
};


#endif