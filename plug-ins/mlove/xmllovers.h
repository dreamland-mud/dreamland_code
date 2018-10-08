/* $Id: xmllovers.h,v 1.1.2.3.10.2 2009/01/01 14:13:18 rufina Exp $
 * ruffina, 2003
 */

#ifndef XMLLOVERS_H
#define XMLLOVERS_H

#include "xmlattribute.h"
#include "xmlmap.h"
#include "xmlstring.h"

class XMLLovers : public XMLMapBase<XMLString>
{
XML_OBJECT;
public:
        XMLLovers ();
        ~XMLLovers ();

public:                 

        void put( const DLString );
        void erase( const DLString );
        bool isPresent( const DLString );
};

#endif

