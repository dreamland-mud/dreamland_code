/* $Id$
 *
 * ruffina, 2004
 */

#include <sstream>
#include <errno.h>

#include "grammar_entities_impl.h"
#include "skillgroup.h"
#include "xmlindexdata.h"
#include "xmlmobilefactory.h"
#include "xmlobjectfactory.h"
#include "olcstate.h"
#include "olc.h"
#include "affect.h"
#include "mercdb.h"
#include "def.h"

void load_mobile(FILE *, MOB_INDEX_DATA *);
void load_object(FILE *, OBJ_INDEX_DATA *);
void load_area_header(FILE *, AREA_DATA *);
void save_mobile(FILE *, const MOB_INDEX_DATA *);
void save_object(FILE *, const OBJ_INDEX_DATA *);
void save_area_header(FILE *, const AREA_DATA *);

using namespace std;

#if 0
int
isreadfn(void *cookie, char *buf, int len)
{
    return ((std::istream *)cookie)->read(buf, len).gcount();
}

int
oswritefn(void *cookie, const char *buf, int len)
{
    if(((ostream *)cookie)->write(buf, len))
        return len;
    else
        return -1;
}
#endif 

/*------------------------------------
 * MOB_INDEX_DATA
 *-----------------------------------*/
XMLMobIndexData::XMLMobIndexData()
{
    next = NULL;
    spec_fun = NULL;
    area = NULL;
    player_name = str_dup("no name");
    short_descr = str_dup("(no short description)");
    long_descr = str_dup("(no long description)\n\r");
    description = str_empty;
    vnum = 0;
    count = 0;
    killed = 0;
    sex = 0;
    level = 0;
    act = ACT_IS_NPC;
    affected_by = 0;
    detection = 0;
    dam_type = 0;
    alignment = 0;
    hitroll = 0;
    race = str_dup("human");
    form = 0;
    parts = 0;
    imm_flags = 0;
    res_flags = 0;
    vuln_flags = 0;
    material = str_dup("none");
    off_flags = 0;
    size = SIZE_MEDIUM;
    ac[AC_PIERCE] = 0;
    ac[AC_BASH] = 0;
    ac[AC_SLASH] = 0;
    ac[AC_EXOTIC] = 0;
    hit[DICE_NUMBER] = 0;
    hit[DICE_TYPE] = 0;
    hit[DICE_BONUS] = 0;
    mana[DICE_NUMBER] = 0;
    mana[DICE_TYPE] = 0;
    mana[DICE_BONUS] = 0;
    damage[DICE_NUMBER] = 0;
    damage[DICE_TYPE] = 0;
    damage[DICE_BONUS] = 0;
    start_pos = POS_STANDING;
    default_pos = POS_STANDING;
    wealth = 0;
    new_format = true;
    behavior = 0;
    practicer.setRegistry( skillGroupManager );
    wrapper = 0;
    group = 0;
}

XMLMobIndexData::XMLMobIndexData(const MOB_INDEX_DATA &mob)
{
    static MOB_INDEX_DATA zeroMobIndex;
    
    *(MOB_INDEX_DATA*)this = zeroMobIndex;

    behavior         = 0;
    wrapper          = 0;
    next             = NULL;
    
    act              = mob.act;
    add_affected_by  = mob.add_affected_by;
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
    new_format       = mob.new_format;
    off_flags        = mob.off_flags;
    parts            = mob.parts;
    player_name      = str_dup(mob.player_name);
    practicer        = mob.practicer;
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

    static MOB_INDEX_DATA zMob;
    *(MOB_INDEX_DATA*)this = zMob;
}


void
XMLMobIndexData::fromXML( const XMLNode::Pointer& parent) throw( ExceptionBadType )
{
    clear();
#if 0
    XMLNode::Pointer node = parent->getFirstNode( );
    
    if (!node)
        return;

    if(node->getType( ) != XMLNode::XML_CDATA && node->getType( ) != XMLNode::XML_TEXT )
        throw ExceptionBadType( "XMLMobIndexData", node->getCData( ) );
 
    istringstream is(node->getCData( ));

    FILE *fd = fropen((istream *)&is, isreadfn);

    if(!fd)
        throw ExceptionBadType( "XMLMobIndexData", strerror(errno) );

    load_mobile(fd, this);
    fclose(fd);
#else
    XMLMobileFactory f;
    f.fromXML(parent);
    f.compat(this);
#endif

    vnum = parent->getAttribute( "vnum" ).toInt( );
    area = OLCState::get_vnum_area(vnum);
}

bool
XMLMobIndexData::toXML( XMLNode::Pointer& parent ) const
{
#if 0
    XMLNode::Pointer node( NEW );

    node->setType( XMLNode::XML_TEXT );

    ostringstream os;

    FILE *fd = fwopen((ostream *)&os, oswritefn);

    if(!fd)
        throw ExceptionBadType( "XMLMobIndexData", strerror(errno) );

    save_mobile(fd, this);
    fclose(fd);

    node->setCData( DLString( os.str( ) ) );

    parent->appendChild( node );
#else
    XMLMobileFactory f;
    f.init(this);
    f.toXML(parent);
#endif
    parent->insertAttribute("vnum", vnum);
    return true;
}

/*------------------------------------
 * OBJ_INDEX_DATA
 *-----------------------------------*/
XMLObjIndexData::XMLObjIndexData()
{
    static OBJ_INDEX_DATA zObj;
    *(OBJ_INDEX_DATA*)this = zObj;

    next = NULL;
    extra_descr = NULL;
    affected = NULL;
    area = NULL;
    name = str_dup("no name");
    short_descr = str_dup("(no short description)");
    description = str_dup("(no description)");
    vnum = 0;
    item_type = ITEM_TRASH;
    extra_flags = 0;
    wear_flags = 0;
    count = 0;
    weight = 0;
    cost = 0;
    material = str_dup("none");
    condition = 100;
    
    int v;
    for (v = 0; v < 5; v++)
        value[v] = 0;

    new_format = true;
    behavior = 0; 
    wrapper = 0;
    limit = -1;
    level = 0;
}

XMLObjIndexData::XMLObjIndexData(const obj_index_data &original)
{
    new_format   = original.new_format;
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
    Affect *pAf, *pAfNext;

    free_string(name);
    free_string(short_descr);
    free_string(description);

    for (pAf = affected; pAf; pAf = pAfNext) {
        pAfNext = pAf->next;
        ddeallocate(pAf);
    }

    for (pExtra = extra_descr; pExtra; pExtra = pExtraNext) {
        pExtraNext = pExtra->next;
        free_extra_descr(pExtra);
    }
    
    static OBJ_INDEX_DATA zObj;
    *(OBJ_INDEX_DATA*)this = zObj;
}

void
XMLObjIndexData::fromXML( const XMLNode::Pointer& parent) throw( ExceptionBadType )
{
    clear();
#if 0
    XMLNode::Pointer node = parent->getFirstNode( );
    
    if (!node)
        return;

    if(node->getType( ) != XMLNode::XML_CDATA && node->getType( ) != XMLNode::XML_TEXT )
        throw ExceptionBadType( "XMLObjIndexData", node->getCData( ) );
 
    istringstream is(node->getCData( ));

    FILE *fd = fropen((istream *)&is, isreadfn);

    if(!fd)
        throw ExceptionBadType( "XMLObjIndexData", strerror(errno) );

    load_object(fd, this);
    fclose(fd);
#else
    XMLObjectFactory f;
    f.fromXML(parent);
    f.compat(this);
#endif
    
    vnum = parent->getAttribute( "vnum" ).toInt( );
    area = OLCState::get_vnum_area(vnum);
}

bool
XMLObjIndexData::toXML( XMLNode::Pointer& parent ) const
{
#if 0
    XMLNode::Pointer node( NEW );
    
    node->setType( XMLNode::XML_TEXT );

    ostringstream os;
    FILE *fd = fwopen((ostream *)&os, oswritefn);

    if(!fd)
        throw ExceptionBadType( "XMLObjIndexData", strerror(errno) );

    save_object(fd, this);
    fclose(fd);

    node->setCData( DLString( os.str( ) ) );

    parent->appendChild( node );
#else
    XMLObjectFactory f;
    f.init(this);
    f.toXML(parent);
#endif
    parent->insertAttribute("vnum", vnum);
    return true;
}

int XMLObjIndexData::getVnum() const
{
    return vnum;
}

area_data * XMLObjIndexData::getArea() const
{
    return area;
}

Scripting::Object * XMLObjIndexData::getWrapper() const
{
    OBJ_INDEX_DATA *original = get_obj_index(vnum);
    if (original)
        return original->wrapper;
    return NULL;
}

const char * XMLObjIndexData::getIndexType() const
{
    return "obj";
}

int XMLMobIndexData::getVnum() const
{
    return vnum;
}

area_data * XMLMobIndexData::getArea() const
{
    return area;
}

Scripting::Object * XMLMobIndexData::getWrapper() const
{
    MOB_INDEX_DATA *original = get_mob_index(vnum);
    if (original)
        return original->wrapper;
    return NULL;
}

const char * XMLMobIndexData::getIndexType() const
{
    return "mob";
}

