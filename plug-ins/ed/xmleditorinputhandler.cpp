/* $Id$
 *
 * ruffina, 2004
 */

#include "xmleditorinputhandler.h"
#include "pcharacter.h"

/*-----------------------------------------------------------------------
 * XMLEditorInputHandler
 *-----------------------------------------------------------------------*/
Descriptor *
XMLEditorInputHandler::getOwner( )
{
    /*XXX - throw exception?*/
    return owner;
}

void
XMLEditorInputHandler::done()
{
//    getOwner( )->send("done called");
    getOwner( )->handle_input.pop_front( );
}

int
XMLEditorInputHandler::handle(Descriptor *d, char *cmd)
{
    ::Pointer<XMLEditorInputHandler> dummy(this);

    /*XXX - try/catch?*/
    owner = d;
    eval(cmd);
    owner = 0;

    return 0;
}

void 
XMLEditorInputHandler::prompt( Descriptor *d )
{
    XMLPCStringEditor::prompt( d );
}

void 
XMLEditorInputHandler::attach(Character *ch)
{
    Descriptor *d = ch->desc;
    
    if (!d)
	return;

    d->handle_input.push_front(this);
}


bool 
XMLEditorInputHandler::nodeFromXML( const XMLNode::Pointer & parent )
{
    return InputHandler::nodeFromXML(parent) || XMLPCStringEditor::nodeFromXML(parent);
}

bool
XMLEditorInputHandler::toXML( XMLNode::Pointer& parent ) const
{
    InputHandler::toXML(parent);
    XMLPCStringEditor::toXML(parent);
    return true;
}

