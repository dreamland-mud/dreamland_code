/* $Id: xmllonglong.h,v 1.6.2.2.28.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// xmllonglong.h: interface for the XMLLongLong class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLLONGLONG_H
#define XMLLONGLONG_H

#include "longlong.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная long long
 */
class XMLLongLong : public LongLong
{
public:
        /** По умолчанию значение \0 */
        inline XMLLongLong( ) : LongLong( )
        {
        }
        
        inline XMLLongLong( long long value ) : LongLong( value )
        {
        }
        
        inline XMLLongLong( const DLString& value ) 
                : LongLong( value )
        {
        }
        
        /** Возвращает xml представление переменной */
        bool toXML( XMLNode::Pointer& node ) const;
        /** Инициализация класса из xml данных */
        void fromXML( const XMLNode::Pointer& node ) ;
};


/** Вывод в ostream типа XMLLongLong */
inline std::ostream& operator << ( std::ostream& ostr, const XMLLongLong& xmlLongLong )
{
        ostr << xmlLongLong.getValue( );
        return ostr;
}

class XMLLongLongNoEmpty : public XMLLongLong {
public:
        inline XMLLongLongNoEmpty( )
        {
        }
        
        inline XMLLongLongNoEmpty( int value ) : XMLLongLong( value )
        {
        }

        bool toXML( XMLNode::Pointer& node ) const;
};

#endif
