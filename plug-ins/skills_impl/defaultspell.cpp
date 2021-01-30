/* $Id: defaultspell.cpp,v 1.1.2.13.6.18 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */
#include <math.h>

#include "defaultspell.h"
#include "skillreference.h"
#include "spelltarget.h"
#include "spellmanager.h"
#include "skillgroup.h"
#include "skill_utils.h"
#include "feniaspellhelper.h"

#include "fenia/exceptions.h"
#include "affect.h"
#include "pcharacter.h"
#include "object.h"
#include "room.h"
#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "stats_apply.h"
#include "act_move.h"
#include "profflags.h"
#include "commandflags.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

GSN(spellbane);
GSN(garble);
GSN(spell_craft);
GSN(improved_maladiction);
GSN(improved_benediction);
GSN(holy_remedy);
GSN(mastering_spell);
GSN(magic_concentrate);

PROF(cleric);
PROF(paladin);

GROUP(maladictions);
GROUP(benedictions);
GROUP(curative);
GROUP(healing);
GROUP(clan);
GROUP(combat);
GROUP(none);

DefaultSpell::DefaultSpell( ) 
        : Spell(),
          target( TAR_IGNORE, &target_table ), 
          position( POS_STANDING, &position_table ), 
          type( SPELL_NONE, &spell_types ),
          casted( true ),
          ranged(true),
          order(0, &order_flags),
          damtype(0, &damage_table),
          damflags(0, &damage_flags)
{
}

long long DefaultSpell::getID() const
{
    int myId = 0;

    if (getSkill()->getSkillHelp())
        myId = getSkill()->getSkillHelp()->getID();

    if (myId <= 0)
        throw Scripting::Exception(getSkill()->getName() + ": spell ID not found or zero");

    return (myId << 4) | 5;

}

void DefaultSpell::setSkill( SkillPointer skill )
{
    this->skill = skill;
    SpellManager::registrate( Pointer( this ) );
}

void DefaultSpell::unsetSkill( )
{
    SpellManager::unregistrate( Pointer( this ) );
    skill.clear( );
}

void DefaultSpell::run( Character *ch, SpellTarget::Pointer spt, int level )
{
    char arg[MAX_STRING_LENGTH];                                            
    int sn;
    
    if (!spt)
        return;

    if (FeniaSpellHelper::executeSpell(this, ch, spt, level))
        return;
        
    sn = skill->getIndex( );

    switch (spt->type) {
    case SpellTarget::NONE:
        strcpy( arg, spt->arg );
        run( ch, arg, sn, level );
        break;
    case SpellTarget::CHAR:
        run( ch, spt->victim, sn, level );
        break;
    case SpellTarget::OBJECT:
        run( ch, spt->obj, sn, level );
        break;
    case SpellTarget::ROOM:
        run( ch, spt->room, sn, level );
        break;
    default:
        break;
    }
}

int DefaultSpell::getManaCost( Character *ch )
{
    if (!skill->available( ch ))
        return 50;

    return max( getMana( ),
                100 / (2 + ch->getRealLevel( ) - skill->getLevel( ch )) );
}

/*
 * how far can we cast
 * returns number of rooms
 */
int DefaultSpell::getMaxRange( Character *ch ) const
{
    if (!ranged)
        return 0;

    if (type == SPELL_NONE || type == SPELL_DEFENSIVE)
        return 0;

    if (position.getValue( ) == POS_STANDING)
        return 0;

    int level = skill->getLevel( ch ); 
        
    return max(1,level / 10);
}

/*
 * Find a char for spell usage. Allowed syntax: vict, dir.vict, dir vict
 */
Character * 
DefaultSpell::getCharSpell( Character *ch, const DLString &argument, int *door, int *range, ostringstream &errbuf )
{
    DLString argDoor, argVict;

    if (direction_range_argument(argument, argDoor, argVict, *door)) {
        return find_char(ch, argVict.c_str(), *door, range, errbuf);
    }
    
    return get_char_room(ch, argVict.c_str());
}

/*
 * Utter mystical words for a spell.
 */
void DefaultSpell::utter( Character *ch )
{
    Character *rch;
    DLString utterance = spell_utterance(*skill);
    const char *pat = "$c1 бормочет '$t'.";

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (rch != ch) {
            int chance = (gsn_spell_craft->getEffective( rch ) * 9) / 10;
            
            if (chance < number_percent( ))
                act( pat, ch, utterance.c_str(), rch, TO_VICT );
            else
                act( pat, ch, skill->getNameFor( rch ).c_str( ), rch, TO_VICT );
        }
    }
}

/*
 * apply spell level modifiers
 */
int 
DefaultSpell::getSpellLevel( Character *ch, int range ) 
{
    int slevel;
    int chance;
    int mlevel = ch->getModifyLevel( );
    
    if (ch->is_npc( ))
        return mlevel;
    
    if (ch->getProfession( )->getFlags( ).isSet(PROF_CASTER))
        slevel = mlevel - max(0, mlevel / 20);
    else
        slevel = mlevel - max(5, mlevel / 10);
    
    if (gsn_spell_craft->usable( ch )) {
        if (number_percent() < gsn_spell_craft->getEffective( ch )) {
            slevel = mlevel;
            gsn_spell_craft->improve( ch, true );
        }
        else
            gsn_spell_craft->improve( ch, false );
    }
    
    if (skill->hasGroup(group_maladictions)
        && (chance = gsn_improved_maladiction->getEffective( ch )))
    {
        if (number_percent( ) < chance) {
            slevel = mlevel;
            slevel += chance / 20;
            gsn_improved_maladiction->improve( ch, true );
        }
        else
            gsn_improved_maladiction->improve( ch, false );
    }

    if (skill->hasGroup(group_benedictions)
        &&  (chance = gsn_improved_benediction->getEffective( ch ))) 
    {
        if (number_percent() < chance) {
            slevel = mlevel;
            slevel += chance / 20;
            gsn_improved_benediction->improve( ch, true );
        }
        else
            gsn_improved_benediction->improve( ch, false );
    }

    if (skill->hasGroup(group_healing) 
        || skill->hasGroup(group_curative))
    {
        chance = gsn_holy_remedy->getEffective( ch );

        if (number_percent( ) < chance) {
            slevel = mlevel;
            slevel += chance / 20;
            act( "Свет на мгновение пронизывает твои ладони.", ch, 0, 0, TO_CHAR );
            act( "Свет на мгновение пронизывает ладони $c2.", ch, 0, 0, TO_ROOM );
            gsn_holy_remedy->improve( ch, true );
        }
    }
    
    /*
     * Magic Concentrate, by Kind
     *   f(x) = A0 / (1 + x / B0)
     *     A0 = a / (B0 * Ln(1 + a / B0))
     *      A0 - усиление на нулевом расстоянии - в стее
     *      x  - расстояние до цели
     *      B0 - свободный параметр
     *      a  - дальность действия спелла
     */
    if (skill->hasGroup(group_combat)
        && range >= 0
        && ch->isAffected( gsn_magic_concentrate ))
    {
        int a, x;
        double A0, B0, f;

        a  = max( 1, getMaxRange( ch ) );
        x  = range;
        B0 = 12;
        A0 = a / (B0 * log( 1 + a / B0 ));
        f  = A0 / (1 + x / B0);

        slevel = (int) (f * slevel);
    }

    if (gsn_mastering_spell->usable( ch, false )) {
        if (number_percent() < gsn_mastering_spell->getEffective( ch )) {
            slevel += ( number_range( 1, 4 ) );
            gsn_mastering_spell->improve( ch, true );
        }
        else
            gsn_mastering_spell->improve( ch, false );
    }

    slevel = max( 1, slevel + get_int_app(ch).slevel );

    slevel += skill_level_bonus(**skill, ch);

    return slevel;
}

/*
 * Parse spell targets.
 * Valid combinations are:
 *
 * one of TAR_IGNORE, TAR_CREATE_OBJ, TAR_CREATE_MOB, TAR_ROOM, TAR_PEOPLE
 * TAR_CHAR_WORLD and TAR_OBJ_*
 * TAR_CHAR_SELF and TAR_OBJ_*
 * TAR_CHAR_ROOM and TAR_OBJ_*
 * TAR_OBJ_*
 * TAR_ROOM|TAR_PEOPLE and TAR_CHAR_SELF
 */

static bool is_self_name( const DLString &arg, Character *ch )
{
    if (arg.empty( ))
        return false;
    
    if (arg == "self" || arg == "я")
        return true;

    if (is_name( arg.c_str( ), ch->getNameP( '7' ).c_str( ) ))
        return true;

    if (get_char_room( ch, arg.c_str( ) ) == ch)
        return true;

    return false;
}

SpellTarget::Pointer 
DefaultSpell::locateTargets( Character *ch, const DLString &arg, std::ostringstream &buf )
{
    char carg[MAX_STRING_LENGTH];
    Character *victim;
    SpellTarget::Pointer result( NEW );

    strcpy( carg, arg.c_str( ) );
    
    if (target.isSet( TAR_IGNORE|TAR_CREATE_OBJ )) {
        result->type = SpellTarget::NONE;
        result->arg = arg.c_str( );
        return result;
    }

    if (target.isSet( TAR_CREATE_MOB )) {
        if (!arg.empty( )) {
            if (!( victim = get_char_room( ch, arg.c_str( ) ) )) {
                buf << "Кого именно ты хочешь призвать?" << endl;
                result->error = TARGET_ERR_SUMMON_WHO; 
                return result;
            }

            result->type = SpellTarget::CHAR;
            result->victim = victim;
        }
        else {
            result->type = SpellTarget::NONE;
            result->arg = arg.c_str( );
        }

        return result;
    }
    
    if (target.isSet( TAR_ROOM|TAR_PEOPLE )) {
        if (arg.empty( )) {
            result->type = SpellTarget::ROOM;
            result->room = ch->in_room;
            return result;
        }
        
        if (target.isSet( TAR_CHAR_SELF )) {
            if (!is_self_name( arg, ch )) {
                buf << "Ты не можешь использовать это заклинание на других." << endl;
                result->error = TARGET_ERR_NOT_ON_OTHERS;
                return result;
            }

            result->type = SpellTarget::CHAR;
            result->victim = ch;
            return result;
        }

        // Ignore specified target for room spells, to allow for scrolls with multiple target types.
        result->type = SpellTarget::ROOM;
        result->room = ch->in_room;
        return result;
    }

    if (target.isSet( TAR_CHAR_WORLD )) {
        if (arg.empty( )) {
            buf << "Колдовать на кого?" << endl;
            result->error = TARGET_ERR_CAST_ON_WHOM;
            return result;
        }

        victim = get_char_world_doppel( ch, carg );

        if (victim) {
            result->type = SpellTarget::CHAR;
            result->victim = victim;
            return result;
        }
        
        SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
        if (objresult)
            return objresult;
        
        buf.str( "" );
        buf << "Ты не находишь никого с таким именем." << endl;
        result->error = TARGET_ERR_CHAR_NOT_FOUND;
        return result;
    }
    
    if (target.isSet( TAR_CHAR_SELF )) {
        if (!arg.empty( ) && !is_self_name( arg, ch )) {
            SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
            if (objresult)
                return objresult;

            buf.str( "" );
            buf << "Ты не можешь использовать это заклинание на других." << endl;
            result->error = TARGET_ERR_NOT_ON_OTHERS;
            return result;
        }

        result->type = SpellTarget::CHAR;
        result->victim = ch;
        return result;
    }

    if (target.isSet( TAR_CHAR_ROOM )) {
        victim = NULL;
        result->range = 0;

        if (arg.empty( )) {
            if (getSpellType( ) == SPELL_OFFENSIVE) 
                victim = ch->fighting;
            else {
                victim = ch;
                result->range = -1;
            }
        }
        else {
            int maxrange = getMaxRange( ch );
            
            if (maxrange > 0) {
                victim = getCharSpell( ch, arg, &result->door, &maxrange, buf );

                if (victim) {
                    if (ch->isAffected(gsn_garble ) && number_percent( ) < 10)
                        victim = ch;
                        
                    if (victim->is_npc() && IS_SET(victim->act,ACT_NOTRACK )
                        && victim->in_room != ch->in_room )
                    {
                        buf << "Твое заклинание не действует на "
                            << victim->getNameP( '4' ) << " на таком расстоянии." << endl;
                        result->error = TARGET_ERR_TOO_FAR;
                        return result;
                    }
                    
                    result->range = std::max( 0, getMaxRange( ch ) - maxrange );
                    result->castFar = true;

                } else if (result->door >= 0 && result->door < DIR_SOMEWHERE) {
                    // Victim not found, but a range spell was likely casted.
                    result->error = TARGET_ERR_CHAR_NOT_FOUND;
                    return result;
                }
            }
            else 
                victim = get_char_room( ch, carg );
        }

        if (victim) {
            result->type = SpellTarget::CHAR;
            result->victim = victim;
            return result;
        }
        
        result->range = -1;

        SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
        if (objresult)
            return objresult;
            
        buf.str( "" );
        buf << "Произнести заклинание... на кого?" << endl;
        result->error = TARGET_ERR_CAST_ON_WHOM;
        return result;
    }

    SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
    if (objresult)
        return objresult;
    
    if (buf.str( ).empty( ))
        buf << "Произнести заклинание... на кого?" << endl;

    result->error = TARGET_ERR_CAST_ON_WHOM;
    return result;
}

SpellTarget::Pointer
DefaultSpell::locateTargetObject( Character *ch, const DLString &arg, std::ostringstream &buf )
{
    char carg[MAX_STRING_LENGTH];
    Object *obj;
    SpellTarget::Pointer null;
    SpellTarget::Pointer result( NEW );
    
    strcpy( carg, arg.c_str( ) );
    
    if (ch->is_npc( ))
        return null;
    
    if (target.isSet( TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM|TAR_OBJ_WORLD )) {
        obj = NULL;

        if (arg.empty( )) {
            buf << "Произнести заклинание на что?" << endl;
            result->error = TARGET_ERR_CAST_ON_WHAT;
            return result;
        }
        
        if (target.isSet( TAR_OBJ_EQUIP ) && target.isSet( TAR_OBJ_INV ))
            obj = get_obj_wear_carry( ch, carg );
        if (!obj && target.isSet( TAR_OBJ_EQUIP ))
            obj = get_obj_wear( ch, carg );
        if (!obj && target.isSet( TAR_OBJ_INV )) 
            obj = get_obj_carry( ch, carg );
        if (!obj && target.isSet( TAR_OBJ_ROOM )) 
            obj = get_obj_room( ch, carg );
        if (!obj && target.isSet( TAR_OBJ_WORLD ))
            obj = get_obj_world( ch, carg );
        
        if (obj) {
            result->type = SpellTarget::OBJECT;
            result->obj = obj;
            return result;
        }

        if (target.isSet( TAR_OBJ_INV|TAR_OBJ_EQUIP ))
            buf << "У тебя нет этого." << endl;
        else if (target.isSet( TAR_OBJ_ROOM ))
            buf << "Ты не видишь здесь такого предмета." << endl;
        else if (target.isSet( TAR_OBJ_WORLD ))
            buf << "Ты не можешь обнаружить ничего с таким именем." << endl;

        result->error = TARGET_ERR_OBJ_NOT_FOUND;
        return result;
    }

    return null;
}

bool DefaultSpell::checkPosition( Character *ch ) const
{
    if (ch->position < position.getValue( )) {
        ch->println("Ты не можешь использовать это заклинание, когда сражаешься.");
        return false;
    }
    
    return true;
}

/**
 * See if this spell can be casted when ordered:
 * - charmed mobs cannot cast offensive stuff or spells marked with 'player_only'
 * - charmed players can cast everything
 */
bool DefaultSpell::properOrder(Character *ch) const
{
    if (!IS_CHARMED(ch))
        return true;

    if (!ch->is_npc())
        return true;

    if (order.isSet(ORDER_PLAYER_ONLY))
        return false;    

    if (getSpellType() == SPELL_OFFENSIVE)
        return false;

    return true;
}

bool DefaultSpell::isPrayer( Character *caster ) const
{
    if (!isCasted( ))
        return false;

    if (getSkill( )->hasGroup(group_clan))
        return false;

    if (getSkill( )->hasGroup(group_none))
        return false;
    
    return caster->getProfession( )->getFlags( caster ).isSet(PROF_DIVINE);
}

/*
 * spellbane
 * - prayers:
 *        + no reaction if casted nearby or ranged
 *        + block everything defensive targeted on you w/o damage
 *      + pass everyting offensive w/o messages
 *
 * - magical spells:
 *        + deflect everything defensive on you, with msg and small damage on pk
 *        + slightly damage pk if anything is casted nearby but not on you 
 *          or your foe, or your clanmate
 *        + throw dice to deflect offensive on you, results: 
 *          pass all, partial, partial with spellbane, none with spellbane
 *        + partially block (with serious damage on pk) all offensive directed to 
 *          the victim you or your group is fighting with, near and ranged
 *
 *  - pets of btr block all defensive casts
 */
void DefaultSpell::baneMessage( Character *ch, Character *vch ) const
{
    if (isPrayer( ch )) {
        act("Твои боги не благосклонны к $C3.", ch, 0, vch, TO_CHAR);
        act("Боги $c2 не благосклонны к тебе.", ch, 0, vch, TO_VICT);
        act("Боги $c2 не благосклонны к $C3.", ch, 0, vch, TO_NOTVICT);
    }
    else if (ch != vch) {
        act("$C1 отклоняет твое заклинание!", ch, 0, vch, TO_CHAR);
        act("Ты отклоняешь заклинание $c2!", ch, 0, vch, TO_VICT);
        act("$C1 отклоняет заклинание $c2!", ch, 0, vch, TO_NOTVICT);
    }
    else {
        act("Ты отклоняешь заклинание!", ch, 0, vch, TO_VICT);
        act("$C1 отклоняет заклинание!", ch, 0, vch, TO_NOTVICT);
    }
}

void DefaultSpell::baneDamage( Character *ch, Character *vch, int dam ) const
{
    if (is_safe_nomessage( vch, ch ))
        return;
        
    if (is_safe_nomessage( ch, vch )) 
        return;
    
    gsn_spellbane->improve( vch, true, ch );
    damage_nocatch( vch, ch, ( dam ), gsn_spellbane, DAM_NEGATIVE, true, DAMF_SPELL );
}

void DefaultSpell::baneAround( Character *ch, int failChance, int dam ) const
{
    if (ch->is_npc( ) && !IS_CHARMED(ch))
        return;

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        baneAction( ch, rch, failChance, dam );
    }
}

void DefaultSpell::baneForAssist( Character *ch, Character *vch ) const
{
    Character *fch = vch->fighting;

    for (Character *rch = vch->in_room->people; rch; rch = rch->next_in_room) {
        if (rch == vch || rch == ch)
            continue;

        if ((fch && is_same_group( fch, rch )) || rch->fighting == vch)
            baneAction( ch, rch, 100, 3 * rch->getModifyLevel( ) );
    }
}

bool DefaultSpell::baneAction( Character *ch, Character *bch, int failChance, int dam ) const
{
    if (!bch->isAffected( gsn_spellbane ))
        return false;

    if (is_safe_nomessage( bch, ch )) 
        return false;

    if ((failChance * number_percent( ) / 100) > (2 * bch->getSkill( gsn_spellbane ) / 3))
        return false;
        
    baneMessage( ch, bch );
    baneDamage( ch, bch, dam );
    return true;
}

bool DefaultSpell::spellbane( Character *ch, Character *vch ) const
{
    int mlevel = vch ? vch->getModifyLevel( ) : ch->getModifyLevel( );
    bool offensive = (type == SPELL_OFFENSIVE);
    bool defensive = (type == SPELL_DEFENSIVE);
    bool bannedVictim = (vch && vch->isAffected( gsn_spellbane ));
    
    if (vch && vch->is_npc( ) && vch->master && vch->master->isAffected( gsn_spellbane )) {
        if (!offensive) {
            baneMessage( ch, vch );
            return true;
        }
    }

    if (isPrayer( ch )) {
        if (!bannedVictim || offensive)
            return false;
        
        baneMessage( ch, vch );
        return true;
    }
    
    try {
        if (bannedVictim && defensive) {
            baneMessage( ch, vch );
            baneDamage( ch, vch, mlevel );
            return true;
        }

        if (bannedVictim && !defensive) {
            int chance = gsn_spellbane->getEffective( vch ); 
            int damage = offensive ? 3 * mlevel : mlevel;

            if (!vch->is_npc( )) {
                chance = 2 * chance / 3;
            }

            if (number_percent( ) > chance) { 
                gsn_spellbane->improve( vch, false, ch );
                return false;
            }
            
            baneMessage( ch, vch );
            baneDamage( ch, vch, damage );
            return true;
        }
        
        if (!offensive) {
            baneAround( ch, 0, mlevel / 2 );
            return false;
        }

        if (!vch) {
            baneAround( ch, 100, mlevel );
            return false;
        }
        
        baneForAssist( ch, vch );
        return false;

    } catch (const VictimDeathException &) {
        return true;
    }
}

int DefaultSpell::getBeats( ) const
{
    return skill->getBeats( ); 
}
int DefaultSpell::getMana( ) const
{
    return skill->getMana( ); 
}
int DefaultSpell::getTarget( ) const
{
    return target.getValue( );
}
int DefaultSpell::getSpellType( ) const
{
    return type.getValue( );
}
int DefaultSpell::getPosition() const
{
    return position;
}

SkillPointer DefaultSpell::getSkill( ) const
{
    return skill;
}
bool DefaultSpell::isCasted( ) const
{
    return casted.getValue( );
}

AnatoliaCombatSpell::AnatoliaCombatSpell()
                : 
                  savesCheck(true)
{

}

void AnatoliaCombatSpell::run( Character *ch, Character *victim, int sn, int level )
{
    // Calculate spell flags.
    bitstring_t flags = DAMF_SPELL | damflags;

    // Calculate damage.
    int dam = ::dice(level, dice) + diceBonus;
    
    // Do a saves check if required.
    if (savesCheck && saves_spell(level, victim, damtype, ch, flags))
        dam /= 2;

    // Display messages.
    if (!msgNotVict.empty())
        act(msgNotVict.c_str(), ch, 0, victim, TO_NOTVICT);

    if (!msgChar.empty())
        act(msgChar.c_str(), ch, 0, victim, TO_CHAR);

    if (!msgVict.empty())
        act(msgVict.c_str(), ch, 0, victim, TO_VICT);

    // Apply waitstate to the victim.
    if (waitMax != 0)
        victim->setWait(number_range(waitMin, waitMax));

    // Inflict the damage.
    damage_nocatch(ch, victim, dam, sn, damtype, true, flags);
}
