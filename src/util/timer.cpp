/* $Id: timer.cpp,v 1.1.2.4 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "logstream.h"
#include "exception.h"
#include "timer.h"

Timer::Timer(struct timeval value)
{
    this->value = value;
}

Timer::Timer()
{
    static struct timeval zero_time;
    value = zero_time;
}

Timer::~Timer()
{
}

void Timer::sleep() 
{
    if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &value) < 0) {
        if (errno != EINTR) {
            logsystem("select sleep");
            throw Exception();
        }
    }
}

bool Timer::elapsed() const
{
    return value.tv_sec == 0L && value.tv_usec == 0L;
}

void Timer::update()
{
    gettimeofday(&value, NULL);
}

Timer Timer::operator + (const Timer &tb) const
{
    Timer result;
    struct timeval *r = &(result.value);
    const struct timeval *a = &value;
    const struct timeval *b = &(tb.value);

    r->tv_sec = a->tv_sec + b->tv_sec;
    r->tv_usec = a->tv_usec + b->tv_usec;

    while (r->tv_usec >= 1000000) {
        r->tv_usec -= 1000000;
        r->tv_sec++;
    }

    return result;
}

Timer Timer::operator - (const Timer &tb) const
{
    static const struct timeval null = { 0, 0 };
    Timer result;
    struct timeval *r = &(result.value);
    const struct timeval *a = &value;
    const struct timeval *b = &(tb.value);
    
    if (a->tv_sec < b->tv_sec) {
        *r = null;
    }
    else if (a->tv_sec == b->tv_sec) {
        if (a->tv_usec < b->tv_usec) {
            *r = null;
        } else {
            r->tv_sec = 0;
            r->tv_usec = a->tv_usec - b->tv_usec;
        }
    } else {               
        r->tv_sec = a->tv_sec - b->tv_sec;
        
        if (a->tv_usec < b->tv_usec) {
            r->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
            r->tv_sec--;
        } else {
            r->tv_usec = a->tv_usec - b->tv_usec;
        }
    }
    
    return result;    
}

Timer & Timer::operator = (const Timer &a)
{
    value = a.value;
    return *this;
}


bool Timer::operator < (const Timer &tb) const
{
    return toLong() < tb.toLong();
}

long Timer::toLong() const
{
    return value.tv_sec * 1000000 + value.tv_usec;
}

