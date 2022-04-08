/* $Id: room.cpp,v 1.1.2.9.6.6 2009/09/19 00:53:17 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "room.h"

#include "fenia/register-impl.h"

#include "skill.h"
#include "skillmanager.h"
#include "clanreference.h"
#include "profession.h"
#include "affect.h"
#include "object.h"
#include "character.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

CLAN(none);
PROF(none);
LIQ(none);

extra_exit_data *ExtraExitList::find(const DLString &keyword) const
{
    for (auto &eexit: *this)
        if (is_name(keyword.c_str(), eexit->keyword))
            return eexit;

    return 0;
}

bool ExtraExitList::findAndDestroy(const DLString &keyword)
{
    extra_exit_data *eexit = find(keyword);
    if (!eexit)
        return false;

    remove(eexit);
    delete eexit;
    return true;
}

RoomIndexData::RoomIndexData()
        : extra_descr(0),
          name(&str_empty[0]), description(&str_empty[0]), 
          vnum(0), room_flags(0),
          sector_type(0), heal_rate(100), mana_rate(100),
          clan( clan_none ),  guilds( professionManager ),
          liquid( liq_none ), areaIndex(0), room(0)
{
    for (int i = 0; i < DIR_SOMEWHERE; i++) 
        exit[i] = 0;
}

Room::Room( ) : 
                position( -1 ),
                people( 0 ), contents( 0 ),
                area( 0 ),
                owner(&str_empty[0]),
                vnum( 0 ), room_flags( 0 ), 
                light( 0 ),
                affected_by( 0 ),
                behavior( RoomBehavior::NODE_NAME ),
                pIndexData(0),
                sector_type(SECT_MAX),
                liquid(liq_none),
                mod_heal_rate( 0 ), mod_mana_rate( 0 ), ID(0)
{
    for (int i = 0; i < DIR_SOMEWHERE; i++) 
        exit[i] = 0;
}

int Room::getHealRate() const
{
    return pIndexData->heal_rate + mod_heal_rate;
}

int Room::getManaRate() const
{
    return pIndexData->mana_rate + mod_mana_rate;
}

bool Room::isOwner( Character *ch ) const
{
    if (owner == 0 || owner[0] == '\0')
        return false;

    return is_name( ch->getNameC(), owner );
}

bool Room::hasExits() const
{
    for (int i = 0; i < DIR_SOMEWHERE; i++)
        if (exit[i] != 0)
            return true;
    return false;
}

const char * Room::areaName() const
{
    return pIndexData->areaIndex->name;
}


bool Room::isPrivate( ) const
{
    return getCapacity( ) == 0;
}

int Room::getCapacity( ) const
{
    Character *rch;
    int count;

    if (IS_SET( room_flags, ROOM_IMP_ONLY ))
        return 0;

    count = 0;
    for (rch = people; rch != 0; rch = rch->next_in_room)
        count++;

    if (IS_SET(room_flags, ROOM_PRIVATE))
        return max( 0, 2 - count );

    if (IS_SET(room_flags, ROOM_SOLITARY))
        return max( 0, 1 - count );

    return -1;
}


bool Room::isCommon( ) 
{
    if (IS_SET(room_flags, ROOM_IMP_ONLY | ROOM_GODS_ONLY | ROOM_HEROES_ONLY ))
        return false;

    if (IS_SET(room_flags,ROOM_NEWBIES_ONLY))
        return false;

    if (IS_SET(area->area_flag, AREA_WIZLOCK))
        return false;

    if (pIndexData->clan != clan_none)
        return false;
    
    if (!pIndexData->guilds.empty( ))
        return false;
    
    if (behavior && !behavior->isCommon( ))
        return false;

    return true;
}



/*
 * Room light
 */
bool Room::isDark( ) const
{
    if (light > 0)
        return false;

    if (IS_SET(room_flags, ROOM_DARK))
        return true;
    
    if (getSectorType() == SECT_INSIDE || getSectorType() == SECT_CITY)
        return false;

    if (weather_info.sunlight == SUN_LIGHT || weather_info.sunlight == SUN_RISE)
        return false;

    return true;
}

list<Character*> Room::getPeople ( )
{
        Character *tmp_vict;

        list<Character*> people;        

        for (tmp_vict = this->people; tmp_vict != 0; tmp_vict = tmp_vict->next_in_room)
        {
                people.push_back(tmp_vict);
        }

        return people;
}

int Room::getSectorType() const
{
    return sector_type == SECT_MAX ? pIndexData->sector_type : sector_type;
}

LiquidReference & Room::getLiquid()
{
    if (liquid == liq_none)
        return pIndexData->liquid;

    return liquid;
}
