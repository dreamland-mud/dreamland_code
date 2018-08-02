/* $Id: profiler.cpp,v 1.1.4.1.6.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include <sys/time.h>

#include "profiler.h"
#include "logstream.h"

void
Profiler::start()
{
    started = clock( );
}

void
Profiler::stop()
{
    stopped = clock( );
}

long Profiler::getStart() const
{
    return started;
}

long Profiler::getStop() const
{
    return stopped;
}

long
Profiler::msec()
{
    return 1000*(stopped - started)/CLOCKS_PER_SEC;
}


ProfilerBlock::ProfilerBlock(const char *i) : id(i)
{
    start();
}

ProfilerBlock::~ProfilerBlock()
{
    stop();
    LogStream::sendNotice( ) << "Prof: " << id << " takes " << msec() << " msecs" << endl;
}

