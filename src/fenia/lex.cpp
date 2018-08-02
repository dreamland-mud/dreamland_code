/* $Id: lex.cpp,v 1.1.2.3.18.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: lex.cpp,v 1.1.2.3.18.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <iostream>

using namespace std;

#include "lex.h"
#include "register-impl.h"

namespace Scripting {

Lex *Lex::self = 0;

const DLString &
Lex::getName(id_t id) 
{
    const DLString &rc = id2str[id];
    return rc;
}

Lex::id_t 
Lex::resolve(const DLString &s) 
{
    std::map<DLString, id_t>::iterator i = str2id.find(s);
    
    if(i != str2id.end())
	return i->second;
    
    str2id[s] = lastid;
    id2str[lastid] = s;

    return lastid++;
}

Lex *
Lex::getThis()
{
    if(!self) {
	new Lex();
	/*XXX - ?*/
	self->id2str[ID_THIS] = "this";
	self->str2id["this"] = ID_THIS;
	self->id2str[ID_ORIGIN] = "origin";
	self->str2id["origin"] = ID_ORIGIN;
    }
    
    return self;
}

IdRef::operator Register ()
{
    if(id == ID_UNDEF)
	id = Lex::getThis()->resolve(name);

    return Register(id);
}

void
XMLIdentifier::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType ) 
{
    XMLNode::Pointer node = parent->getFirstNode( );

    if( node && (node->getType( ) == XMLNode::XML_CDATA || 
		 node->getType( ) == XMLNode::XML_TEXT) )
    {
	value = Lex::getThis()->resolve(node->getCData( ));
    }
}

bool
XMLIdentifier::toXML( XMLNode::Pointer& parent ) const 
{
    XMLNode::Pointer node( NEW );
    
    node->setType( XMLNode::XML_TEXT );
    node->setCData( Lex::getThis()->getName(value) );
    
    parent->appendChild( node );
    return true;
}

}
