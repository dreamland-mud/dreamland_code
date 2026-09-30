/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          xmldocument.cpp  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <fstream>
#include <iostream>

#undef yyFlexLexer
#define yyFlexLexer xmlFlexLexer
#include <FlexLexer.h>

#include "xmldocument.h"
#include "xmlparser.h"
#include "logstream.h"
#include "iconvmap.h"

IconvMap koi2utf("koi8-u", "utf-8");

XMLDocument::XMLDocument( )
        : encoding( DEFAULT_ENCODING ),
                version( DEFAULT_VERSION )
{
}

DLString XMLDocument::encode(const DLString &str) const
{
    if(encoding == "UTF-8") {
        return DLString(koi2utf(str));
    } else {
        return str;
    }
}

/*
 * XML 1.0 forbids C0 control bytes other than tab/LF/CR, and they can't be
 * escaped either. Legacy data holds the colour push/pop markers as raw bytes
 * ({ followed by 0x01/0x02, from before mudtags used {1/{2); rewrite those to
 * the equivalent {1/{2 and drop any other control byte (e.g. ANSI escapes a
 * client sent into a channel), so every file we write parses as XML.
 */
static DLString sanitize_controls( const DLString &str )
{
    DLString out;
    char prev = 0;

    for (std::string::const_iterator i = str.begin( ); i != str.end( ); i++) {
        char ch = *i;

        if ((unsigned char)ch < ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
            if (prev == '{' && (ch == '\001' || ch == '\002'))
                out += (ch == '\001' ? '1' : '2');
            prev = 0;
            continue;
        }

        out += ch;
        prev = ch;
    }

    return out;
}

void XMLDocument::emit( const XMLNode &node, ostream& ostr, int space, bool& cdataPrev ) const
{
    const NodeList &nlist = node.getNodeList( );
    NodeList::const_iterator inode;
    
    for( inode = nlist.begin( ); inode != nlist.end( ); inode++ )
    {
        XMLNode::Pointer pnode = *inode;
        switch( pnode->getType( ) )
        {
        case XML_CDATA:
            ostr << "<![CDATA[" << sanitize_controls(encode(pnode->getCData( ))) << "]]>";
            cdataPrev = true;
            break;
        case XML_TEXT:
            {
                DLString str(sanitize_controls(encode(pnode->getCData( ))));

                for( std::string::const_iterator ipos = str.begin( );ipos != str.end( );ipos++ ) {
                    char ch = *ipos;
                    switch( ch ) {
                    // Only & and < must be escaped in XML text content; emit ', ", >
                    // raw so asave does not churn area/help files away from canonical ASCII.
                    case '&':   ostr << "&amp;";        break;
                    case '<':   ostr << "&lt;";        break;
                    default:    ostr << ch;
                    }
                }
            }
            cdataPrev = true;
            break;
        default:
            cdataPrev = false;
            ostr << endl;
            
            for( int i = 0; i < space; i++ ) 
                ostr << ' ';
            
            ostr << '<' << pnode->getName( );
            
            const AttributeListType &attrs = pnode->getAttributes( );
            AttributeListType::const_iterator iattr;
            
            // Attribute values: backslash keeps its legacy \\ escape (the parser
            // still reads old \" files), the rest are standard XML entities.
            for( iattr = attrs.begin( ); iattr != attrs.end( ); iattr++ ) {
                ostr << ' ' << iattr->first << "=\"";
                DLString val(sanitize_controls(encode(iattr->second)));
                for (std::string::const_iterator ipos = val.begin( ); ipos != val.end( ); ipos++) {
                    switch (*ipos) {
                    case '\\': ostr << "\\\\";   break;
                    case '&':  ostr << "&amp;";  break;
                    case '<':  ostr << "&lt;";   break;
                    case '"':  ostr << "&quot;"; break;
                    default:   ostr << *ipos;
                    }
                }
                ostr << '"';
            }
            
            if( pnode->getType( ) == XML_LEAF )
                ostr << "/>";
            else {
                ostr << '>';
                
                emit( **pnode, ostr, space + 2, cdataPrev );
                if( !cdataPrev ) {
                    ostr << endl;
                    
                    for( int i = 0; i < space; i++ ) 
                        ostr << ' ';
                }

                ostr << "</" << pnode->getName( ) << '>';
                cdataPrev = false;
            }
        }
    }
}

void XMLDocument::save( ostream& ostr ) const  
{
    ostr << "<?xml version=\"" << version << "\" encoding=\"" << encoding << "\"?>";
    bool cdataPrev = false;
    emit( *this, ostr, 0, cdataPrev );
    ostr << endl;
}

void XMLDocument::load( istream& istr ) 
{
    nodes.clear( );
    XMLDocument::Pointer document( this );
    XMLParser lex( document, &istr );

    lex.yylex( );
}
const char* const XMLDocument::DEFAULT_ENCODING = "UTF-8";
const char* const XMLDocument::DEFAULT_VERSION = "1.0";
        
