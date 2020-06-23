/* $Id: profiler.h,v 1.1.4.1.6.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __PROFLER_H__
#define __PROFLER_H__

#include <sys/time.h>
using std::string;

class Profiler {
public:
    void start();
    void stop();

    long msec();

private:
    struct timeval started, stopped;
};

/**
 * Report to the logs how many milliseconds elapsed between creation and destruction. 
 * Takes optional threshold argument, to only report blocks that take more than certain amount of time.
 */
class ProfilerBlock : public Profiler {
public:
    ProfilerBlock(const string &id, long threshold = 0);
    ~ProfilerBlock();
    
private:
    const string id;
    long threshold;
};

#endif
