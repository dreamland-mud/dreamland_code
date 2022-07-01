/* $Id: run.cpp,v 1.8.2.11.6.6 2009/11/08 17:35:53 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "run.h"
#include "exitsmovement.h"
#include "directions.h"
#include "movetypes.h"
#include "door_utils.h"

#include "char.h"
#include "commandtemplate.h"
#include "pcharacter.h"
#include "room.h"
#include "dlscheduler.h"

#include "merc.h"
#include "def.h"


static bool isBigLetter( char c )
{
    return door_is_big(c) || door_is_big_ru(c);
}

static bool isSmallLetter( char c )
{
    return door_is_small(c) || door_is_small_ru(c);
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

    if (ch->is_npc()) {
        ch->pecho("Тебе доступно только передвижение пешком, увы.");
        return;
    }

    pch = ch->getPC();
    walk = constArguments;
    walk.stripWhiteSpace( );

    if (pch->fighting) {
        pch->pecho("Но ты же сражаешься! Используй команду {y{lRсбежать{lEflee{x для побега из боя.");
        return;
    }

    if (!pch || walk.empty( )) {
        ch->pecho("По какому маршруту ты хочешь бежать?");
        return;
    }

    if (pch->position < POS_STANDING) {
        pch->pecho("Исходное положение для бега -- стоя!");
        return;
    }
    
    for (i = 0; i < walk.length( ); i++) {
        int cnt = 0;
        
        while (isdigit( walk[i] )) {
            cnt = cnt * 10 + walk[i++] - '0';

            if (i >= walk.length( )) {
                pch->pecho("Маршрут не может оканчиваться на цифру.");
                return;
            }
        }
        
        if (isBigLetter( walk[i] )) {
            if (cnt > 0) {
                pch->pecho("До упора в одну сторону можно побежать только один раз.");
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
        pch->pecho("Слишком далеко бежать.");
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
                    buf << "неизвестно куда...";
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
        if (!pexit || !to_room)
            return false;
            
        if (!checkVisibility( ch ))
            return false;
            
        rc = getDoorStatus( ch );
        if (rc == RC_MOVE_OK || rc == RC_MOVE_CLOSED || rc == RC_MOVE_PASS_ALWAYS || rc == RC_MOVE_PASS_POSSIBLE)
            return true;

        return false;
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
        walk->show(ch);
        attributes.eraseAttribute( "speedwalk" );        
        return;
    }
    
    if (ch->wait > 0)
        return;

    if (RunMovement( ch, walk ).move( ) != RC_MOVE_OK || walk->isEmpty( )) {
        walk->show(ch);
        attributes.eraseAttribute( "speedwalk" );        
    }
}

void SpeedWalkUpdateTask::after( )
{
    DLScheduler::getThis( )->putTaskInitiate( Pointer( this ) );
}

/*-----------------------------------------------------------------------------
 * XMLAttributeSpeedWalk 
 *----------------------------------------------------------------------------*/

/** Display remaining path to the player, if any. */
void XMLAttributeSpeedWalk::show(PCharacter *ch) const
{
    if (path.empty())
        return;

    // Speedwalk is kept in enhanced mode: replace nnn with 3n.
    DLString collated;
    char last_letter = path.at(0);
    int cnt = 0;

    // TODO: use collate_speedwalk() from the 'traverse' plugin.
    for (size_t i = 0; i < path.size(); i++) {
        if (path.at(i) == last_letter && isSmallLetter(last_letter)) {
            cnt++;
        } else {
            if (cnt > 1) // Don't show '1n', just 'n'.
                collated << cnt;
            collated << last_letter;
            last_letter = path.at(i);
            cnt = 1;
        }
    }

    // Add last remaining letter from the path.
    if (isSmallLetter(last_letter)) {
        if (cnt > 1)
            collated << cnt;
        collated << last_letter;
    }
        
    if (ch->isCoder())
        ch->printf("Развернутый маршрут: %s\r\n", path.c_str());

    if (!collated.empty())
        ch->printf("Тебе оставалось бежать: {hs{c%s{x\r\n", collated.c_str());
}

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

