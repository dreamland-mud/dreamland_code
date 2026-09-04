/* $Id: profiler.h,v 1.1.4.1.6.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __PROFLER_H__
#define __PROFLER_H__

#include <sys/time.h>
#include <string>
#include <map>

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

/**
 * Always-on cumulative per-block profiling. Every ProfilerBlock records one
 * sample here on destruction, regardless of its log threshold -- so the stats
 * cover ALL calls, not just the slow ones the threshold logs. Read via
 * profiler_stats(); dumped to the notice log by profiler_stats_dump().
 * No lock: the main update loop and the Fenia coroutine scheduler are
 * token-serialized (never run at the same instant), so ProfilerBlock
 * destructors -- even ones reached from Fenia natives -- never race this map.
 */
struct ProfileCounter {
    long count;
    long total;   // sum of elapsed msec across all samples
    long mx;      // largest single sample, msec

    ProfileCounter() : count(0), total(0), mx(0) {}
};

const std::map<string, ProfileCounter> & profiler_stats();
void profiler_stats_reset();
void profiler_stats_dump();

#endif
