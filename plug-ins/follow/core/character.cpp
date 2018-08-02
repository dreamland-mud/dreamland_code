/* $Id$
 *
 * ruffina, 2004
 */
#include "character.h"
#include "follow_utils.h"

/*-------------------------------------------------------------------------
 * followers
 *------------------------------------------------------------------------*/
void Character::die_follower( )
{
    follower_die( this );
}

void Character::add_follower( Character *mch )
{
    follower_add( this, mch );
}

void Character::stop_follower( )
{
    follower_stop( this );
}

