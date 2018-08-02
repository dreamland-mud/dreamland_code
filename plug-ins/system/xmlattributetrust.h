/* $Id$
 *
 * ruffina, 2004
 */
#ifndef XMLATTRIBUTETRUST_H
#define XMLATTRIBUTETRUST_H

#include <sstream>
#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "xmlboolean.h"
#include "xmlglobalbitvector.h"
#include "xmlreversevector.h"
#include "xmlstring.h"

class Character;
class PCharacter;

class XMLAttributeTrust : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeTrust> Pointer;
    
    XMLAttributeTrust( );
    virtual ~XMLAttributeTrust( );
    
    bool parse( const DLString &, ostringstream & );
    bool check( Character * ) const;
    bool checkAllow( PCharacter * ) const;
    bool checkDeny( PCharacter * ) const;
    
protected:
    XML_VARIABLE XMLBoolean all;
    XML_VARIABLE XMLGlobalBitvector clansAllow, clansDeny;
    XML_VARIABLE XMLReverseVector<XMLString> playersAllow, playersDeny;
};

#endif
