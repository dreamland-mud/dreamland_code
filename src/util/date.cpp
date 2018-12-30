/* $Id: date.cpp,v 1.8.2.1.30.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          date.cpp  -
                             -------------------
    begin                : Tue May 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <ctime>

#undef yyFlexLexer
#define yyFlexLexer dateFlexLexer
#include <FlexLexer.h>

#include "dateparser.h"
#include "date.h"

time_t Date::getCurrentTime( )
{
    return ::time( 0 );
}

DLString Date::getCurrentTimeAsString( )
{
    return getTimeAsString( getCurrentTime( ) );
}

DLString Date::getCurrentTimeAsString( const char* param )
{
    return getTimeAsString( getCurrentTime( ), param );
}


DLString Date::getTimeAsString( ) const
{
    return getTimeAsString( time );
}

DLString Date::getTimeAsString( const char* param ) const
{
    return getTimeAsString( time, param );
}

DLString Date::getTimeAsString( time_t time )
{
    DLString timeBS = ctime( &time );
    return timeBS.substr( 0, timeBS.length( ) - 1 );
}

DLString Date::getTimeAsString( time_t time, const char *param )
{
    char buf[1000];
    strftime( buf, sizeof( buf ), param, localtime( &time ) );
    return DLString( buf );
}

Date Date::newInstance( )
{
    return Date( ::time( 0 ) );
}

int Date::getSecondFromString( const DLString& date ) throw( ExceptionBadDateString )
{
        istringstream istr(date.c_str());
        DateParser parser( date, &istr );
        return        parser.getSecond( );
}

DLString Date::getStringFromSecond( int time )
{
        int sign = 1;
        if( time < 0 )
        {
                sign = -1;
                time = -time;
        }
        int month = time / SECOND_IN_MONTH; time -= month * SECOND_IN_MONTH;
        int day = time / SECOND_IN_DAY; time -= day * SECOND_IN_DAY;
        int hour = time / SECOND_IN_HOUR; time -= hour * SECOND_IN_HOUR;
        int minute = time / SECOND_IN_MINUTE; time -= minute * SECOND_IN_MINUTE;
        
        bool available = false;
        std::basic_ostringstream<char> buf;
        if( sign == -1 )
        {
                if( available ) buf << ' ';
                else available = true;
                buf << '-';
        }
        if( month > 0 )
        {
                if( available ) buf << ' ';
                else available = true;
                buf << month << "month(s)";
        }
        if( day > 0 )
        {
                if( available ) buf << ' ';
                else available = true;
                buf << day << "day(s)";
        }
        if( hour > 0 )
        {
                if( available ) buf << ' ';
                else available = true;
                buf << hour << "hour(s)";
        }
        if( minute > 0 )
        {
                if( available ) buf << ' ';
                else available = true;
                buf << minute << "minute(s)";
        }
        if( time > 0 )
        {
                if( available ) buf << ' ';
                else available = true;
                buf << time << "second(s)";
        }
        return buf.str( );
}
