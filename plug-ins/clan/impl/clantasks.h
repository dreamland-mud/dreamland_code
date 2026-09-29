#ifndef CLANTASKS_H
#define CLANTASKS_H

#include "schedulertaskroundplugin.h"
#include "descriptorstatelistener.h"

/** Banks clan tenure of online players and raises tenure ranks (5-8). */
class ClanTenureTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<ClanTenureTask> Pointer;

    virtual void run( );
    virtual int getPriority( ) const;
    virtual void after( );
};

/** At boot and daily: players offline for 180 days lose tenure ranks and offices. */
class ClanDecayTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<ClanDecayTask> Pointer;

    virtual void run( );
    virtual int getPriority( ) const;
    virtual void after( );
};

/** On entering the game: late decay, then bank tenure (creates the clan record). */
class ClanLoginListener : public DescriptorStateListener {
public:
    typedef ::Pointer<ClanLoginListener> Pointer;

    virtual void run( int, int, Descriptor * );
};

#endif
