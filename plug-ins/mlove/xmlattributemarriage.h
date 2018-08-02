/* $Id: xmlattributemarriage.h,v 1.1.2.9.10.3 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEMARRIAGE_H
#define XMLATTRIBUTEMARRIAGE_H

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "xmlvector.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "playerattributes.h"

class XMLAttributeMarriage : 
   public EventHandler<WhoisArguments>,
   public XMLVariableContainer,
   public RemortAttribute
{
XML_OBJECT
public: 
        typedef ::Pointer<XMLAttributeMarriage> Pointer;

	XMLAttributeMarriage( );

	virtual bool handle( const WhoisArguments & );

	XML_VARIABLE XMLString spouse;
	XML_VARIABLE XMLBoolean wife;
	XML_VARIABLE XMLVectorBase<XMLString> history;

};

#endif

