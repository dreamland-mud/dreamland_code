#include "mobilefactory.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "json_utils.h"
#include "race.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "religion.h"
#include "behavior.h"
#include "dlscheduler.h"
#include "dreamland.h"
#include "flagtable.h"
#include "autoflags.h"
#include "dl_strings.h"
#include "def.h"

CLAN(none);

MOB_INDEX_DATA * mob_index_hash[1024];


mob_index_data::mob_index_data( ) 
                     : practicer( skillGroupManager ), 
                       religion( religionManager ),
                       affects(skillManager),
                       behaviors(behaviorManager),
                       wrapper ( 0 ),
                       clan(clan_none)

{
    next = NULL;
    vnum = 0;
    group = 0;
    count = 0;
    killed = 0;
    act = ACT_IS_NPC;
    affected_by = 0;
    detection = 0;
    alignment = 0;
    level = 0;
    hitroll = 0;
    hit[DICE_NUMBER] = 0;
    hit[DICE_TYPE] = 0;
    hit[DICE_BONUS] = 0;
    mana[DICE_NUMBER] = 0;
    mana[DICE_TYPE] = 0;
    mana[DICE_BONUS] = 0;
    damage[DICE_NUMBER] = 0;
    damage[DICE_TYPE] = 0;
    damage[DICE_BONUS] = 0;
    ac[AC_PIERCE] = 0;
    ac[AC_BASH] = 0;
    ac[AC_SLASH] = 0;
    ac[AC_EXOTIC] = 0;
    dam_type = 0;
    off_flags = 0;
    imm_flags = 0;
    res_flags = 0;
    vuln_flags = 0;
    start_pos = POS_STANDING;
    default_pos = POS_STANDING;
    sex = 0;
    race = "human";
    wealth = 0;
    form = 0;
    parts = 0;
    size = SIZE_MEDIUM;
    material = "none";
    area = NULL;
}

mob_index_data::~mob_index_data()
{
    
}

DLString mob_index_data::getProperty(const DLString &key) const
{
    // Look in props on index data: props[key] or props["blablah"][key]
    DLString jsonValue;
    JsonUtils::findValue(props, key, jsonValue);
    return jsonValue;
}

const char * mob_index_data::getDescription( lang_t lang ) const
{
    return description.get(lang).c_str();
}

const char * mob_index_data::getShortDescr( lang_t lang ) const
{
    return short_descr.get(lang).c_str();
}

const char * mob_index_data::getLongDescr( lang_t lang ) const
{
    return long_descr.get(lang).c_str();
}

int mob_index_data::getSize() const
{
    return size == NO_FLAG ? raceManager->find(race)->getSize() : size;
}

MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % 1024];
          pMobIndex != 0;
          pMobIndex  = pMobIndex->next )
    {
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;
    }

    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
	    LogStream::sendError() << "get_mob_index: vnum " << vnum << " not found on world startup" << endl;

    return 0;
}

