/* $Id: xmldate.h,v 1.5.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          xmldate.h  -  description
                             -------------------
    begin                : Wed May 30 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef XMLDATA_H
#define XMLDATA_H

#include "date.h"
#include "xmlnode.h"

/**
 * @author Igor S. Petrenko
 */
class XMLDate : public Date
{
public:
        /** По умолчанию значение 0 */
        inline XMLDate( ) 
        {
        }
        
        inline XMLDate( long time ) : Date( time )
        {
        }
        
        inline XMLDate& operator = ( const Date& newDate )
        {
                time = newDate.getTime( );
                return *this;
        }

        /** Возвращает xml представление переменной */
        bool toXML( XMLNode::Pointer& node ) const;
        /** Инициализация класса из xml данных */
        void fromXML( const XMLNode::Pointer& node ) ;
};


class XMLDateNoEmpty : public XMLDate
{
public:
        inline XMLDateNoEmpty( ) 
        {
        }
        
        inline XMLDateNoEmpty( long time ) : XMLDate( time )
        {
        }
        
        bool toXML( XMLNode::Pointer& node ) const;
};


/** Вывод в ostream типа XMLDate */
inline std::ostream& operator << ( std::ostream& ostr, const XMLDate& xmlDate )
{
        ostr << xmlDate.getTimeAsString( );
        return ostr;
}

#endif
