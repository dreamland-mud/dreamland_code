/* $Id$
 *
 * ruffina, 2004
 */
#ifndef XMLATTRIBUTERESTRING_H
#define XMLATTRIBUTERESTRING_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlmap.h"
#include "xmlattribute.h"
#include "playerattributes.h"

class Character;
class PCharacter;
class Object;

class XMLItemRestring : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLItemRestring> Pointer;
    
    void dress( Object *, PCharacter * ) const;

    XML_VARIABLE XMLStringNoEmpty name;
    XML_VARIABLE XMLStringNoEmpty shortDescr;
    XML_VARIABLE XMLStringNoEmpty longDescr;
    XML_VARIABLE XMLStringNoEmpty description;
};

class XMLAttributeRestring : public RemortAttribute, public XMLMapBase<XMLItemRestring> {
public:
    typedef ::Pointer<XMLAttributeRestring> Pointer;

    void dress( Object *, PCharacter *, const DLString & = DLString::emptyString ) const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
    static const DLString DEFAULT_KEYWORD;
};

void dress_created_item( int sn, Object *, Character *, const DLString & = DLString::emptyString );

#endif
