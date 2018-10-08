/***************************************************************************
                          mocfunctional.h  -  description
                             -------------------
    begin                : Fri Apr 13 2001
    copyright            : (C) 2001 by nofate
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef MOCFUNCTIONAL_H
#define MOCFUNCTIONAL_H

template<class T>
inline T toList( string str, char c = ' ' )
{
        T list;

        if( str.empty( ) )
        {
                list.push_back( "./" );
                return list;
        }

        if( str[str.length( ) - 1] != c )
        {
                str += c;
        }
        string::size_type opos = str.find_first_not_of( c );
        for( string::size_type pos = str.find( c, opos );
                pos != string::npos && opos != string::npos ;
                pos = str.find( c, opos ) )
        {
                list.push_back( str.substr( opos, pos - opos ) );
                opos = str.find_first_not_of( c, pos + 1 );
        }
        return list;
}


#endif
