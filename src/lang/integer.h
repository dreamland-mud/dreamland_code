/* $Id: integer.h,v 1.8.2.1.30.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          integer.h  -  description
                             -------------------
    begin                : Mon Oct 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef INTEGER_H
#define INTEGER_H

#include "dlobject.h"
#include "dlstring.h"

/**
 * @author Igor S. Petrenko
 */
class Integer 
{
public:
        static const DLString TYPE;

public:        
        /** По умолчанию значение 0 */
        inline Integer( ) : value( 0 )
        {
        }

        inline Integer( int value ) : value( value )
        {
        }

        inline Integer( const DLString& value ) throw( ExceptionBadType )
        {
                fromString( value );
        }

        DLString toString( ) const throw( );
        void fromString( const DLString& value ) throw( ExceptionBadType );

        inline int getValue( ) const
        {
                return value;
        }

        inline void setValue( int value )
        {
                this->value = value;
        }

        inline operator int & ( ) {
            return value;
        }

        inline operator const int & ( ) const {
            return value;
        }

private:
        int value;
};




/** Вывод в ostream типа Integer */
inline std::ostream& operator << ( std::ostream& ostr, const Integer& integer )
{
        ostr << integer.getValue( );
        return ostr;
}

#endif
