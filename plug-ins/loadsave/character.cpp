/* $Id$
 *
 * ruffina, 2004
 */
#include "character.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "dreamland.h"
#include "def.h"

CLAN(none);
WEARLOC(stuck_in);
WEARLOC(hold_leg);

/****************************************************************************
 * visibility of things 
 ****************************************************************************/

/* visibility on a room -- for entering and exits */
bool Character::can_see( Room *pRoomIndex ) const
{
	if (!pRoomIndex)
		return false;

        if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY)
                && get_trust() < MAX_LEVEL )
                return false;

        if ( IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
                && !is_immortal() )
                return false;

        if ( IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
                && !is_immortal() )
                return false;

        if ( IS_SET(pRoomIndex->area->area_flag, AREA_WIZLOCK) 
                && !is_immortal() )
                return false;

        if ( IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
                && getRealLevel( ) > PK_MIN_LEVEL && !is_immortal())
                return false;
        
        return true;
}

/*
 * True if char can see exit.
 */
bool Character::can_see( EXIT_DATA *pexit ) const
{
    if (!pexit->u1.to_room)
        return false;

    if (!can_see( pexit->u1.to_room ))
        return false;

    if ( !is_npc() && ( IS_GHOST( this ) || IS_DEATH_TIME( this ) ) )
        return true;

    if ( (!is_npc() && IS_SET(act, PLR_HOLYLIGHT)) || (is_npc() && is_immortal()))
        return true;

    if (IS_SET( pexit->exit_info, EX_INVISIBLE ) && !CAN_DETECT(this, DETECT_INVIS))
        return false;

    if (IS_SET( pexit->exit_info, EX_IMPROVED ) && !CAN_DETECT(this, DETECT_IMP_INVIS))
        return false;

    if (IS_SET( pexit->exit_info, EX_CAMOUFLAGE ) && !CAN_DETECT(this,ACUTE_VISION))
        return false;

    if (IS_SET( pexit->exit_info, EX_HIDDEN ) && !CAN_DETECT(this, DETECT_HIDDEN))
        return false;

    if (IS_SET( pexit->exit_info, EX_FADE ) && !CAN_DETECT(this, DETECT_FADE))
        return false;

    return true;
}


/*
 * True if char can see extra exit.
 */
bool Character::can_see( EXTRA_EXIT_DATA *peexit ) const
{
        if (!peexit->u1.to_room)
            return false;

        if (!can_see( peexit->u1.to_room ))
            return false;

        if ( !is_npc()
                && ( IS_GHOST( this )
                        || IS_DEATH_TIME( this ) ) )
                return true;

        if ( (!is_npc() && IS_SET(act, PLR_HOLYLIGHT))
                || (is_npc() && is_immortal()))
                return true;

        if ( IS_AFFECTED(this, AFF_BLIND) )
                return false;

        if ( in_room == NULL )
                return false;

        if ( IS_SET( peexit->exit_info, EX_INVISIBLE )
                && !CAN_DETECT(this, DETECT_INVIS) )
                return false;

        if ( IS_SET( peexit->exit_info, EX_IMPROVED )
                && !CAN_DETECT(this, DETECT_IMP_INVIS) )
                return false;

        if ( IS_SET( peexit->exit_info, EX_CAMOUFLAGE )
                && !CAN_DETECT(this,ACUTE_VISION))
                return false;

        if ( IS_SET( peexit->exit_info, EX_HIDDEN )
                && !CAN_DETECT(this, DETECT_HIDDEN) )
                return false;

        if ( IS_SET( peexit->exit_info, EX_FADE )
                && !CAN_DETECT(this, DETECT_FADE) )
                return false;

        if (in_room->isDark( ) && !IS_AFFECTED(this, AFF_INFRARED) )
                return false;

        return true;
}

bool Character::canEnter( Room *room ) 
{
    if (!room)
        return false;

    if (!can_see( room ))
        return false;
        
    if (room->clan != clan_none && room->clan != getClan( ))
        return false;
    
    if (!room->guilds.empty( ) && !room->guilds.isSet( getProfession( ) ))
        return false;
    
    if (room->behavior && !room->behavior->canEnter( this ))
        return false;

    return true;
}

bool Object::mustDisappear( Character *ch )
{
    if (ch->is_immortal( ))
        return false;

    if (item_type == ITEM_KEY && value[0] == 0)
        return true;
        
    if (item_type == ITEM_MAP && !value[0])
        return true;
    
    if (wear_loc == wear_stuck_in || wear_loc == wear_hold_leg)
        return false;

    if (pIndexData->limit > 0) {
        if (ch->getModifyLevel( ) > level + 20)
            return true;
            
        if (ch->getModifyLevel( ) < level - 3)
            return true;

        if (!IS_SET(ch->act, PLR_CONFIRMED))
            return true;
    }
    
    if (ch->getModifyLevel( ) < level - 3 && level > ANGEL)
        return true;
    
    return false;
}


short get_wear_level( Character *ch, Object *obj ) 
{
    int wear_mod, level_diff;
    
    wear_mod = ch->getProfession( )->getWearModifier( obj->item_type );
    level_diff = ch->getModifyLevel( ) - ch->getRealLevel( );
            
    return std::max( 1, obj->level - wear_mod - level_diff );
}

bool eyes_blinded( Character *ch )
{
    if (!IS_AFFECTED(ch, AFF_BLIND))
        return false;
        
    if (ch->getConfig( )->holy)
        return false;

    return true;
}

bool eyes_darkened( Character *ch )
{
    if (!ch->in_room->isDark( ))
        return false;

    if (ch->getConfig( )->holy)
        return false;

    if (ch->is_vampire() || IS_AFFECTED(ch, AFF_INFRARED))
        return false;
        
    if (IS_GHOST(ch) || IS_DEATH_TIME(ch))
        return false;

    return true;
}

