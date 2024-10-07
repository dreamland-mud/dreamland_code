/* $Id$
 *
 * ruffina, 2004
 */
#ifndef XMLAREAHELP_H
#define XMLAREAHELP_H

#include "xmlstring.h"
#include "xmlvariablecontainer.h"
#include "xmlmultistring.h"

class XMLAreaHelp : public DLString, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAreaHelp> Pointer;

    XMLAreaHelp();
    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& ) ;

    XML_VARIABLE XMLMultiString keyword;
    XML_VARIABLE XMLMultiString title;
    XML_VARIABLE XMLMultiString extra;
    XML_VARIABLE XMLMultiString text;

    int level;
    DLString labels;
    int id;
};

#endif
