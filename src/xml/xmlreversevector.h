/* $Id: xmlreversevector.h,v 1.1.2.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef XMLREVERSEVECTOR_H
#define XMLREVERSEVECTOR_H

#include <sstream>
#include "xmlvector.h"

template <typename T>
class XMLReverseVector : public XMLVectorBase<T> {
public:
    using std::vector<T>::back;
    using std::vector<T>::size;
    using std::vector<T>::begin;
    using std::vector<T>::at;
    using std::vector<T>::push_back;
    using std::vector<T>::erase;

    virtual void fromXML( const XMLNode::Pointer& node ) 
    {
        reverse.clear( );
        XMLVectorBase<T>::fromXML( node );
    }
    virtual bool nodeFromXML( const XMLNode::Pointer& child ) 
    {
        if (!XMLVectorBase<T>::nodeFromXML( child ))
            return false;

        reverse[back( )] = size( ) - 1;
        return true;
    }
    bool hasElement( const T &element ) const 
    {
        return getIndexOf( element ) != -1;
    }
    int getIndexOf( const T &element ) const
    {
        typename Reverse::const_iterator r = reverse.find( element );
        if (r == reverse.end( ))
            return -1;
        else
            return r->second;
    }
    void remove( const T &element ) 
    {
        typename Reverse::iterator r = reverse.find( element );
        if (r != reverse.end( )) {
            erase( begin( ) + r->second );
            reverse.erase( r );
        }
    }
    void add( const T &element )
    {
        if (!hasElement( element )) {
            push_back( element );
            reverse[back( )] = size( ) - 1;
        }
    }
    DLString toString( ) const
    {
        ostringstream buf;
        
        for (unsigned int i = 0; i < size( ); i++) {
            if (!buf.str( ).empty( ))
                buf << ", ";
            buf << at( i );
        }

        return buf.str( );
    }
    
private:
    typedef std::map<T, int> Reverse;
    Reverse reverse;
};

#endif

