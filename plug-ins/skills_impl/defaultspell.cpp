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
#include "feniaskillaction.h"
#include "religion.h"

#include "fenia/exceptions.h"
#include "affect.h"
#include "pcharacter.h"
#include "object.h"
#include "room.h"
#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "stats_apply.h"
#include "commonattributes.h"
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
GROUP(combat);

RELIG(none);

CLAN(shalafi);


DefaultSpell::DefaultSpell( ) 
        : Spell(),
          target( TAR_IGNORE, &target_table ), 
          position( POS_STANDING, &position_table ), 
          type( SPELL_NONE, &spell_types ),
          casted( true ),
          ranged(true),
          flags(0, &spell_flags),
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

    if (FeniaSkillActionHelper::executeSpellRun(this, ch, spt, level))
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

void DefaultSpell::apply( Character *ch, SpellTargetPointer spt, int level )
{
    char arg[MAX_STRING_LENGTH];                                            
    
    if (!spt)
        return;

    if (FeniaSkillActionHelper::executeSpellApply(this, ch, spt, level))
        return;
        
    switch (spt->type) {
    case SpellTarget::NONE:
        strcpy( arg, spt->arg );
        apply( ch, arg, level );
        break;
    case SpellTarget::CHAR:
        apply( ch, spt->victim, level );
        break;
    case SpellTarget::OBJECT:
        apply( ch, spt->obj, level );
        break;
    case SpellTarget::ROOM:
        apply( ch, spt->room, level );
        break;
    default:
        break;
    }
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

    if (!target.isSet(TAR_CHAR_ROOM))
        return 0;

    int level = ch ? skill->getLevel( ch ) : 1; 
        
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
    if (isPrayer(ch))
        utterPrayer(ch);
    else
        utterMagicSpell(ch);
}

void DefaultSpell::utterPrayer(Character *ch)
{
    if (ch->is_npc()) {
        ch->recho("%^C1 молится своим богам.", ch);
    } else if (ch->getReligion() == god_none) {
        ch->pecho("Ты неумело молишься, прося богов о помощи.");  
        ch->recho("%^C1 неумело молится, прося богов о помощи.", ch); 
    } else {
        ch->pecho("Ты возносишь молитву %N3.", ch->getReligion()->getRussianName().c_str());
        ch->recho("%^C1 возносит молитву %N3.", ch, ch->getReligion()->getRussianName().c_str());
    }
}

void DefaultSpell::utterMagicSpell(Character *ch)
{
    Character *rch;
    DLString utterance = spell_utterance(*skill);
    const char *pat = "$c1 бормочет '$t'.";

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (rch != ch) {
            int chance = (gsn_spell_craft->getEffective( rch ) * 9) / 10;
            
            if (chance < number_percent( ))
                oldact( pat, ch, utterance.c_str(), rch, TO_VICT );
            else
                oldact( pat, ch, skill->getNameFor( rch ).c_str( ), rch, TO_VICT );
        }
    }
}

static Religion * get_random_god(Character *ch)
{
    int cnt = 0;
    Religion *result = 0;

    for (int r = 0; r < religionManager->size(); r++) {
        Religion *rel = religionManager->find(r);
        if (rel->available(ch) && number_range(0, cnt++) == 0)
            result = rel;
    }

    return result;
}

/**
 * Apply spell level penalties for prayers, return 'false' to prevent casting.
 */
bool DefaultSpell::canPray(Character *ch, int &slevel)
{
    if (ch->is_npc())
        return true;

    if (!skill->getSpell() || !skill->getSpell()->isCasted())
        return true;

    if (!skill->getSpell()->isPrayer(ch))
        return true;

    if (ch->getReligion() != god_none)
        return true;

    // Divine non-believers get a spell level penalty.
    int mlevel = ch->getModifyLevel();
    if (mlevel > 1) {
        int penalty = 100 - mlevel * 2;
        slevel = slevel * penalty / 100;
    }

    if (slevel <= 0) {
        ch->pecho("Никто из богов больше не откликнется на твои молитвы, пока ты не изберешь себе {hh1религию{x.");
        return false;
    }

    // Choose a random deity to fulfil the prayer and remember their name.
    XMLStringAttribute::Pointer randomGodAttr = ch->getPC()->getAttributes().getAttr<XMLStringAttribute>("randomGod");
    randomGodAttr->clear();

    Religion *randomGod = get_random_god(ch);
    if (randomGod) {
        ch->pecho("Твою просьбу исполняет %N1 и советует поскорее выбрать себе {hh1религию{x.", randomGod->getRussianName().c_str());
        randomGodAttr->setValue(randomGod->getName());
    }

    return true;
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
    bool fPrayer = isPrayer(ch);
    
    if (ch->is_npc( ))
        return mlevel;

    if (ch->getProfession( )->getFlags( ).isSet(PROF_CASTER))
        slevel = mlevel - max(0, mlevel / 20); // 0-5 levels penalty
    else if (ch->getProfession( )->getFlags( ).isSet(PROF_HYBRID))
        slevel = mlevel - max(2, mlevel / 16); // 2-6 levels penalty  
    else if (ch->getProfession( )->getFlags( ).isSet(PROF_AGILE))
        slevel = mlevel - max(3, mlevel / 13); // 3-8 levels penalty
    else    
        slevel = mlevel - max(5, mlevel / 10); // 5-10 levels penalty

    // Too old to be a cleric-atheist.
    if (!canPray(ch, slevel))
        return -1;

    // shalafi get half penalty
    if (ch->getClan( ) == clan_shalafi)
        slevel = max(slevel, (slevel + mlevel)/2);
        
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
            oldact("Твои глаза на мгновение вспыхивают {1{rбагровым{2.", ch, 0, 0, TO_CHAR );
            oldact("Глаза $c2 на мгновение вспыхивают {1{rбагровым{2.", ch, 0, 0, TO_ROOM );                
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
            oldact("Твои глаза на мгновение вспыхивают {1{Yзолотом{2.", ch, 0, 0, TO_CHAR );
            oldact("Глаза $c2 на мгновение вспыхивают {1{Yзолотом{2.", ch, 0, 0, TO_ROOM );                 
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
            oldact("Свет на мгновение пронизывает твои ладони.", ch, 0, 0, TO_CHAR );
            oldact("Свет на мгновение пронизывает ладони $c2.", ch, 0, 0, TO_ROOM );
            gsn_holy_remedy->improve( ch, true );
        }
    }
    
    /*
     * Magic Concentrate, by Kind, updated by Taiphoen.
     * Applicable to ranged, offensive, casted, non-prayer spells.
     *   f(x) = A0 / (1 + x / B0)
     *     A0 = a / (B0 * Ln(1 + a / B0))
     *      A0 - усиление на нулевом расстоянии - в стее
     *      x  - расстояние до цели
     *      B0 - свободный параметр
     *      a  - дальность действия спелла
     */
    int maxRange = getMaxRange(ch);               
    if ( ch->isAffected(gsn_magic_concentrate) &&
         !fPrayer &&
         flags.isSet(SPELL_MAGIC) &&
         maxRange > 0 )
    {
        int a, x;
        double A0, B0, f;

        a  = max( 1, maxRange );
        x  = range;
        B0 = 12;
        A0 = a / (B0 * log( 1 + a / B0 ));
        f  = A0 / (1 + x / B0);

        slevel = (int) (f * slevel);
        oldact("Яркая искра вспыхивает между твоих ладоней, фокусируя магический заряд.", ch, 0, 0, TO_CHAR );
        oldact("Яркая искра вспыхивает между ладоней $c2, фокусируя магический заряд.", ch, 0, 0, TO_ROOM );            
    }

    if (gsn_mastering_spell->usable( ch, false )) {
        if (number_percent() < gsn_mastering_spell->getEffective( ch )) {
            slevel += ( number_range( 1, 4 ) );
            gsn_mastering_spell->improve( ch, true );
        }
        else
            gsn_mastering_spell->improve( ch, false );
    }
    
    // shalafi atheist bonus
    if ( ch->getReligion() == god_none && ch->getClan( ) == clan_shalafi )
        slevel += 2;
        
    if (fPrayer)
        slevel = max( 1, slevel + get_wis_app(ch).slevel );
    else
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
    
    if (target.isSet( TAR_ROOM|TAR_PEOPLE )) {
        if (arg.empty( )) {
            result->type = SpellTarget::ROOM;
            result->room = ch->in_room;
            return result;
        }
        
        if (target.isSet( TAR_CHAR_SELF )) {
            if (!is_self_name( arg, ch )) {
                buf << "Это заклинание нельзя использовать на других.";
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
            buf << "Для этого заклинания нужно указать имя персонажа в качестве цели.";
            result->error = TARGET_ERR_CAST_ON_WHOM;
            return result;
        }

        victim = get_char_world( ch, carg, FFIND_DOPPEL );

        if (victim) {
            result->type = SpellTarget::CHAR;
            result->victim = victim;
            return result;
        }
        
        SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
        if (objresult)
            return objresult;
        
        buf.str( "" );
        buf << "Увы, никого с таким именем обнаружить не удается.";
        result->error = TARGET_ERR_CHAR_NOT_FOUND;
        return result;
    }
    
    if (target.isSet( TAR_CHAR_SELF )) {
        if (!arg.empty( ) && !is_self_name( arg, ch )) {
            SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
            if (objresult)
                return objresult;

            buf.str( "" );
            buf << "Это заклинание нельзя использовать на других.";
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

        // Empty argument: defaults to your victim in battle for offensive spells, or to yourself for defensive spells.
        if (arg.empty( )) {
            if (getSpellType( ) == SPELL_OFFENSIVE) 
                victim = ch->fighting;
            else {
                victim = ch;
                result->range = -1;
            }

            if (!victim) {
                // Victim still not found, e.g. offensive spell cast outside of battle.
                buf.str( "" );
                buf << "Нужно указать цель для заклинания.";
                result->error = TARGET_ERR_CAST_ON_WHOM;
                return result;
            }

        }
        // Non-empty argument: locate victim.
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
                        buf << "Это заклинание не подействует на "
                            << victim->getNameP( '4' ) << " на таком расстоянии.";
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
        buf << "Увы, никого с таким именем в этой местности обнаружить не удается.";
        result->error = TARGET_ERR_CAST_ON_WHOM;
        return result;
    }

    SpellTarget::Pointer objresult = locateTargetObject( ch, arg, buf );
    if (objresult && objresult->obj)
        return objresult;

    if (target.isSet( TAR_IGNORE|TAR_CREATE_OBJ )) {
        result->type = SpellTarget::NONE;
        result->arg = arg.c_str( );
        return result;
    }

    if (target.isSet( TAR_CREATE_MOB )) {
        if (!arg.empty( )) {
            if (!( victim = get_char_room( ch, arg.c_str( ) ) )) {
                buf.str("");
                buf << "Увы, никого с таким именем перепризвать не удается.";
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
    
    if (buf.str( ).empty( ))
        buf << "Для этого заклинания нужно указать цель.";

    result->error = TARGET_ERR_CAST_ON_WHOM;

    if (objresult && objresult->error)
        result->error = objresult->error;

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
    
    if (target.isSet( TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM|TAR_OBJ_WORLD )) {
        obj = NULL;

        if (ch->is_npc( )) {
            buf << "Ты не можешь использовать заклинания, направленные на предметы.";
            result->error = TARGET_ERR_CAST_ON_WHAT;
            return result;
        }
    
        if (arg.empty( )) {
            buf << "Для этого заклинания нужно выбрать предмет в качестве цели.";
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
            buf << "Увы, в инвентаре или экипировке такого предмета найти не удается.";
        else if (target.isSet( TAR_OBJ_ROOM ))
            buf << "Увы, в этой местности такого предмета найти не удается.";
        else if (target.isSet( TAR_OBJ_WORLD ))
            buf << "Увы, во всей Тэре такого предмета найти не удается.";

        result->error = TARGET_ERR_OBJ_NOT_FOUND;
        return result;
    }

    return null;
}

bool DefaultSpell::checkPosition( Character *ch ) const
{
    if (ch->position < position.getValue( )) {
        ch->pecho("Это заклинание нельзя использовать во время сражения.");
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

    if (order.isSet(ORDER_NEVER))
        return false;

    if (!ch->is_npc() && !order.isSet(ORDER_EXCEPT_PK))
        return true;

    if (order.isSet(ORDER_PLAYER_ONLY))
        return false;    

    if (getSpellType() == SPELL_OFFENSIVE)
        return false;

    return true;
}

bool DefaultSpell::targetIsObj() const
{
    return target.isSet(TAR_OBJ_EQUIP|TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_WORLD);
}

bool DefaultSpell::targetIsRoom() const
{
    return target.isSet(TAR_ROOM);
}

bool DefaultSpell::targetIsChar() const
{
    return target.isSet(TAR_CHAR_ROOM|TAR_CHAR_SELF|TAR_CHAR_WORLD);
}

bool DefaultSpell::targetIsRanged() const
{
    return target.isSet(TAR_CHAR_ROOM);
}

bool DefaultSpell::isPrayer( Character *caster ) const
{
    if (!isCasted( ))
        return false;

    // For spells that are exclusively marked as 'magic' or 'prayer', ignore caster's class flags.
    if (flags.isSet(SPELL_MAGIC) && !flags.isSet(SPELL_PRAYER))
        return false;

    if (!flags.isSet(SPELL_MAGIC) && flags.isSet(SPELL_PRAYER))
        return true;

    // If marked as both or neither, let caster's class determine spell origin.
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
        oldact("Твои боги не благосклонны к $C3.", ch, 0, vch, TO_CHAR);
        oldact("Боги $c2 не благосклонны к тебе.", ch, 0, vch, TO_VICT);
        oldact("Боги $c2 не благосклонны к $C3.", ch, 0, vch, TO_NOTVICT);
    }
    else if (ch != vch) {
        oldact("$C1 отклоняет твое заклинание!", ch, 0, vch, TO_CHAR);
        oldact("Ты отклоняешь заклинание $c2!", ch, 0, vch, TO_VICT);
        oldact("$C1 отклоняет заклинание $c2!", ch, 0, vch, TO_NOTVICT);
    }
    else {
        oldact("Ты отклоняешь заклинание!", ch, 0, vch, TO_VICT);
        oldact("$C1 отклоняет заклинание!", ch, 0, vch, TO_NOTVICT);
    }
}

void DefaultSpell::baneDamage( Character *ch, Character *vch, int dam ) const
{
    if (is_safe_nomessage( vch, ch ))
        return;
        
    if (is_safe_nomessage( ch, vch )) 
        return;
    
    gsn_spellbane->improve( vch, true, ch );
    damage_nocatch( vch, ch, ( dam ), gsn_spellbane, DAM_NEGATIVE, true, DAMF_MAGIC );
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

int DefaultSpell::getBeats(Character *ch) const
{
    return skill->getBeats(ch); 
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

