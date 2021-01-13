/* $Id$
 *
 * ruffina, 2004
 */
#ifndef XMLAREAHELP_H
#define XMLAREAHELP_H

#include "xmlstring.h"
#include "xmlvariablecontainer.h"

class XMLAreaHelp : public XMLString {
XML_OBJECT
public:
    typedef ::Pointer<XMLAreaHelp> Pointer;

    XMLAreaHelp();
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& );

    DLString keywordAttribute;
    int level;
    DLString labels;
    int id;
    DLString titleAttribute;
};

#endif
