/* $Id$
 *
 * ruffina, 2004
 */

#include <sstream>
#include <errno.h>
#include <string.h>

#include "wrappermanager.h"
#include "wrapperbase.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "skillgroup.h"
#include "xmlindexdata.h"
#include "xmlmobilefactory.h"
#include "xmlobjectfactory.h"
#include "room.h"
#include "olcstate.h"
#include "olc.h"
#include "affect.h"

#include "def.h"

void load_mobile(FILE *, MOB_INDEX_DATA *);
void load_object(FILE *, OBJ_INDEX_DATA *);
void load_area_header(FILE *, AreaIndexData *);
void save_mobile(FILE *, const MOB_INDEX_DATA *);
void save_object(FILE *, const OBJ_INDEX_DATA *);
void save_area_header(FILE *, const AreaIndexData *);

using namespace std;

/*------------------------------------
 * MOB_INDEX_DATA
 *-----------------------------------*/
XMLMobIndexData::XMLMobIndexData()
{
}

XMLMobIndexData::XMLMobIndexData(const MOB_INDEX_DATA &mob)
                    : XMLMobIndexData()
{
    act              = mob.act;
    affected_by      = mob.affected_by;
    alignment        = mob.alignment;
    area             = mob.area;
    count            = mob.count;
    dam_type         = mob.dam_type;
    default_pos      = mob.default_pos;
    description      = str_dup(mob.description);
    detection        = mob.detection;
    form             = mob.form;
    group            = mob.group;
    hitroll          = mob.hitroll;
    imm_flags        = mob.imm_flags;
    killed           = mob.killed;
    level            = mob.level;
    long_descr       = str_dup(mob.long_descr);
    material         = str_dup(mob.material);
    off_flags        = mob.off_flags;
    parts            = mob.parts;
    player_name      = str_dup(mob.player_name);
    practicer        = mob.practicer;
    religion         = mob.religion;
    affects          = mob.affects;
    race             = str_dup(mob.race);
    res_flags        = mob.res_flags;
    sex              = mob.sex;
    gram_number      = mob.gram_number;
    short_descr      = str_dup(mob.short_descr);
    size             = mob.size;
    spec_fun         = mob.spec_fun;
    start_pos        = mob.start_pos;
    vnum             = mob.vnum;
    vuln_flags       = mob.vuln_flags;
    wealth           = mob.wealth;
    clan             = mob.clan;
    
    memcpy(hit, mob.hit, sizeof(mob.hit));
    memcpy(mana, mob.mana, sizeof(mob.mana));
    memcpy(damage, mob.damage, sizeof(mob.damage));
    memcpy(ac, mob.ac, sizeof(mob.ac));
}

XMLMobIndexData::~XMLMobIndexData()
{
    clear();
}

void
XMLMobIndexData::clear()
{
    free_string(player_name);
    free_string(short_descr);
    free_string(long_descr);
    free_string(description);
    free_string(material);
    free_string(race);
}


void
XMLMobIndexData::fromXML( const XMLNode::Pointer& parent) 
{
    clear();
    XMLMobileFactory f;
    f.fromXML(parent);
    f.compat(this);

    vnum = parent->getAttribute( "vnum" ).toInt( );
    area = OLCState::get_vnum_area(vnum);
}

bool
XMLMobIndexData::toXML( XMLNode::Pointer& parent ) const
{
    XMLMobileFactory f;
    f.init(this);
    f.toXML(parent);

    parent->insertAttribute("vnum", vnum);
    return true;
}

/*------------------------------------
 * OBJ_INDEX_DATA
 *-----------------------------------*/
XMLObjIndexData::XMLObjIndexData()
{
}

XMLObjIndexData::XMLObjIndexData(const obj_index_data &original)
{
    name         = str_dup(original.name);
    short_descr  = str_dup(original.short_descr);
    description  = str_dup(original.description);
    vnum         = original.vnum;
    reset_num    = original.reset_num;
    material     = str_dup(original.material);
    item_type    = original.item_type;
    extra_flags  = original.extra_flags;
    wear_flags   = original.wear_flags;
    level        = original.level;
    condition    = original.condition;
    gram_gender  = original.gram_gender;
    count        = original.count;
    weight       = original.weight;
    cost         = original.cost;
    memcpy(value, original.value, sizeof(value));
    limit        = original.limit;
    area         = original.area;
    if(original.behavior)
        behavior         = XMLDocument::Pointer( NEW, **original.behavior );
    else
        behavior         = 0;
}

XMLObjIndexData::~XMLObjIndexData()
{
    clear();
}

void
XMLObjIndexData::clear()
{
    EXTRA_DESCR_DATA *pExtra, *pExtraNext;

    free_string(name);
    free_string(short_descr);
    free_string(description);

    affected.deallocate();

    for (pExtra = extra_descr; pExtra; pExtra = pExtraNext) {
        pExtraNext = pExtra->next;
        free_extra_descr(pExtra);
    }
}

void
XMLObjIndexData::fromXML( const XMLNode::Pointer& parent) 
{
    clear();

    XMLObjectFactory f;
    f.fromXML(parent);
    f.compat(this);
    
    vnum = parent->getAttribute( "vnum" ).toInt( );
    area = OLCState::get_vnum_area(vnum);
}

bool
XMLObjIndexData::toXML( XMLNode::Pointer& parent ) const
{
    XMLObjectFactory f;
    f.init(this);
    f.toXML(parent);

    parent->insertAttribute("vnum", vnum);
    return true;
}

int XMLObjIndexData::getVnum() const
{
    return vnum;
}

AreaIndexData * XMLObjIndexData::getArea() const
{
    return area;
}

const char * XMLObjIndexData::getIndexType() const
{
    return "obj";
}

int XMLMobIndexData::getVnum() const
{
    return vnum;
}

AreaIndexData * XMLMobIndexData::getArea() const
{
    return area;
}

const char * XMLMobIndexData::getIndexType() const
{
    return "mob";
}

XMLRoomIndexData::XMLRoomIndexData(RoomIndexData *room)
{
    this->room = room;
}

int XMLRoomIndexData::getVnum() const
{
    return room->vnum;
}

AreaIndexData * XMLRoomIndexData::getArea() const
{
    return room->areaIndex;
}

const char * XMLRoomIndexData::getIndexType() const
{
    return "room";
}

