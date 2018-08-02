/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributeticker.h"

#include "date.h"
#include "pcharacter.h"
#include "dreamland.h"

/*
 * XMLAttributeTimer
 */
XMLAttributeTimer::~XMLAttributeTimer( )
{
}

/*
 * XMLAttributeTicker
 */
XMLAttributeTicker::XMLAttributeTicker( int time ) : time( time )
{
}

XMLAttributeTicker::XMLAttributeTicker( ) : time( -1 )
{
}

XMLAttributeTicker::~XMLAttributeTicker( )
{
}

int XMLAttributeTicker::getTime( ) const
{
    return time.getValue( ) + since.getValue( );
}

void XMLAttributeTicker::setTime( int time )
{
    since = dreamland->getCurrentTime( );
    this->time = time;
}

bool XMLAttributeTicker::tick( PCMemoryInterface *pcm )
{
    if (time != -1 && getTime( ) - dreamland->getCurrentTime( ) <= 0) {
	end( pcm );
	return true;
    }

    return false;
}

DLString XMLAttributeTicker::getTimeString( bool russian ) const
{
    DLString str;
    
    str = (russian ? "на" : "for");
    
    if (time.getValue( ) == -1)
	str += (russian ? "всегда" : "ever" );
    else
	str += " " + Date::getStringFromSecond( time.getValue( ) );
    
    return str;
}

DLString XMLAttributeTicker::getUntilString( bool russian ) const
{
    DLString str;
    
    if (time.getValue( ) == -1)
	str += (russian ? "навсегда" : "forever" );
    else {
	str += (russian ? "до" : "until" );
	str += " " + Date::getTimeAsString( getTime( ) );
    }
    
    return str;
}

/*
 * XMLAttributeOnlineTicker
 */ 
bool XMLAttributeOnlineTicker::pull( PCharacter* character )
{
    return tick( character );
}


/*
 * XMLAttributeMemoryTicker
 */
bool XMLAttributeMemoryTicker::pull( PCMemoryInterface *pcm )
{
    return tick( pcm );
}



