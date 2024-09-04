/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include <sys/time.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <csignal>

#include "dreamland.h"

#include "lastlogstream.h"
#include "logstream.h"
#include "dl_math.h"

#include "fenia/impl.h"
#include "fenia/register-impl.h"
#include "dlpluginmanager.h"
#include "dlscheduler.h"
#include "objectmanager.h"
#include "npcharactermanager.h"
#include "pcharactermanager.h"
#include "race.h"
#include "skillmanager.h"
#include "clanmanager.h"
#include "profession.h"
#include "hometown.h"
#include "wearlocation.h"
#include "feniamanager.h"
#include "socketmanager.h"
#include "servlet.h"
#include "process.h"
#include "religion.h"
#include "liquid.h"
#include "desire.h"
#include "helpmanager.h"
#include "skillgroup.h"
#include "bonus.h"
#include "eventbus.h"
#include "behavior.h"

#include "merc.h"
#include "def.h"

using namespace std;

DreamLand *dreamland = NULL;
const long DreamLand::DEFAULT_OPTIONS = DL_PK | DL_SAVE_OBJS | DL_SAVE_MOBS; 

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

DreamLand::DreamLand( )
        : currentTime( 0 ), 
          bootTime( 0 ), 
          lastID( 1 ), 
          rebootCounter( -1 ),
          workingOptions( 0, &dreamland_flags ),
          options( DEFAULT_OPTIONS, &dreamland_flags ),
          feniaDbEnv( new DbEnvContext( ) )
{
        checkDuplicate( dreamland );
        dreamland = this;
        
        setSignals( );
        setCurrentTime( );
        setBootTime( );
        
        scheduler.construct( );
        pluginManager.construct( );
        objectManager.construct( );
        npcharacterManager.construct( );
        pcharacterManager.construct( );
        raceManager.construct( );
        skillManager.construct( );
        clanManager.construct( );
        professionManager.construct( );
        feniaManager.construct( );
        processManager.construct( );
        wearlocationManager.construct( );
        hometownManager.construct( );
        liquidManager.construct( );
        religionManager.construct( );
        desireManager.construct( );
        helpManager.construct( );
        skillGroupManager.construct( );
        bonusManager.construct( );
        socketManager.construct( );
        servletManager.construct( );
        eventBus.construct();
        behaviorManager.construct();

        basic_ostringstream<char> buf;
        buf << resetiosflags( ios::left );
}

DreamLand::~DreamLand( )
{
        feniaManager->close( );
        
        processManager.clear( );
        pluginManager.clear( );
        pcharacterManager.clear( );
        npcharacterManager.clear( );
        objectManager.clear( );
        scheduler.clear( );
        raceManager.clear( );
        skillManager.clear( );
        clanManager.clear( );
        professionManager.clear( );
        wearlocationManager.clear( );
        hometownManager.clear( );
        feniaManager.clear( );
        liquidManager.clear( );
        religionManager.clear( );
        desireManager.clear( );
        helpManager.clear( );
        skillGroupManager.clear( );
        bonusManager.clear( );
        servletManager.clear( );
        socketManager.clear( );
        eventBus.clear();
        behaviorManager.clear();

        getFeniaDbEnv( )->close( );
        
        dreamland = 0;
}

void DreamLand::run( )
{
    // init random number generator
    init_mm( );

    LogStream::sendNotice( ) << "Mud status: [" << options.names( ) << "]" << endl;
    LogStream::sendNotice( ) << "Looping..." << endl;

    do {
        pulseStart( );

        LastLogStream::send( ) <<  "Scheduler run"  << endl;
        scheduler->tick( );
        
        // If you are thinking of moving this to a ScheluerTask - think again.
        // ScheldulerPriorityMap will contain tasks defined in plugins.
        // When those plugins are going to be unloaded, Scheduler::slay() requests
        // are going to be ignored for the currently running tick.
        // After the scheduer tick has finished processing, it will attempt to destroy
        // the ScheldulerPriorityMap which holds Pointers to stuff defined in plugins
        // which are gone now. Thus, either Scheduler should support ::slay operation for
        // the currently running tick, or plugin reload should be happening outside of
        // the Scheduler.
        LastLogStream::send( ) <<  "Plugins reload"  << endl;
        pluginManager->checkReloadRequest( );

        LastLogStream::send( ) <<  "Time goes..."  << endl;
        pulseEnd( );

        setCurrentTime( );
    }
    while (!isShutdown( ));

    LogStream::sendNotice( ) << "Normal termination of the game." << endl;
}

void DreamLand::pulseStart( )
{
    gettimeofday(&pulseStartTime, NULL);
}

static int 
sleepTime(struct timeval *pulseEnd)
{
    struct timeval now, tmp;

    gettimeofday(&now, NULL);
    timersub(pulseEnd, &now, &tmp);
    return tmp.tv_sec*1000 + tmp.tv_usec/1000;
}

void DreamLand::pulseEnd( )
{
    struct timeval pulseEnd, pulseWidth = { 0, 1000000/getPulsePerSecond( ) };
    int ms;
    
    timeradd(&pulseStartTime, &pulseWidth, &pulseEnd);

    Scripting::Object::manager->sync(&pulseEnd);

    ms = sleepTime(&pulseEnd);

    if(ms < 0) {
        LogStream::sendError() << "pulse overflow " << -ms << "msec" << endl;
        return;
    }

    do {
        socketManager->run(ms);
        ms = sleepTime(&pulseEnd);
    } while(ms > 0);
}


void DreamLand::load( bool recursive )
{
    if (!loadConfig( ))
        throw Exception( "Cannot load DreamLand configuration" );

    if (recursive) {
        bootVersion++;
        workingOptions.setValue( options );
        save( false );

        if(!logPattern.empty( )) {
            DLFile logFile( getBasePath( ), logPattern );
            LogStream::redirect(new FileLogStream(logFile.getPath( )));
        }

        getFeniaDbEnv( )->open( DLFile( getBasePath( ), feniaDbDir ).getPath( ) );
        
        feniaManager->open( );
        feniaManager->load( );

        if (apiPort != 0)
            servletManager->open(apiPort);
        
        pluginManager->loadAll( );
    }
}

void DreamLand::save( bool recursive )
{
    if (!saveConfig( ))
        throw Exception( "Cannot save DreamLand configuration" );

    if (recursive) {
        feniaManager->sync( 0 );
    }
}


void DreamLand::shutdown( )
{
    setOption( DL_SHUTDOWN );
}

bool DreamLand::isShutdown( ) const
{
    return hasOption( DL_SHUTDOWN );
}

void DreamLand::setCurrentTime( )
{
    currentTime = time(NULL); 
}

void DreamLand::setBootTime( )
{
    bootTime = currentTime;
}


void DreamLand::setSignals( )
{
    signal( SIGTERM, &signalHandler );
    signal( SIGSTOP, &signalHandler );
    signal( SIGINT, &signalHandler );
    signal( SIGXCPU, &signalHandler );
    signal( SIGXFSZ, &signalHandler );

    signal( SIGPIPE, SIG_IGN );
}

void DreamLand::signalHandler( int signo )
{
    LogStream::sendWarning( ) << "Caught signal "<< signo << "." << endl;
    dreamland->shutdown( );
}


/*
 * Enforce generation of references to weakly-defined symbols in src, so that if the same symbol is defined by two
 * different plugins, the reference would always resolve in favour of libdreamland.so. This prevents a situation
 * when both plugins have a reference to, say, std::_Sp_make_shared_tag::_S_ti()::__tag, and one of them always 
 * fails to unload because some other plugin has a reference to its symbol.

 * Encountered this situation with tao/pegtl library and libolc.so.
 */
#include <memory>
void dummy() 
{
    struct dummy {} d;
    std::make_shared<dummy>(d);
}

