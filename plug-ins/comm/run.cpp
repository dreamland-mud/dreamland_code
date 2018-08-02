/* $Id: run.cpp,v 1.8.2.11.6.6 2009/11/08 17:35:53 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "run.h"
#include "exitsmovement.h"
#include "directions.h"
#include "movetypes.h"

#include "char.h"
#include "commandtemplate.h"
#include "pcharacter.h"
#include "room.h"
#include "dlscheduler.h"

#include "merc.h"
#include "def.h"


static bool isBigLetter( char c )
{
    return c == 'N' || c == 'S' || c == 'D' || c == 'U' || c == 'E' || c == 'W' ||
	   c == 'С' || c == 'Ю' || c == 'О' || c == 'П' || c == 'В' || c == 'З';
}

static bool isSmallLetter( char c )
{
    return c == 'n' || c == 's' || c == 'd' || c == 'u' || c == 'e' || c == 'w' ||
           c == 'с' || c == 'ю' || c == 'о' || c == 'п' || c == 'в' || c == 'з';
}

/*-----------------------------------------------------------------------------
 * 'run' command 
 *----------------------------------------------------------------------------*/
CMDRUN( run )
{
    PCharacter *pch;
    DLString walk;
    ostringstream buf;
    unsigned int i;

    pch = ch->getPC( );
    walk = constArguments;
    walk.stripWhiteSpace( );

    if (!pch || walk.empty( )) {
	ch->send_to( "Куда собрались?\r\n" );
	return;
    }

    if (pch->fighting) {
	pch->send_to("Закончи сражение, потом беги.\n\r");
	return;
    }

    if (pch->position < POS_STANDING) {
	pch->send_to("Исходное положение для бега - стоя!\n\r");
	return;
    }
    
    for (i = 0; i < walk.length( ); i++) {
	int cnt = 0;
	
	while (isdigit( walk[i] )) {
	    cnt = cnt * 10 + walk[i++] - '0';

	    if (i >= walk.length( )) {
		pch->send_to( "Маршрут не может оканчиваться на цифру.\r\n" );
		return;
	    }
	}
	
	if (isBigLetter( walk[i] )) {
	    if (cnt > 0) {
		pch->send_to( "Нельзя побежать 'несколько раз до упора'.\r\n" );
		return;
	    }
	}
	else if (!isSmallLetter( walk[i] )) {
	    pch->printf( "Непонятное направление для бега: '%c'.\r\n", walk[i] );
	    return;
	}
	
	cnt = max( cnt, 1 );

	while (cnt-- > 0)
	    buf << walk[i];
    }
    
    if (buf.str( ).length( ) > 100) {
	pch->send_to( "Слишком далеко бежать.\r\n" );
	return;
    }
    
    pch->getAttributes( ).getAttr<XMLAttributeSpeedWalk>( "speedwalk" )->setValue( buf.str( ) );
}

/*-----------------------------------------------------------------------------
 * RunMovement 
 *----------------------------------------------------------------------------*/
class RunMovement : public ExitsMovement {
public:
    RunMovement( PCharacter *ch, XMLAttributeSpeedWalk::Pointer walk )
                     : ExitsMovement( ch, MOVETYPE_RUNNING )
    {
	this->walk = walk;
    }

    virtual ~RunMovement( )
    {
    }
    
    virtual int move( )
    {
	if (walk->getSteps( ) > 100) {
	    msgSelfRoom( ch, 
	                 "Ты начинаешь задыхаться и останавливаешься.",
	                 "%1$^C1 начинает задыхаться и останавливается." );
	    return RC_MOVE_FAIL;
	}

	init( );

	if (isSmallLetter( walk->getFirstCommand( ) )) {
	    if (moveRecursive( )) {
		walk->clearFirstCommand( );
		return RC_MOVE_OK;
	    }
	    else
		return RC_MOVE_FAIL;
	}

	if (!checkContinuousWay( )) {
	    ostringstream buf;
	    
	    walk->clearFirstCommand( );
	    walk->clearSteps( );
	    
	    buf << "Ты натыкаешься на препятствие и ";
	    
	    if (walk->isEmpty( ))
		buf << "останавливаешься";
	    else {
		int d0 = walk->getFirstDoor( );
		
		buf << "продолжаешь свой путь ";

		if (d0 >= 0 && d0 < DIR_SOMEWHERE) 
		    buf << dirs[d0].leave;
		else 
		    buf << "неизвестно куда..";
	    }

	    buf << "." << endl;
	    msgSelf( ch, buf.str( ).c_str( ) );
	    return RC_MOVE_OK;
	}

	if (moveRecursive( )) {
	    walk->incSteps( );
	    return RC_MOVE_OK;
	}

	return RC_MOVE_FAIL;
    }

protected:
    void init( )
    {
	door = walk->getFirstDoor( );

	if (door < 0) 
	    return;

	if (!( pexit = from_room->exit[door] ))
	    return;

	if (!ch->can_see( pexit ))
	    return;
	
	to_room = pexit->u1.to_room;
	exit_info = pexit->exit_info;
    }

    bool checkContinuousWay( )
    {
	bool fOpenWay;
	
	if (!pexit || !to_room)
	    return false;
	    
	silence = true;
	fOpenWay = checkVisibility( ch ) && checkClosedDoor( ch );
	silence = false;
	return fOpenWay;
    }
    
    XMLAttributeSpeedWalk::Pointer walk;
};

/*-----------------------------------------------------------------------------
 * Speedwalk Update Task
 *----------------------------------------------------------------------------*/
void SpeedWalkUpdateTask::run( PCharacter *ch )
{
    XMLAttributeSpeedWalk::Pointer walk;
    XMLAttributes &attributes = ch->getAttributes( );
    
    walk = attributes.findAttr<XMLAttributeSpeedWalk>( "speedwalk" );
    
    if (!walk)
	return;

    if (walk->isEmpty( )) {
	attributes.eraseAttribute( "speedwalk" );
	return;
    }

    if (ch->fighting || ch->position < POS_STANDING) {
	attributes.eraseAttribute( "speedwalk" );
	return;
    }
    
    if (ch->wait > 0)
	return;

    if (RunMovement( ch, walk ).move( ) != RC_MOVE_OK || walk->isEmpty( )) 
	attributes.eraseAttribute( "speedwalk" );
}

void SpeedWalkUpdateTask::after( )
{
    DLScheduler::getThis( )->putTaskInitiate( Pointer( this ) );
}

/*-----------------------------------------------------------------------------
 * XMLAttributeSpeedWalk 
 *----------------------------------------------------------------------------*/
char XMLAttributeSpeedWalk::getFirstCommand( ) const
{
    if (path.getValue( ).empty( ))
	return '\0';
    else
	return path.getValue( ).at( 0 );
}

void XMLAttributeSpeedWalk::clearFirstCommand( )
{
    if (!isEmpty( )) {
	DLString str = path.getValue( );

	str.erase( 0, 1 );
	path.setValue( str );
    }

    if (isEmpty( ))
	steps = 0;
}

int XMLAttributeSpeedWalk::getFirstDoor( ) const
{
    char c = getFirstCommand( );
    return direction_lookup( Char::lower(c) );
}

