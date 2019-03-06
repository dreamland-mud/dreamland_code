/* $Id: xmlstreamable.h,v 1.1.2.5.18.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __XMLSTREAMABLE_H__
#define __XMLSTREAMABLE_H__

#include <sstream>

#include "xmlpointer.h"
#include "xmlpersistent.h"
#include "xmldocument.h"

// MOC_SKIP_BEGIN

template <typename Ptr>
class XMLStreamableBase : public Ptr {
public:
    XMLStreamableBase( )
    {
    }
    XMLStreamableBase( const DLString & name ) : nodeName( name )
    {
    }

    void fromStream( std::istream& istr )
    {
        XMLDocument::Pointer root( NEW );

        root->load( istr );
        fromXML( root->getFirstNode( ) );
    }
    
    void toStream( std::ostream& ostr ) const
    {
        XMLNode::Pointer node( NEW );
        XMLDocument::Pointer root( NEW );

        if (toXML( node )) {
            node->setName( nodeName );
            root->appendChild( node );
            root->save( ostr );
        }
    }

    using Ptr::toXML;
    using Ptr::fromXML;
    using Ptr::operator =;

    const DLString nodeName;
};

template <typename T>
class XMLPersistentStreamable :  public XMLStreamableBase<XMLPersistent<T> > 
{
    typedef XMLStreamableBase<XMLPersistent<T> > Parent;
public:
    XMLPersistentStreamable( ) { }
    XMLPersistentStreamable( const DLString & name ) : Parent( name )
    {
    }
};

template <typename T>
class XMLStreamable : public XMLStreamableBase<XMLPointer<T> >
{
    typedef XMLStreamableBase<XMLPointer<T> > Parent;
public:
    XMLStreamable( ) { }
    XMLStreamable( const DLString & name ) : Parent( name )
    {
    }
};

// MOC_SKIP_END

#endif
