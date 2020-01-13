/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          xmlmatchpattern.h  -  description
                             -------------------
    begin                : Tue Oct 16 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef        XMLMATCHPATTERN_H
#define        XMLMATCHPATTERN_H

#include <sstream>

#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short Поиск по xml документу
 */
class XMLMatchPattern : public yyFlexLexer
{
public:
        typedef XMLNode::NodeList NodeList;
        typedef XMLNode::NodeList::Pointer NodeListPointer;

public:
        XMLMatchPattern( const XMLNode::Pointer node, const DLString& pattern );
        virtual ~XMLMatchPattern( );
        
        virtual int yylex( ) ;
        
        inline NodeListPointer getList( )
        {
                return list;
        }

private:
        static const char EQUAL = 1;
        static const char NOT_EQUAL = 2;
        static const char LESS = 3;
        static const char LESS_OR_EQUAL = 4;
        static const char MORE = 5;
        static const char MORE_OR_EQUAL = 6;

private:
        XMLNode::Pointer root;
        DLString pattern;
        NodeList::Pointer list;
        std::basic_ostringstream<char> ostr;
        std::basic_istringstream<char> istr;
        
        DLString firstVariable;
        DLString secondVariable;
        char operation;
        
        int position;

private:
        NodeListPointer lookupArgument( const NodeListPointer list );
        NodeListPointer lookupName( const DLString& name, const NodeListPointer list );
        void listDown( );
                
};


#endif
