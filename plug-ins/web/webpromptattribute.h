/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBPROMPTATTRIBUTE_H
#define WEBPROMPTATTRIBUTE_H

#include "xmlmap.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"
#include "xmlattribute.h"

class WebPromptAttribute : public XMLAttribute, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<WebPromptAttribute> Pointer;
    
    virtual ~WebPromptAttribute( );
    virtual void init( );

    XML_VARIABLE XMLMapBase<XMLString> prompt;

    void clear( );

};

#endif
