/* $Id: timer.h,v 1.1.2.4 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef DL_TIMER_H
#define DL_TIMER_H

#include <unistd.h>
#include <sys/time.h>

class Timer {
public:
    Timer(struct timeval);
    Timer();
    virtual ~Timer();

    void update();
    void sleep(); 
    bool elapsed() const;
    long toLong() const;

    inline long getUSec() const;
    inline long getSec() const;
    inline void setSec(long);
    inline void setUSec(long);
    
    Timer & operator = (const Timer &);
    Timer operator + (const Timer &) const;
    Timer operator - (const Timer &) const;
    bool operator < (const Timer &) const;

protected:
    struct timeval value;
};

inline long Timer::getUSec() const
{
    return value.tv_usec;
}
inline long Timer::getSec() const
{
    return value.tv_sec;
}
inline void Timer::setSec(long sec)
{
    value.tv_sec = sec;
}
inline void Timer::setUSec(long usec)
{
    value.tv_usec = usec;
}
 
#endif
