/* $Id: boolean.h,v 1.2.2.1.30.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/***************************************************************************
                          boolean.h  -  description
                             -------------------
    begin                : Fri Oct 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef BOOLEAN_H
#define BOOLEAN_H

#include "dlstring.h"
/**
 * @author Igor S. Petrenko
 */
class Boolean
{
public:
        static const DLString TYPE;
        static const DLString VAL_TRUE;
        static const DLString VAL_FALSE;

public:        
        /** По умолчанию значение true */
        inline Boolean( ) : value( true )
        {
        }

        inline Boolean( bool value ) : value( value )
        {
        }

        inline Boolean( const DLString& value ) throw( ExceptionBadType )
        {
                fromString( value );
        }

        DLString toString( ) const throw( );
        void fromString( const DLString& value ) throw( ExceptionBadType );
        
        inline bool getValue( ) const
        {
                return value;
        }

        inline void setValue( bool value )
        {
                this->value = value;
        }

        inline operator bool & ( ) {
            return value;
        }

        inline operator const bool & ( ) const {
            return value;
        }

private:
        bool value;
};



/** Вывод в ostream типа Boolean */
inline std::ostream& operator << ( std::ostream& ostr, const Boolean& boolean )
{
        ostr << boolean.getValue( );
        return ostr;
}

#endif
