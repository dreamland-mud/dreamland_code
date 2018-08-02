/* $Id: xmlattributecodesource.h,v 1.1.2.3.6.1 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef XMLATTRIBUTECODESOURCE_H
#define XMLATTRIBUTECODESOURCE_H

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "xmlvariablecontainer.h"

#include "xmlstring.h"
#include "xmlvector.h"
#include "xmled.h"

class XMLAttributeCodeSource : public XMLAttribute, public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeCodeSource> Pointer;
  
    XMLAttributeCodeSource( );
    virtual ~XMLAttributeCodeSource( );

    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLVectorBase<XMLString> content;
};

#endif

