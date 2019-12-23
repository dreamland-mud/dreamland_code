/* $Id: xmllong.h,v 1.6.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// xmllong.h: interface for the XMLLong class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLLONG_H
#define XMLLONG_H

#include "long.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная long
 */
class XMLLong : public Long
{
public:
        /** По умолчанию значение \0 */
        inline XMLLong( ) : Long( )
        {
        }
        
        inline XMLLong( long value ) : Long( value )
        {
        }
        
        inline XMLLong( const DLString& value ) 
                : Long( value )
        {
        }
        
        /** Возвращает xml представление переменной */
        bool toXML( XMLNode::Pointer& node ) const;
        /** Инициализация класса из xml данных */
        void fromXML( const XMLNode::Pointer& node ) ;
};


class XMLLongNoEmpty : public XMLLong {
public:
        inline XMLLongNoEmpty( )
        {
        }
        
        inline XMLLongNoEmpty( int value ) : XMLLong( value )
        {
        }

        bool toXML( XMLNode::Pointer& node ) const;
};



/** Вывод в ostream типа XMLLong */
inline std::ostream& operator << ( std::ostream& ostr, const XMLLong& xmlLong )
{
        ostr << xmlLong.getValue( );
        return ostr;
}

#endif
