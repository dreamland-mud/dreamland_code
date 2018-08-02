/* $Id: xmlattributelovers.h,v 1.1.2.10.10.2 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTELOVERS_H
#define XMLATTRIBUTELOVERS_H

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "playerattributes.h"

#include "xmllovers.h"

class XMLAttributeLovers : 
    public RemortAttribute, public XMLVariableContainer 
{
XML_OBJECT
public: 
        typedef ::Pointer<XMLAttributeLovers> Pointer;

	XMLAttributeLovers( );
	virtual ~XMLAttributeLovers( );

	XML_VARIABLE XMLLovers lovers;
};

#endif

