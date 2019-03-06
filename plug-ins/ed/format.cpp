/* $Id$
 *
 * ruffina, 2004
 */

#include <iomanip>
#include "format.h"

/*------------------------------------------------------------------
 * text formatting
 *------------------------------------------------------------------*/

Formatter::Formatter( ostream &o, istream &i ) : os(o), is(i)
{
}

Formatter::~Formatter( )
{
}

bool
Formatter::is_separator(char c)
{
    switch(c) {
        case ' ':
            return true;
        case '\r':
        case '\n':
        case '\t':
            return true;
        default:
            return false;
    }
}

bool
Formatter::getword(DLString &word)
{
    word.clear( );
    int c;

    for(;;) {
        c = is.get( );
        
        if(c < 0)
            return false;

        if(is_separator(c))
            break;        
        
        if(c < ' ')
            continue;
        
        word += (char) c;
    }

    is.unget( );

    return true;
}

bool
Formatter::skipspaces( )
{
    int c;

    for(;;) {
        c = is.get( );
        
        if(c < 0)
            return false;

        if(!is_separator(c))
            break;
    }

    is.unget( );

    return true;
}


void 
Formatter::printline(unsigned int tab, unsigned int width)
{
    words_t::const_iterator i;
    int wc = words.size( );
    int spaces = width - tab;
    
    if(wc == 0)
        return;

    if(tab)
        os << setw(tab) << "";
    
    for(i = words.begin( ); i != words.end( ); i++) 
        spaces -= i->colorLength( );

    for(i = words.begin( ); i != words.end( ); i++) {
        if(i != words.begin( )) {
            int pad = ( spaces + wc/2 )/wc;
            os << setw( pad ) << " ";
            spaces -= pad;
        }
        os << *i;
        wc--;
    }
    os << endl;
}

void
Formatter::format(unsigned int tab, unsigned int width)
{
    unsigned int len;

    for(len = tab; !is.eof( ); len++) {
        DLString word;

        skipspaces( ) && getword(word);

        if(len + word.colorLength( ) > width) {
            if(!word.empty( ) && words.empty( )) {
                words.push_back(word);
                word.clear( );
            }

            printline(tab, width);
            words.clear( );
            len = tab = 0;
        }
        
        if(!word.empty( )) {
            len += word.colorLength( );
            words.push_back( word );
        }
    }

    printline(tab, 0);
}


