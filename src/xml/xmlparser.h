/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          xmlparser.h  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <sstream>
#include <iostream>
#include <stack>

#include "xmldocument.h"
#include "dlstring.h"

/**
 * @author Igor S. Petrenko
 * @short Загрузка XML с помощью flex
 */
class XMLParser : public yyFlexLexer, public virtual DLObject
{
public:
	XMLParser( XMLDocument::Pointer& document, istream* );
	int yylex( );
	virtual ~XMLParser( );
	
private:
	typedef std::stack<XMLNode::Pointer> StackType;

        DLString decode(const DLString &) const;
	
	
	XMLDocument::Pointer root;
	std::basic_ostringstream<char> ostr;

	XMLDocument::Pointer rootNode;
	XMLNode::Pointer currentNode;
	DLString value;
	DLString space;
	DLString attributeName;
	DLString attributeValue;

	StackType st;
};

#endif
