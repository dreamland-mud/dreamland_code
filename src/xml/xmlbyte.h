/* $Id: xmlbyte.h,v 1.7.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// xmlbyte.h: interface for the XMLByte class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLBYTE_H
#define XMLBYTE_H

#include "byte.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная unsigned char
 */
class XMLByte : public Byte
{
public:
        /** По умолчанию значение 0 */
        inline XMLByte( ) : Byte( )
        {
        }
        
        inline XMLByte( unsigned char value ) : Byte( value )
        {
        }
        
        inline XMLByte( const DLString& value ) 
                : Byte( value )
        {
        }

        /** Возвращает xml представление переменной */
        bool toXML( XMLNode::Pointer& node ) const;
        /** Инициализация класса из xml данных */
        void fromXML( const XMLNode::Pointer& node ) ;
};




/** Вывод в ostream типа XMLByte */
inline std::ostream& operator << ( std::ostream& ostr, const XMLByte& xmlByte )
{
        ostr << xmlByte.getValue( );
        return ostr;
}

#endif
