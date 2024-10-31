/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          character.cpp  -  description
                             -------------------
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/


#include <sstream>
#include <iostream>

#include "logstream.h"
#include "class.h"
#include "noun.h"
#include "grammar_entities_impl.h"

#include "fenia/register-impl.h"

#include "wrapperbase.h"
#include "feniamanager.h"

#include "mobilebehavior.h"

#include "skill.h"
#include "skillmanager.h"
#include "skillreference.h"
#include "spelltarget.h"
#include "clanreference.h"
#include "race.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "character.h"
#include "affect.h"

#include "dreamland.h"
#include "def.h"



using Scripting::Register;
using Scripting::RegisterList;

CLAN(none);
PROF(none);
RACE(none);
GSN(deafen);
GSN(doppelganger);
RELIG(none);

PlayerConfig::PlayerConfig( )
{
    holy = color = false;
    ruskills = runames = rucommands = ruexits = ruother = false;
}

PlayerConfig::PlayerConfig( const PCharacter *ch )
{
    holy = IS_SET(ch->act, PLR_HOLYLIGHT);
    color = IS_SET(ch->act, PLR_COLOR);
    ruskills = ch->config.isSet(CONFIG_RUSKILLS);
    runames = ch->config.isSet(CONFIG_RUNAMES);
    rucommands = ch->config.isSet(CONFIG_RUCOMMANDS);
    ruexits = ch->config.isSet(CONFIG_RUEXITS);
    ruother = ch->config.isSet(CONFIG_RUOTHER);
}

Character::Character( )
        :       ethos( ETHOS_NULL, &ethos_table ),
                act( 0, &plr_flags ),
                comm( 0, &comm_flags ),   
                add_comm( 0, &add_comm_flags ),
                imm_flags( 0, &::imm_flags ),
                res_flags( 0, &::res_flags ),
                vuln_flags( 0, &::vuln_flags ),
                affected_by( 0, &affect_flags ),
                detection( 0, &detect_flags ),
                position( POS_STANDING, &position_table ),
                posFlags( 0, &position_flags ),
                wearloc( wearlocationManager ),
                armor( &ac_type ),
                perm_stat( &stat_table ), 
                mod_stat( &stat_table ),
                form(0, &form_flags),
                parts(0, &part_flags),
                trap( 0, &trap_flags )
{
}

Character::~Character(void)
{
    if (carrying)
        LogStream::sendFatal() << "~Character: inventory not empty" << endl;

    if (!affected.empty())
        LogStream::sendFatal() << "~Character: affected not empty" << endl;
}

/****************************************************************************
 * recycle 
 ****************************************************************************/
void Character::init( )
{
    ID = 0;
    dead = false;
    level = 0;
    clan.assign( clan_none );
    sex = SEX_NEUTRAL;
    race.assign( race_none );
    religion.assign( god_none );
    last_fight_time = 0;

    extracted = false;
    reply = 0;
    next = 0;
    prev = 0;
    next_in_room = 0;
    master = 0;
    leader = 0;
    doppel = 0;
    fighting = 0;
    last_fought = 0;
    desc = 0;
    affected.clear();
    carrying = 0;
    on = 0;
    in_room = 0;
    was_in_room = 0;
    ethos.setValue( ETHOS_NULL );
    timer = 0;
    wait = 0;
    daze = 0;

    hit = 20;
    max_hit = 20;
    mana = 100;
    max_mana = 100;
    move = 100;
    max_move = 100;
    gold = 0;
    silver = 0;
    exp = 0;

    invis_level = 0;
    incog_level = 0;
    
    prompt.clear( );
    batle_prompt.clear( );
    lines = 0;

    act = 0;
    comm = 0;
    add_comm = 0;
    imm_flags = 0;
    res_flags = 0;
    vuln_flags = 0;
    affected_by = 0;
    detection = 0;
    position = POS_STANDING;
    posFlags = 0;
    wearloc.clear( );

    carry_weight = 0;
    carry_number = 0;
    saving_throw = 0;
    alignment = 0;
    hitroll = 0;
    damroll = 0;
    armor.clear( );
    armor.fill( 100 );
    wimpy = 0;
    dam_type = DAMW_PUNCH;
    heal_gain = 0;
    mana_gain = 0;
    mod_beats = 0;

    perm_stat.clear( );
    perm_stat.fill( 13 );
    mod_stat.clear( );

    form = 0;
    parts = 0;
    size = 0;
    material.clear();

    endur = 0;
    
    riding = false;
    mount =  0;

    ambushing.clear();

    death_ground_delay = 0;
    trap.clear( );

    wrapper = 0;
}

void Character::extract( )
{
    if (carrying)
        throw Exception( getNameP('1') + " extract: inventory not empty" );
    
    affected.deallocate();

    init( );
}

/*****************************************************************************
 * set-get methods inherited from CharacterMemoryInterface
 * with some additional methods needed
 *****************************************************************************/
short Character::getLevel( ) const 
{
    return level.getValue( );
}
void Character::setLevel( short level ) 
{
    this->level.setValue( level );
}
short Character::getRealLevel( ) const 
{
    return level.getValue( );
}
ClanReference & Character::getClan( ) 
{
    return clan;
}
void Character::setClan( const ClanReference & clan ) 
{
    this->clan.assign( clan );
}

ProfessionReference & Character::getProfession( ) 
{
    return profession;
}
void Character::setProfession( const ProfessionReference & profession ) 
{
    this->profession.assign( profession );
}

ReligionReference & Character::getReligion( ) 
{
    return religion;
}
void Character::setReligion( const ReligionReference &religion )
{
    this->religion.assign( religion );
}
RaceReference &Character::getRace( ) 
{
    return race;
}
void Character::setRace( const RaceReference & race ) 
{
    this->race.assign( race );
}

int Character::getAlignment( ) const
{
        return alignment;
}

void Character::setAlignment( int value )
{
        alignment = value;
}

int Character::getEthos( ) const
{
        return ethos;
}

void Character::setEthos( int value )
{
        ethos = value;
}

PCharacter * Character::getPlayer( )
{
    return getPC();
}

NPCharacter * Character::getMobile( )
{
    return getNPC();
}

const GlobalBitvector & Character::getWearloc( )
{
    return wearloc;
}

/*****************************************************************************
 * name and sex formatting
 *****************************************************************************/
DLString Character::sees( const Character *ch, char needcase ) const  
{
    return ch->toNoun( this, FMT_INVIS )->decline( needcase );
}
DLString Character::seeName( const Character *ch, char needcase ) const
{
    return ch->toNoun( this, FMT_PRETITLE )->decline( needcase );
}

/****************************************************************************
 * visibility of things 
 ****************************************************************************/
static bool mprog_invisible( Character *ch, const Character *looker )
{
    FENIA_CALL( ch, "Invisible", "C", looker );
    FENIA_NDX_CALL( ch->getNPC( ), "Invisible", "CC", ch, looker );
    return false;
}

/*
 * True if char can see victim.
 */
bool Character::can_see( const Character *victim ) const
{
    // RT changed so that WIZ_INVIS has levels
    if ( this == victim )
        return true;
    
    if (!can_sense( victim ))
        return false;
    
    if (mprog_invisible( const_cast<Character *>( victim ), this ))
        return false;

    if (!victim->is_npc( )) {
        if ( get_trust() < victim->getPC( )->invis_level )
            return false;

        if (IS_GHOST( victim ))
        {
            if ( is_immortal()
                    || ( CAN_DETECT( this, DETECT_INVIS )
                            && !IS_AFFECTED(this, AFF_BLIND) ) )
                    return true;

            if (IS_GHOST(this) ||IS_DEATH_TIME(this))
                return true;

            return false;
        }

        if ( get_trust() < victim->getPC( )->incog_level && in_room != victim->in_room )
            return false;
    } 

    if ( !is_npc() && ( IS_GHOST( this ) || IS_DEATH_TIME( this ) ) )
        return true;

    if ( (!is_npc() && IS_SET(act, PLR_HOLYLIGHT)) || (is_npc() && is_immortal()))
        return true;

    if (IS_AFFECTED(this, AFF_BLIND) )
        return false;

    if (position <= POS_SLEEPING)
        return false;

    if ( !in_room )
        return false;

    if (in_room->isDark( ) && !IS_AFFECTED(this, AFF_INFRARED) )
        return false;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) && !CAN_DETECT(this, DETECT_INVIS) )
        return false;

    if ( IS_AFFECTED(victim, AFF_IMP_INVIS) && !CAN_DETECT(this, DETECT_IMP_INVIS) )
        return false;

    if ( IS_AFFECTED(victim,AFF_CAMOUFLAGE) && !CAN_DETECT(this,ACUTE_VISION))
        return false;

    if ( IS_AFFECTED(victim, AFF_HIDE) 
        && !CAN_DETECT(this, DETECT_HIDDEN)
        && victim->fighting == NULL)
        return false;

    if ( IS_AFFECTED(victim, AFF_FADE)
        && !CAN_DETECT(this, DETECT_FADE)
        && victim->fighting == NULL)
        return false;

    return true;
}


static bool oprog_invisible( Object *obj, const Character *ch ) 
{
    FENIA_CALL( obj, "Invisible", "C", ch )
    FENIA_NDX_CALL( obj, "Invisible", "OC", obj, ch )

    if (obj->behavior)
        return !obj->behavior->visible( ch );

    return false;
}


/*
 * True if char can see obj.
 */
bool Character::can_see( const Object *obj ) const
{
    if (oprog_invisible( const_cast<Object *>( obj ), this ))
        return false;
        
  if( !is_npc() && IS_GHOST( this ) ) {
    return true;
  }
    
    if ( !is_npc() && ( IS_SET(act, PLR_HOLYLIGHT) ||
                          IS_GHOST( this ) ||
                          IS_DEATH_TIME( this ) ) )
        return true;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
        return false;

    if ( IS_AFFECTED( this, AFF_BLIND ) )
        return false;

    if ( obj->item_type == ITEM_LIGHT && obj->value2() != 0 )
        return true;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !CAN_DETECT(this, DETECT_INVIS) )
        return false;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
        return true;

    if (in_room->isDark( ) && !IS_AFFECTED(this, AFF_INFRARED) )
        return false;

    if ( obj->item_type == ITEM_TATTOO )
        return true;

    return true;
}

/*
 * True if char can HEAR humming obj.
 */
bool Character::can_hear( const Object *obj ) const
{
    // not sure why this code is here
    /*
    if (oprog_invisible( const_cast<Object *>( obj ), this ))
        return false; */
        
    if ( !IS_SET(obj->extra_flags, ITEM_HUM)
            || isAffected(gsn_deafen) )
    {
            return false;
    }

    return true;
}

bool Character::can_sense( const Character *victim ) const
{
    if (victim == this)
        return true;

    if (IS_SET(in_room->room_flags, ROOM_DUMB)
        && !is_npc( )
        && !victim->is_npc( )
        && !(victim->is_immortal( ) || is_immortal( )))
        return false;
    
    if (DIGGED(this) && DIGGED(victim))
        return false;

    return true;
}


/****************************************************************************
 * object manipulations 
 ****************************************************************************/
short Character::getWearLevel( Object *obj ) 
{
    int wear_mod;
    
    wear_mod = getProfession( )->getWearModifier( obj->item_type );
            
    return std::max( 1, obj->level - wear_mod - (getModifyLevel( ) - getRealLevel( )));
}




/****************************************************************************
 * skills 
 ****************************************************************************/
int Character::getSkill( int sn )
{
    return skillManager->find( sn )->getEffective( this );
} 

/****************************************************************************
 * misc 
 ****************************************************************************/
const Character * Character::getDoppel( const Character *looker ) const
{
    if (looker && looker->getConfig( ).holy)
        return this;
    else if (doppel && isAffected(gsn_doppelganger))
        return doppel->getDoppel( looker );
    else
        return this;
}

Character * Character::getDoppel( const Character *looker )
{
    if (looker && looker->getConfig( ).holy)
        return this;
    else if (doppel && isAffected(gsn_doppelganger))
        return doppel->getDoppel( looker );
    else
        return this;
}

bool Character::isAffected( int sn ) const
{
    return affected.find(sn) != 0;
}

void Character::dismount( )
{
    if (mount) {
        mount->mount = NULL;
        mount = NULL;
    }
}

