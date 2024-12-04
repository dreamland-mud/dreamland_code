/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          object.cpp  -  description
                             -------------------
    begin                : Fri Apr 27 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "class.h"
#include "logstream.h"
#include "noun.h"
#include "grammar_entities_impl.h"
#include "ru_pronouns.h"
#include "string_utils.h"
#include "json_utils.h"
#include "fenia/register-impl.h"

#include "wrapperbase.h"
#include "feniamanager.h"
#include "string_utils.h"
#include "objectbehavior.h"
#include "object.h"
#include "affect.h"
#include "character.h"

#include "def.h"

WEARLOC(none);

// Global list of all objects
Object * object_list;

const DLString Object::TYPE = "Object";

Object::Object( ) :
                ID( 0 ),
                next( 0 ), prev( 0 ),
                next_content( 0 ), contains( 0 ), in_obj( 0 ), on( 0 ),
                carried_by( 0 ), pIndexData( 0 ),
                in_room( 0 ),
                item_type( 0 ),
                extra_flags( 0 ), wear_flags( 0 ), 
                wear_loc( wear_none ),
                weight( 0 ), cost( 0 ), level( 0 ), condition( 0 ),
                timer( 0 ), timestamp( -1 ),
                extracted( 0 ), water_float( 0 ),
                count( 0 ),
                gram_gender(MultiGender::UNDEF),
                behavior( ObjectBehavior::NODE_NAME ),
                reset_obj(0), reset_mob(0), reset_room(0)
{
        for( int i = 0; i < 5; i++ ) value[i] = 0;
}

void Object::extract( )
{
        affected.deallocate();

        extraDescriptions.deallocate();

        keyword.clearValues();
        short_descr.clearValues();
        description.clearValues();
        
        next = 0;
        prev = 0;
        next_content = 0;
        contains = 0;
        in_obj = 0;
        on = 0;
        carried_by = 0;
        pIndexData = 0;
        in_room = 0;
        pocket = "";
        owner = "";
        cachedNouns.clear( );
        item_type = 0;
        extra_flags = 0;
        wear_flags = 0;
        wear_loc.assign( wear_none );
        weight = 0;
        cost = 0;
        level = 0;
        condition = 0;
        material.clear();
        timer = 0;
        timestamp = -1;
        from.clear();
        extracted = true;
        water_float = 0;
        killer.clear();
        count = 0;
        gram_gender = MultiGender::UNDEF;
        props.clear();

        wrapper = 0;
        behavior.clear( );
        reset_obj = 0;
        reset_mob = 0;
        reset_room = 0;
        ID = 0;
        
        for( int i = 0; i < 5; i++ ) value[i] = 0;
}

Object::~Object( )
{
    affected.deallocate();

    extraDescriptions.deallocate();
}

Character * Object::getCarrier( ) {
    if (!carried_by)
        if (in_obj)
            return in_obj->getCarrier( );
        else
            return NULL;
    else
        return carried_by;
}

Room * Object::getRoom( ) {
    if (carried_by)
        return carried_by->in_room;
    else if (in_room)
        return in_room;
    else if (in_obj)
        return in_obj->getRoom( );
    else
        return NULL;
}

/* object condition aliases */
const char *Object::get_cond_alias( void )
{
 const char *stat;

 if      ( condition >  99 ) stat = "{Cотл.{x";
 else if ( condition >= 80 ) stat = "{cхор.{x";
 else if ( condition >= 60 ) stat = "{Yнорм.{x";
 else if ( condition >= 40 ) stat = "{yср.{x";
 else if ( condition >= 20 ) stat = "{Rпл.{x";
 else                        stat = "{rуж.{x";

 return stat;
}

int Object::floating_time( )
{
 int  ftime;

 ftime = 0;
 switch( item_type )
 {
    default: break;
    case ITEM_KEY       : ftime = 1;    break;
    case ITEM_LOCKPICK  : ftime = 1;    break;
    case ITEM_ARMOR     : ftime = 2;    break;
    case ITEM_TREASURE  : ftime = 2;    break;
    case ITEM_PILL      : ftime = 2;    break;
    case ITEM_POTION    : ftime = 3;    break;
    case ITEM_TRASH     : ftime = 3;    break;
    case ITEM_PARCHMENT : ftime = 3;    break;
    case ITEM_SPELLBOOK : ftime = 3;    break;
    case ITEM_FOOD      : ftime = 4;    break;
    case ITEM_CONTAINER : ftime = 5;    break;
    case ITEM_KEYRING   : ftime = 5;    break;
    case ITEM_CORPSE_NPC: ftime = 10;   break;
    case ITEM_CORPSE_PC : ftime = 10;   break;
 }
 ftime = number_fuzzy( ftime ) ;

 return max( 0, ftime );
}

bool Object::may_float(void)
{
    if (behavior)
        if (behavior->mayFloat( ))
            return true;
    
    switch (item_type) {
    case ITEM_BOAT: 
    case ITEM_PORTAL:
        return true;
        
    default:
        return false;
    }
}

// Add or update extra descr value for this exact keywords in given language
ExtraDescription * Object::addExtraDescr( const DLString &keys, const DLString &value, lang_t lang )
{
    ExtraDescription *ed = extraDescriptions.find(keys);

    if (!ed) {
        ed = new ExtraDescription;
        extraDescriptions.push_back(ed);
    }

    ed->keyword = keys;
    ed->description[lang] = value;
    return ed;
}

const DLString& Object::getExtraDescr(const DLString& keys, lang_t lang)
{
    ExtraDescription *ed = extraDescriptions.find(keys);
    if (ed)
        return ed->description.get(lang);

    return DLString::emptyString;
}

ExtraDescription* Object::getProperDescription()
{
    return extraDescriptions.find(String::toString(getKeyword()));
}

ExtraDescription* Object::addProperDescription()
{    
    ExtraDescription *ed = getProperDescription();
    if (ed)
        return ed;
    
    ed = new ExtraDescription();
    ed->keyword = String::toString(getKeyword());
    extraDescriptions.push_back(ed);
    return ed;
}

void Object::clearProperDescription()
{
    extraDescriptions.findAndDestroy(String::toString(getKeyword()));
}


/*
 * String fields set/get methods
 */
void Object::setKeyword( const DLString &str, lang_t lang )
{
    keyword[lang] = str;
}

void Object::setKeyword(const DLString& str)
{
    keyword.fromMixedString(str);
}

void Object::setKeyword(const XMLMultiString &str)
{
    keyword = str;
}

void Object::setShortDescr(const DLString &str, lang_t lang)
{
    short_descr[lang] = str;
    updateCachedNoun(lang);
}

void Object::setDescription( const DLString &str, lang_t lang )
{
    description[lang] = str;
}

void Object::setMaterial( const DLString &material )
{
    this->material = material;
}

void Object::setOwner( const DLString &newOwner )
{
    this->owner = newOwner;
}

DLString Object::getShortDescr( char gram_case, lang_t lang )
{
    return toNoun( )->decline( gram_case ); // TODO toNoun should take 'lang' argument
}

static bool is_anti_align( Character *ch, int flags )
{
    if (IS_SET(flags, ITEM_ANTI_EVIL) && IS_EVIL( ch ))
        return true;

    if (IS_SET(flags, ITEM_ANTI_GOOD) && IS_GOOD( ch ))
        return true;

    if (IS_SET(flags, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL( ch ))
        return true;
    
    return false;
}

bool Object::isAntiAligned( Character *ch ) const
{
    return is_anti_align( ch, extra_flags );
}

bool Object::wasAntiAligned( Character *ch ) const
{
    return is_anti_align( ch, pIndexData->extra_flags );
}

/*
 * Return # of objects which an object counts as.
 */
int Object::getNumber( ) const
{
    int number;

    if (item_type == ITEM_MONEY)
        number = 0;
    else
        number = 1;

    return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int Object::getWeight( ) const
{
    int sum;
    Object *tobj;

    sum = weight;

    for (tobj = contains; tobj != 0; tobj = tobj->next_content)
        sum += tobj->getWeight( ) * getWeightMultiplier( ) / 100;

    return sum;
}

/*
 * True object weight w/o container multipliers 
 */
int Object::getTrueWeight( ) const
{
    int sum;
    Object *tobj;

    sum = weight;

    for (tobj = contains; tobj != 0; tobj = tobj->next_content)
        sum += tobj->getWeight( );

    return sum;
}


bool Object::isAffected( int sn ) const
{
    return affected.find(sn) != 0;
}

bool Object::hasOwner( const Character *ch ) const
{
    if (owner.empty())
        return false;

    if (ch->is_npc( ))
        return false;

    return ch->getNameC() == owner;
}

using namespace Grammar;

Noun::Pointer Object::toNoun( const DLObject *forWhom, int flags ) const
{
    const Character *wch = dynamic_cast<const Character *>(forWhom);
    
    if (IS_SET(flags, FMT_INVIS)) {
        if (wch && !wch->can_see( this )) 
            return something;
    }
    
    // TODO toNoun takes lang parameter from 'forWhom'
    return cachedNouns.find(LANG_DEFAULT)->second;
}

void Object::updateCachedNouns()
{
    for (int l = LANG_MIN; l < LANG_MAX; l++)
        updateCachedNoun((lang_t)l);
}

void Object::updateCachedNoun(lang_t lang)
{
    MultiGender g = gram_gender == MultiGender::UNDEF 
        ? pIndexData->gram_gender : gram_gender;

    if (cachedNouns.find(lang) == cachedNouns.end()) { 
        cachedNouns[lang] = InflectedString::Pointer( NEW, 
                                             getShortDescr(lang), 
                                             g );
    }
    else {
        cachedNouns[lang]->setFullForm( getShortDescr(lang) );
        cachedNouns[lang]->setGender( g );
    }
}

int Object::getWeightMultiplier( ) const
{
    if (item_type == ITEM_CONTAINER) {
        if (value4() <= 0) return 100;
        else return value4();
    }
    else return 100;
}

DLString Object::getProperty(const DLString &key) const
{
    // Find property on the item itself
    DLString jsonValue;
    if (JsonUtils::findValue(props, key, jsonValue))
        return jsonValue;

    // Look in index data props 
    return pIndexData->getProperty(key);
}

void Object::removeProperty(const DLString &key)
{
    props.removeMember(key);
}

void Object::setProperty(const DLString &key, const DLString &value)
{
    props[key] = value;
}

void Object::setProperty(const DLString &key, const DLString &subkey, const DLString &value)
{
    props[key][subkey] = value;
}

static bool get_value0_from_proto(const Object *obj)
{
    switch (obj->item_type) {
        case ITEM_WEAPON: 
            // Always grab weapon's class from the proto, unless it's a restring (e.g. cleric's mace).
            return !obj->getRealShortDescr(LANG_DEFAULT).empty() && obj->getProperty("tier").empty();
        default:
            return false;
    }
}

static bool get_value1_from_proto(const Object *obj)
{
    switch (obj->item_type) {
        case ITEM_WEAPON: 
            // Bypass object dices and return those from proto if the weapon is not enchanted, not
            // dynamically created (ranger staff, bow, arrow, stone) and not restringed (cleric's mace).
            // TODO: deprecated?
            return !obj->getRealShortDescr(LANG_DEFAULT).empty() 
                    && obj->level == obj->pIndexData->level
                    && obj->getProperty("tier").empty();
        default:
            return false;
    }
}

static bool get_value2_from_proto(const Object *obj)
{
    return get_value1_from_proto(obj);
}

static bool get_value3_from_proto(const Object *obj)
{
    switch (obj->item_type) {
        case ITEM_WEAPON: 
            // Grab weapon damage type (pierce, slash) from proto unless it's a restring (e.g. cleric's mace).
            return !obj->getRealShortDescr(LANG_DEFAULT).empty() && obj->getProperty("tier").empty();
        default:
            return false;
    }
}

static bool get_value4_from_proto(const Object *obj)
{
    return false;
}

bool Object::getsValueFromProto(int index) const
{
    switch (index) {
        case 0: return get_value0_from_proto(this);
        case 1: return get_value1_from_proto(this);
        case 2: return get_value2_from_proto(this);
        case 3: return get_value3_from_proto(this);
        case 4: return get_value4_from_proto(this);
        default:
            return false;        
    }
}

int Object::itemOrProtoValue(int index) const
{
    return getsValueFromProto(index) ? pIndexData->value[index] : value[index];
}

void Object::itemOrProtoValue(int index, int v)
{
    value[index] = v;
}


int Object::valueByIndex(int index) const
{
    switch (index) {
        case 0: return value0();
        case 1: return value1();
        case 2: return value2();
        case 3: return value3();
        case 4: return value4();
        default:
            bug("Object::valueByIndex invalid index [%d] for [%d]", index, pIndexData->vnum);
            break;
    }
    return -1;
}

void Object::valueByIndex(int index, int value)
{
    switch (index) {
        case 0: value0(value); break;
        case 1: value1(value); break;
        case 2: value2(value); break;
        case 3: value3(value); break;
        case 4: value4(value); break;
        default:
            bug("Object::valueByIndex invalid index [%d] for [%d] and value [%d]", index, pIndexData->vnum, value);
            break;
    }
}

list<Object *> Object::getItems()
{
    list<Object *> items;

    for (Object *obj = contains; obj; obj = obj->next_content)
        items.push_back(obj);

    return items;
}

const DLString & Object::getRealKeyword( lang_t lang ) const
{
    return keyword.get(lang);
}

const DLString & Object::getRealShortDescr( lang_t lang ) const
{
    return short_descr.get(lang);
}

const DLString & Object::getRealDescription( lang_t lang ) const
{
    return description.get(lang);
}

const DLString & Object::getKeyword( lang_t lang ) const
{
    return String::firstNonEmpty(keyword, pIndexData->keyword, lang);
}

const DLString & Object::getShortDescr( lang_t lang ) const
{
    return String::firstNonEmpty(short_descr, pIndexData->short_descr, lang);
}

const DLString & Object::getDescription( lang_t lang ) const
{
    return String::firstNonEmpty(description, pIndexData->description, lang);
}