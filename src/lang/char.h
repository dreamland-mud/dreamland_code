/* $Id: char.h,v 1.2.2.2.30.6 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          char.h  -  description
                             -------------------
    begin                : Fri Oct 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef CHAR_H
#define CHAR_H

#include "dlstring.h"

/**
 * @author Igor S. Petrenko
 */
class Char
{
public:
        static const DLString TYPE;

public:        
        /** По умолчанию значение \0 */
        inline Char( ) : value( '\0' )
        {
        }

        inline Char( char value ) : value( value )
        {
        }

        inline Char( const DLString& value ) 
        {
                fromString( value );
        }
        
        DLString toString( ) const ;
        void fromString( const DLString& value ) ;
        
        inline char getValue( ) const
        {
                return value;
        }

        inline void setValue( char value )
        {
                this->value = value;
        }

        bool isDelimiter( ) const;
        bool isUpper( ) const;
        bool isLower( ) const;

        inline operator char & ( ) {
            return value;
        }

        /** Переводит символ в нижний регистр */
        static char lower( char c );
        
        /** Переводит символ в верхний регистр */
        static char upper( char c );
        
private:
        char value;
};



/** Вывод в ostream типа Char */
inline std::ostream& operator << ( std::ostream& ostr, const Char& charValue )
{
        ostr << charValue.getValue( );
        return ostr;
}



#endif
