/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __XMLEDITORINPUTHANDLER_H__
#define __XMLEDITORINPUTHANDLER_H__

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "xmlvariablecontainer.h"
#include "xmlpcstringeditor.h"
#include "descriptor.h"

class XMLEditorInputHandler : public InputHandler, public XMLPCStringEditor
{
XML_OBJECT
public:
    typedef ::Pointer<XMLEditorInputHandler> Pointer;

    virtual Descriptor *getOwner( );
    
    virtual void done();
    virtual void help() { }

    virtual int handle(Descriptor*, char *);
    virtual void prompt( Descriptor *d );

    virtual bool nodeFromXML( const XMLNode::Pointer & );
    virtual bool toXML( XMLNode::Pointer& ) const;
    
    void attach(Character *ch);
    
    Descriptor *owner;
};

#endif
