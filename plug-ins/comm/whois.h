/* $Id: whois.h,v 1.1.2.3.10.2 2007/09/11 00:27:23 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef WHOIS_H
#define WHOIS_H

#include <sstream>

#include "commandplugin.h"
#include "defaultcommand.h"

using std::endl;

class Whois : public CommandPlugin {
XML_OBJECT
public:
        class LinesList : public std::vector<DLString> {
        public:
            void add( std::basic_ostringstream<char> &, bool = true );
            void addNoCR( std::basic_ostringstream<char> & );
        };
        
        typedef ::Pointer<Whois> Pointer;
    
        Whois( );

        virtual void run( Character*, const DLString& constArguments );
        
private:
        static const DLString COMMAND_NAME;
                
};

#endif

