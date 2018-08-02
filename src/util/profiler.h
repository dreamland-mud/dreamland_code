/* $Id: profiler.h,v 1.1.4.1.6.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __PROFLER_H__
#define __PROFLER_H__

class Profiler {
public:
    void start();
    void stop();
    long getStart() const;
    long getStop() const;

    long msec();

private:
    long started, stopped;
};

class ProfilerBlock : public Profiler {
public:
    ProfilerBlock(const char *id);
    ~ProfilerBlock();
    
private:
    const char *id;
};

#endif
