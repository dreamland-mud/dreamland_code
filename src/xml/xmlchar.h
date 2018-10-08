/* $Id: xmlchar.h,v 1.7.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// xmlchar.h: interface for the XMLClar class.
//
//////////////////////////////////////////////////////////////////////

#ifndef XMLCHAR_H
#define XMLCHAR_H

#include "char.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 * @short XML переменная char
 */
class XMLChar : public Char
{
public:
        /** По умолчанию значение \0 */
        inline XMLChar( ) : Char( )
        {
        }
        
        inline XMLChar( char value ) : Char( value )
        {
        }
        
        inline XMLChar( const DLString& value ) throw( ExceptionBadType )
                : Char( value )
        {
        }
        
        /** Возвращает xml представление переменной */
        bool toXML( XMLNode::Pointer& node ) const;
        /** Инициализация класса из xml данных */
        void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
};





/** Вывод в ostream типа XMLChar */
inline std::ostream& operator << ( std::ostream& ostr, const XMLChar& xmlChar )
{
        ostr << xmlChar.getValue( );
        return ostr;
}

#endif
