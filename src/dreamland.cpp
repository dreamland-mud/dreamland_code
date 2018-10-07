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
#include "racelanguage.h"
#include "skillmanager.h"
#include "clanmanager.h"
#include "profession.h"
#include "hometown.h"
#include "wearlocation.h"
#include "feniamanager.h"
#include "process.h"
#include "religion.h"
#include "liquid.h"
#include "desire.h"
#include "helpmanager.h"
#include "skillgroup.h"

#include "mercdb.h"
#include "merc.h"
#include "def.h"

using namespace std;

DreamLand *dreamland = NULL;
const long DreamLand::DEFAULT_OPTIONS = DL_PK | DL_SAVE_OBJS | DL_SAVE_MOBS; 

DreamLand::DreamLand( )
	: currentTime( 0 ), 
	  bootTime( 0 ), 
	  lastID( 1 ), 
	  rebootCounter( -1 ),
	  workingOptions( 0, &dreamland_flags ),
	  options( DEFAULT_OPTIONS, &dreamland_flags ),
	  dbEnv( new DbEnvContext( ) )
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
	raceLanguageManager.construct( );
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
	raceLanguageManager.clear( );
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

	getDbEnv( )->close( );
	
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
	
	LastLogStream::send( ) <<  "Processes pulse"  << endl;
	processManager->yield( );

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
    pulseStartTime = clock( );
}

void DreamLand::pulseEnd( )
{
    // Synchronize to a clock.
    // Sleep( last_time + 1/PULSE_PER_SCD - now ).
    // Careful here of signed versus unsigned arithmetic.
    clock_t nowTime, pulseEnd, sleepTime;
    
    pulseEnd = pulseStartTime + CLOCKS_PER_SEC / getPulsePerSecond( );

    Scripting::Object::manager->sync( pulseEnd );
    
    nowTime = clock( );

    if(nowTime > pulseEnd) {
	sleepTime = nowTime - pulseEnd;
	LogStream::sendError() 
	    << "pulse overflow " 
	    << (1000 * sleepTime / CLOCKS_PER_SEC) << "msec" << endl;
	return;
    }
    sleepTime = pulseEnd - nowTime;
    
#ifndef __MINGW32__
    usleep(1000000*sleepTime/CLOCKS_PER_SEC);
#else
    Sleep(1000*sleepTime/CLOCKS_PER_SEC);
#endif
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

	getDbEnv( )->open( DLFile( getBasePath( ), feniaDbDir ).getPath( ) );
	
	feniaManager->open( );
	feniaManager->load( );

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
#ifndef __MINGW32__
    signal( SIGTERM, &signalHandler );
    signal( SIGSTOP, &signalHandler );
    signal( SIGINT, &signalHandler );
    signal( SIGXCPU, &signalHandler );
    signal( SIGXFSZ, &signalHandler );

    signal( SIGPIPE, SIG_IGN );
#endif
}

void DreamLand::signalHandler( int signo )
{
    LogStream::sendWarning( ) << "Caught signal "<< signo << "." << endl;
    dreamland->shutdown( );
}

