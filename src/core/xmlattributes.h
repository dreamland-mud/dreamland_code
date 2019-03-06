/* $Id: xmlattributes.h,v 1.5.2.14.6.1 2007/06/26 07:24:21 rufina Exp $
 *
 * ruffina, 2003
 * based on XMLAttributes by NoFate, 2001
 */
#ifndef XMLATTRIBUTES_H
#define XMLATTRIBUTES_H

#include "xmlmap.h"
#include "xmlpersistent.h"
#include "xmlattribute.h"
#include "dlstring.h"

class Character;
typedef XMLPersistent<XMLAttribute> XPAttribute;

class XMLAttributes : public XMLMapContainer<XPAttribute> 
{
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributes> Pointer;

        template<typename T>
        inline ::Pointer<T> getAttr( const DLString & attrString )
        {
            ::Pointer<T> attr;
            iterator ipos = find( attrString );
            
            if (ipos == end( )) {
                attr.construct( );
                (*this) [attrString].setPointer( *attr );
                attr->init( );

            } else {
                attr.setPointer( dynamic_cast<T *>( ipos->second.getPointer( ) ) );
            }
            
            return attr;
        }
        
        template<typename T>
        inline ::Pointer<T> findAttr( const DLString& attrString )
        {
            ::Pointer<T> attr;
            iterator ipos = find( attrString );
            
            if (ipos != end( )) 
                attr.setPointer( dynamic_cast<T *>( ipos->second.getPointer( ) ) );
            else
                attr = NULL;
            
            return attr;
        }
        
        template <typename ArgType>
        bool handleEvent( const ArgType &args )
        {
            iterator i, i_next;
            bool rc = false;

            for (i = begin( ); i != end( ); i = i_next) {
                i_next = i;
                i_next++;

                if (i->second->handleEvent( args ))
                    rc = true;
            }

            return rc;
        }

        void eraseAttribute( const DLString & );
        void addAttribute( XMLAttribute::Pointer , const DLString & );
        bool isAvailable( const DLString& ) const;

        virtual bool nodeFromXML( const XMLNode::Pointer & );
};

#endif
