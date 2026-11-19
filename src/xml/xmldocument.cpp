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
#include "dl_ctype.h"

IconvMap koi2utf("koi8-u", "utf-8");

XMLDocument::XMLDocument( )
        : encoding( DEFAULT_ENCODING ),
                version( DEFAULT_VERSION ),
                modifierApostrophe( false )
{
}

/*
 * Nearest character left of i, skipping colour codes ({ plus one character)
 * and inflection pipes. Returns 0 at the start of the string.
 */
static char apostrophe_left( const DLString &str, size_t i )
{
    size_t j = i;

    while (j > 0) {
        j--;
        if (j >= 1 && str[j - 1] == '{') {
            j--;
            continue;
        }
        if (str[j] == '|')
            continue;
        return str[j];
    }

    return 0;
}

/* Same to the right. A lone { at the very end is an ordinary character. */
static char apostrophe_right( const DLString &str, size_t i )
{
    size_t j = i + 1;

    while (j < str.size( )) {
        if (str[j] == '{' && j + 1 < str.size( )) {
            j += 2;
            continue;
        }
        if (str[j] == '|') {
            j++;
            continue;
        }
        return str[j];
    }

    return 0;
}

/*
 * Area files spell the Ukrainian apostrophe as U+02BC. Load turns it into an
 * ASCII ' (iconvmap.cpp, KOI8-U has no such character), so the save puts it
 * back: an ASCII ' between two Cyrillic letters, colour codes and pipes
 * skipped. Quotes and English text keep the ASCII one. Works on KOI8 bytes.
 */
static bool is_modifier_apostrophe( const DLString &str, size_t i )
{
    if (str[i] != '\'')
        return false;

    char l = apostrophe_left( str, i );
    char r = apostrophe_right( str, i );
    return l && r && dl_is_cyrillic( l ) && dl_is_cyrillic( r );
}

/*
 * U+02BC is not a KOI8-U character, so it cannot go through koi2utf. The
 * string is cut at each such apostrophe, every piece is converted on its own
 * and the UTF-8 bytes of U+02BC are glued in between. KOI8-U is a stateless
 * single-byte charset, so converting the pieces equals converting the whole.
 */
DLString XMLDocument::encode(const DLString &str) const
{
    if (encoding != "UTF-8")
        return str;

    if (!modifierApostrophe || str.find( '\'' ) == DLString::npos)
        return DLString(koi2utf(str));

    static const char *const MODIFIER_APOSTROPHE_UTF8 = "\xCA\xBC";
    std::string out;
    size_t start = 0;

    for (size_t i = 0; i < str.size( ); i++) {
        if (!is_modifier_apostrophe( str, i ))
            continue;

        out += koi2utf(str.substr( start, i - start ));
        out += MODIFIER_APOSTROPHE_UTF8;
        start = i + 1;
    }

    out += koi2utf(str.substr( start ));
    return DLString(out);
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
        
