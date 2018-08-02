/* $Id: xmlattributecoder.h,v 1.1.2.1 2007/09/11 00:22:30 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef XMLATTRIBUTECODER_H
#define XMLATTRIBUTECODER_H

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "xmlvariablecontainer.h"
#include "playerattributes.h"

class XMLAttributeCoder : public RemortAttribute, 
			  public XMLVariableContainer
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeCoder> Pointer;

	XMLAttributeCoder( );
	virtual ~XMLAttributeCoder( );
};

#endif

