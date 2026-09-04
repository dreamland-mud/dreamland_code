/* $Id: profiler.cpp,v 1.1.4.1.6.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include <sys/time.h>
#include <string>
#include "profiler.h"
#include "logstream.h"

void
Profiler::start()
{
    gettimeofday(&started, NULL);
}

void
Profiler::stop()
{
    gettimeofday(&stopped, NULL);
}

long
Profiler::msec()
{
    struct timeval result;
    timersub(&stopped, &started, &result);

    return result.tv_sec*1000 + result.tv_usec/1000;
}


ProfilerBlock::ProfilerBlock(const string &i, long t) : id(i), threshold(t)
{
    start();
}

// Always-on cumulative stats, keyed by block id. Meyers singleton keeps the
// map clear of static-init ordering; access needs no lock because the main loop
// and the Fenia coroutine scheduler are token-serialized (see the note in profiler.h).
static std::map<string, ProfileCounter> & prof_map( )
{
    static std::map<string, ProfileCounter> m;
    return m;
}

const std::map<string, ProfileCounter> & profiler_stats( )
{
    return prof_map( );
}

void profiler_stats_reset( )
{
    prof_map( ).clear( );
}

void profiler_stats_dump( )
{
    std::map<string, ProfileCounter>::const_iterator i;
    for (i = prof_map( ).begin( ); i != prof_map( ).end( ); i++) {
        const ProfileCounter &c = i->second;
        long avg = c.count > 0 ? c.total / c.count : 0;
        LogStream::sendNotice( ) << "PerfStat: " << i->first
            << " count=" << c.count
            << " total=" << c.total
            << " avg=" << avg
            << " max=" << c.mx << " msecs" << endl;
    }
}

ProfilerBlock::~ProfilerBlock()
{
    stop();

    long total = msec();

    ProfileCounter &c = prof_map( )[id];
    c.count++;
    c.total += total;
    if (total > c.mx)
        c.mx = total;

    if (total >= threshold)
        LogStream::sendNotice( ) << "Prof: " << id << " takes " << total << " msecs" << endl;
}

