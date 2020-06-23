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

ProfilerBlock::~ProfilerBlock()
{
    stop();

    long total = msec();
    if (total >= threshold)
        LogStream::sendNotice( ) << "Prof: " << id << " takes " << msec() << " msecs" << endl;
}

