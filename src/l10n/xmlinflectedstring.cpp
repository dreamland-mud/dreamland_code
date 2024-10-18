/* $Id: xmlinflectedstring.cpp,v 1.1.2.5 2009/11/08 17:33:28 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <string.h>
#include "grammar_entities_impl.h"
#include "xmlinflectedstring.h"

using namespace Grammar;

const DLString XMLInflectedString::ATTRIBUTE_GRAMMAR = "mg";

void XMLInflectedString::fromXML( const XMLNode::Pointer& parent ) 
{
    XMLNode::Pointer node = parent->getFirstNode( );
    
    if (parent->hasAttribute( ATTRIBUTE_GRAMMAR ))
        mg = MultiGender(parent->getAttribute( ATTRIBUTE_GRAMMAR ).c_str());

    if (!node.isEmpty( )) 
        setFullForm(node->getCData( ));
}

bool XMLInflectedString::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );
    
    node->setType( XMLNode::XML_TEXT );
    node->setCData( getFullForm() );
    
    if (mg != MultiGender::UNDEF)
        parent->insertAttribute( ATTRIBUTE_GRAMMAR, mg.toString() );

    parent->appendChild( node );
    return true;
}

