/* $Id$
 *
 * ruffina, 2004
 */
#include "worldknowledge.h"

#include "logstream.h"
#include "date.h"
#include "dlfileop.h"
#include "dlscheduler.h"
#include "room.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "fread_utils.h"
#include "mercdb.h"

WorldKnowledge *worldKnowledge = NULL;
const DLString WorldKnowledge::FILE_NAME = "worldknowledge.txt";

WorldKnowledge::WorldKnowledge( )
{
    checkDuplicate( worldKnowledge );
    worldKnowledge = this;
}

WorldKnowledge::~WorldKnowledge( )
{
    worldKnowledge = NULL;
}

void WorldKnowledge::initialization( )
{
    SchedulerTaskRoundPlugin::initialization( );
    load( );
}

void WorldKnowledge::destruction( )
{
    save( );
    SchedulerTaskRoundPlugin::destruction( );
}

void WorldKnowledge::save( )
{
    DLFileWrite file( dreamland->getDbDir( ), FILE_NAME );
    
    if (!file.open( ))
        return;

    for (Visits::iterator v = visits.begin( ); v != visits.end( ); v++) {
#if 0        
        ostringstream os;
        os << v->first;
        file.printf( "%s", os.str().c_str( ));
#else        
        file.printf( "%lld ", v->first );
#endif
        for (VisitedRooms::iterator r = v->second.begin( ); r != v->second.end( ); r++)
            file.printf( "%d ", *r );
        
        file.writeln( );
    }
    
    file.writeln( "0" );
    LogStream::sendNotice( ) << "Saved room visits for " << visits.size( ) << " players" << endl;
}

void WorldKnowledge::load( )
{
    long long num;
    PlayerID playerID = 0;
    DLFileRead file( dreamland->getDbDir( ), FILE_NAME );

    if (!file.open( ))
        return;
    
    while (!file.eof( ) && ( num = fread_number64( file.getFP( ) ) )) {
        if (num > 0xffff) 
            playerID = num;
        else 
            visits[playerID].insert( num );
    }

    LogStream::sendNotice( ) << "Loaded room visits for " << visits.size( ) << " players" << endl;
}

void WorldKnowledge::visit( PCharacter *ch )
{
    visits[ch->getID( )].insert( ch->in_room->vnum );
}

void WorldKnowledge::report( PCharacter *ch, ostringstream &buf )
{
    int cnt = visits[ch->getID( )].size( );
    
    buf << "Visited " << cnt * 100 / top_room << "% rooms "
        << "(" << cnt << " of " << top_room << ")" << endl;
}

void WorldKnowledge::run( )
{
    save( );
    DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_MINUTE * 15, Pointer( this ) );
}

int WorldKnowledge::getPriority( ) const
{
    return SCDP_ROUND + 15;
}


