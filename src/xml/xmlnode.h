/* $Id: xmlnode.h,v 1.8.34.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          xmlnode.h  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef XMLNODE_H
#define XMLNODE_H

#include <map>
#include <vector>

#include "pstream.h"

#include "dlobject.h"
#include "dlstring.h"
#include "pointer.h"
#include "exceptionxsl.h"


/**
 * @author Igor S. Petrenko
 * @short XML "единица"
 */
class XMLNode : public virtual DLObject
{
public:
        typedef ::Pointer<XMLNode> Pointer;

public:
        static const DLString ATTRIBUTE_TYPE;
        static const DLString ATTRIBUTE_NODE;
        static const DLString ATTRIBUTE_NAME;

public:
        /**
         * @author Igor S. Petrenko
         * @short список in nodes
         */
        struct NodeList : public std::vector<XMLNode::Pointer>, public virtual DLObject
        {
                typedef ::Pointer<NodeList> Pointer;
        };

        typedef std::map<DLString, DLString> AttributeListType;
        typedef char NodeType;

public:
        static const char XML_NODE = 0;
        static const char XML_LEAF = 1;
        static const char XML_CDATA = 2;
        static const char XML_TEXT = 3;
        
public:
        XMLNode( );
        XMLNode( const DLString& name );
        virtual ~XMLNode( );
        
        inline NodeType getType( ) const
        {
                return type;
        }
        
        inline void setType( NodeType type )
        {
                this->type = type;
        }
        
        inline const DLString& getCData( ) const
        {
                return name;
        }
        
        inline void setCData( const DLString& cdata )
        {
                this->name = cdata;
        }
        
        inline const DLString& getName( ) const
        {
                return name;
        }
        
        inline void setName( const DLString& name )
        {
                this->name = name;
        }

        void appendChild( Pointer& node );
        inline Pointer getFirstNode( )
        {
                if( nodes.empty( ) )
                {
                        return Pointer( );
                }
                else
                {
                        return *nodes.begin( );
                }
        }
        
        inline const Pointer getFirstNode( ) const
        {
                if( nodes.empty( ) )
                {
                        return Pointer( );
                }
                else
                {
                        return *nodes.begin( );
                }
        }
        
        inline const NodeList& getNodeList( ) const
        {
                return nodes;
        }
        
        inline void insertAttribute( const DLString& key, const DLString& value )
        {
                attributes[key] = value;
        }
        
        inline const DLString& getAttribute( const DLString& name ) const
        {
                AttributeListType::const_iterator pos = attributes.find( name );
                if( pos != attributes.end( ) )
                {
                        return pos->second;
                }
                else
                {
                        return DLString::emptyString;
                }
        }
        void getAttribute(const DLString &name, int &value) const;

        inline bool hasAttribute( const DLString &name ) const
        {
                AttributeListType::const_iterator pos = attributes.find( name );
                return pos != attributes.end( );
        }
        
        inline const AttributeListType& getAttributes( ) const
        {
                return attributes;
        }
        
        NodeList::Pointer getElementsByTagName( const DLString& name ) const;
        NodeList::Pointer selectNodes( const DLString& pattern ) const ;
        Pointer selectSingleNode( const DLString& pattern ) const ;
        
        void erase( const Pointer& node );
        bool equal( const XMLNode & ) const;        
        Pointer clone( bool recursive ) const;
        Pointer getTopNode( ) const;
        bool hasNoData( ) const;
        
        inline Pointer getParent( ) const
        {
                return parent;
        }
        
        
protected:
        DLString name;
        AttributeListType attributes;
        NodeList nodes;
        NodeType type;
        XMLNode* parent;
};

opstream &operator << (opstream &, const XMLNode &);
ipstream &operator >> (ipstream &, XMLNode &);


#endif
