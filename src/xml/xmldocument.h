/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          xmldocument.h  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include <iostream>

#include "xmlnode.h"
#include "dlstring.h"
#include "exceptionxmlerror.h"

using namespace std;

/**
 * @author Igor S. Petrenko
 */
class XMLDocument : public XMLNode
{
public:
        typedef ::Pointer<XMLDocument> Pointer;

public:
        static const char* const DEFAULT_ENCODING ;
        static const char* const DEFAULT_VERSION ;
        
public:
        XMLDocument( );

        inline void setVersion( DLString version )
        {
                this->version = version;
        }
        
        inline const DLString& getVersion( ) const
        {
                return version;
        }
        
        inline void setEncoding( DLString encoding )
        {
                this->encoding = encoding;
        }
        
        inline const DLString& getEncoding( ) const
        {
                return encoding;
        }
        
        inline XMLNode::Pointer getDocumentElement( )
        {
                if( nodes.empty( ) ) return XMLNode::Pointer( );
                else return *nodes.begin( );
        }
        
        void save( ostream& ) const throw( ExceptionXMLError );
        void load( istream& ) throw( ExceptionXMLError );
private:
        DLString encode(const DLString &) const;
        void emit( const XMLNode &, ostream&, int, bool& ) const;
        
private:
        DLString encoding;
        DLString version;
};

#endif
