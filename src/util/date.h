/* $Id: date.h,v 1.8.2.1.30.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          date.h  -  Время
                             -------------------
    begin                : Tue May 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef DATE_H
#define DATE_H

#include <ctime>
#include "dlstring.h"
#include "exceptionbaddatestring.h"


/**
 * @author Igor S. Petrenko
 * @short Класс для работы с временем и датой
 */
class Date 
{
public:
        static const int SECOND_IN_MINUTE = 60;
        static const int SECOND_IN_HOUR = 60 * SECOND_IN_MINUTE;
        static const int SECOND_IN_DAY = 24 * SECOND_IN_HOUR;
        static const int SECOND_IN_WEEK = 7 * SECOND_IN_DAY;
        static const int SECOND_IN_MONTH = 30 * SECOND_IN_DAY;
        static const int SECOND_IN_YEAR = 12 * SECOND_IN_MONTH;
        
public:
        /** По умолчанию значение времени 0 */
        inline Date( ) : time( 0 )
        {
        }
        
        /** Значение времени задается как стандартное UNIX время */
        inline Date( time_t time ) : time( time )
        {
        }
        
        inline Date( const Date& newDate ) : time( newDate.time )
        {
        }

        inline Date& operator = ( const Date& newDate )
        {
                time = newDate.time;
                return *this;
        }
        
        /** @return текущее время системы в секундах */
        static time_t getCurrentTime( );
        /** @return Текущее время системы */
        static DLString getCurrentTimeAsString( );
        
        static DLString getCurrentTimeAsString( const char* param );
        
        /**
         * <pre>
         * переводит во время строку вида:
         * [+/-] [<number>mon] [<number>d] ....
         * доступные в порядке убывания:
         *   mon, w, d, h, min, s
         *
         * Пример: - 1d 5min 5seconds
         * В итоге: -86705
         * </pre>
         */
        static int getSecondFromString( const DLString& date ) throw( ExceptionBadDateString );
        
        static DLString getStringFromSecond( int time );
        
        /** @return класс с текущим временем */
        static Date newInstance( );
        
        /** @return текущее время в секундах */
        inline time_t getTime( ) const
        {
                return time;
        }
        /** @return текущее время */
        DLString getTimeAsString( ) const;
        DLString getTimeAsString( const char * ) const;
        /** @return указанное время */
        static DLString getTimeAsString( time_t time );
        static DLString getTimeAsString( time_t time, const char * );
        /** @param установить время */
        inline void setTime( time_t time )
        {
                this->time = time;
        }
        /** @param установить время */
        inline void setTime( const Date& date )
        {
                this->time = date.getTime( );
        }
        
protected:
        /** время в секундах */
        time_t time;
};



#endif
