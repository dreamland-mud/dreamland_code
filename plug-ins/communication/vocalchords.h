/* $Id: vocalchords.h,v 1.1.2.1 2007/05/02 03:05:09 rufina Exp $
 *
 * ruffina, 2007
 */
#ifndef __VOCALCHORDS_H__
#define __VOCALCHORDS_H__

#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"

class XMLAttributeVocalChords : public XMLAttribute, XMLVariableContainer {
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeVocalChords> Pointer;

	XMLAttributeVocalChords( );
	virtual ~XMLAttributeVocalChords( );

	XML_VARIABLE XMLInteger maxIC, coefIC, nowIC;
	XML_VARIABLE XMLInteger maxOOC, coefOOC, nowOOC;
};
/*
 * grats       1 
 * music       2 
 * gossip      3 
 * ic          4 
 * page all    5 
 * 
 * ooc         4
 *
 */
#endif

