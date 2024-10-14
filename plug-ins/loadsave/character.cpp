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
GSN(fire_breath);
GSN(sand_storm);
GSN(dirt_kicking);

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
        
    if (room->pIndexData->clan != clan_none && room->pIndexData->clan != getClan( ))
        return false;
    
    if (!room->pIndexData->guilds.empty( ) && !room->pIndexData->guilds.isSet( getProfession( ) ))
        return false;
    
    if (room->behavior && !room->behavior->canEnter( this ))
        return false;

    return true;
}

bool Object::mustDisappear( Character *ch )
{
    if (ch->is_immortal( ))
        return false;

    if (item_type == ITEM_KEY && value0() == 0)
        return true;
        
    if (item_type == ITEM_MAP && !value0())
        return true;
    
    if (wear_loc == wear_stuck_in || wear_loc == wear_hold_leg)
        return false;

    if (pIndexData->limit > 0) {
        if (ch->getModifyLevel( ) > level + 20) {
        ch->pecho("Ты уже слишком опыт{Smен{Sfна{Sx для этого лимита.");
            return true;
    }
            
        if (ch->getModifyLevel( ) < level - 3) {
        ch->pecho("Ты еще слишком неопыт{Smен{Sfна{Sx для этого лимита.");
            return true;
    }

        if (!IS_SET(ch->act, PLR_CONFIRMED)) {
        ch->pecho("Чтобы пользоваться лимитами, надо сначала {hhподтвердить{x описание.");
            return true;
    }
    }
    
    if (ch->getModifyLevel( ) < level - 3 && level > ANGEL)
        return true;
    
    return false;
}


bool eyes_blinded( Character *ch )
{
    if (!IS_AFFECTED(ch, AFF_BLIND))
        return false;
        
    if (ch->getConfig( ).holy)
        return false;

    return true;
}

bool eyes_darkened( Character *ch )
{
    if (!ch->in_room->isDark( ))
        return false;

    if (ch->getConfig( ).holy)
        return false;

    if (ch->is_vampire() || IS_AFFECTED(ch, AFF_INFRARED))
        return false;
        
    if (IS_GHOST(ch) || IS_DEATH_TIME(ch))
        return false;

    return true;
}

void eyes_blinded_msg( Character *ch )
{
    if (!IS_AFFECTED(ch, AFF_BLIND))
        return;

    for (auto &paf: ch->affected.findAllWithBits(&affect_flags, AFF_BLIND)) {
        if (paf->type == gsn_fire_breath)
            ch->pecho( "Твои глаза слезятся из-за дыма, и ты ничего не видишь." );
        else if (paf->type == gsn_sand_storm)
            ch->pecho( "Песок в глазах мешает тебе что-либо разглядеть." );
        else if (paf->type == gsn_dirt_kicking)
            ch->pecho( "Ты ничего не видишь из-за пыли, попавшей в глаза." );
        else
            continue;

        return;
    }

    ch->pecho( "Твои глаза слепы, ты ничего не видишь!" );
}


