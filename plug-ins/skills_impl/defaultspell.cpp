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

#include "affect.h"
#include "pcharacter.h"
#include "object.h"
#include "room.h"
#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "act_move.h"
#include "profflags.h"
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

DefaultSpell::DefaultSpell( ) 
	: target( TAR_IGNORE, &target_table ), 
	  position( POS_STANDING, &position_table ), 
	  type( SPELL_NONE, &spell_types ),
	  casted( true )
	  
{
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
    int level = skill->getLevel( ch );
    
    if (type == SPELL_NONE || type == SPELL_DEFENSIVE)
	return 0;

    if (position.getValue( ) == POS_STANDING)
        return 0;
	
    if (level < 26)
        return 0;
	
    return level / 10;
}

/*
 * Find a char for spell usage.
 */
Character * 
DefaultSpell::getCharSpell( Character *ch, const DLString &argument, int *door, int *range )
{
    char buf[MAX_INPUT_LENGTH];
    unsigned int i, j;

    for (i = 0; argument[i] != '\0' && argument[i] != '.'; i++)
	buf[i] = argument[i];
    buf[i] = '\0';

    if (i == 0 || (*door = direction_lookup(buf)) < 0)
	return get_char_room( ch, argument.c_str( ) );
    
    for (i++, j = 0; i < argument.size( ); j++, i++)
	buf[j] = argument[i];
    buf[j] = '\0';
    
    return find_char( ch, buf, *door, range, false );
}

/*
 * Utter mystical words for a spell.
 */
void DefaultSpell::utter( Character *ch )
{
    char buf  [MAX_STRING_LENGTH];
    char spellName [MAX_STRING_LENGTH];
    Character *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	const char *	old;
	const char *	_new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"	    	},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "ust",        "lon"           },
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    strcpy( spellName, skill->getName( ).c_str( ) );

    for ( pName = spellName; *pName != '\0'; pName += length ) {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ ) {
	    if ( !str_prefix( syl_table[iSyl].old, pName ) ) {
		strcat( buf, syl_table[iSyl]._new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    const char *pat = "$c1 бормочет '$t'.";

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
	if (rch != ch) {
	    int chance = (gsn_spell_craft->getEffective( rch ) * 9) / 10;
	    
	    if (chance < number_percent( ))
		act( pat, ch, buf, rch, TO_VICT );
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
    
    if (ch->getTrueProfession( )->getFlags( ).isSet(PROF_CASTER))
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
    
    if (skill->getGroup( ) == group_maladictions
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

    if (skill->getGroup( ) == group_benedictions
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

    if (skill->getGroup( ) == group_healing 
	|| skill->getGroup( ) == group_curative)
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
    if (skill->getGroup( ) == group_combat
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
	    slevel += ch->applyCurse( number_range( 1, 4 ) );
	    gsn_mastering_spell->improve( ch, true );
	}
	else
	    gsn_mastering_spell->improve( ch, false );
    }

    if (ch->getCurrStat(STAT_INT ) > 21)
      slevel = max( 1, (slevel + (ch->getCurrStat(STAT_INT ) - 21)) );
    else
      slevel = max( 1, slevel );

    for (Affect *paf = ch->affected; paf; paf = paf->next)
	if (paf->location == APPLY_LEVEL)
	    slevel += paf->modifier;
    
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
    SpellTarget::Pointer null;

    strcpy( carg, arg.c_str( ) );
    
    if (target.isSet( TAR_IGNORE|TAR_CREATE_OBJ )) {
	result->type = SpellTarget::NONE;
	result->arg = arg.c_str( );
	return result;
    }

    if (target.isSet( TAR_CREATE_MOB )) {
	if (!arg.empty( )) {
	    if (!( victim = get_char_room( ch, arg.c_str( ) ) )) {
		buf << "Кого именно ты хочешь позвать?" << endl;
		return null;
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
		return null;
	    }

	    result->type = SpellTarget::CHAR;
	    result->victim = ch;
	    return result;
	}

	buf << "Этому заклинанию не нужно указывать цель." << endl;
	return null;
    }

    if (target.isSet( TAR_CHAR_WORLD )) {
	if (arg.empty( )) {
	    buf << "Колдовать на кого?" << endl;
	    return null;
	}

	victim = get_char_world_doppel( ch, carg );

	if (victim) {
	    result->type = SpellTarget::CHAR;
	    result->victim = victim;
	    return result;
	}
	
	if (( result = locateTargetObject( ch, arg, buf ) ))
	    return result;
	
	buf.str( "" );
	buf << "Твоя попытка закончилась неудачей." << endl;
	return null;
    }
    
    if (target.isSet( TAR_CHAR_SELF )) {
	if (!arg.empty( ) && !is_self_name( arg, ch )) {
	    if (( result = locateTargetObject( ch, arg, buf ) ))
		return result;

	    buf.str( "" );
	    buf << "Ты не можешь использовать это заклинание на других." << endl;
	    return null;
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
		victim = getCharSpell( ch, arg, &result->door, &maxrange );

		if (victim) {
		    if (ch->isAffected(gsn_garble ) && number_percent( ) < 10)
			victim = ch;
			
		    if (victim->is_npc() && IS_SET(victim->act,ACT_NOTRACK )
                        && victim->in_room != ch->in_room )
		    {
			buf << "Твое заклинание не действует на "
			    << victim->getNameP( '4' ) << " на таком расстоянии." << endl;
			return null;
		    }
		    
		    result->range = std::max( 0, getMaxRange( ch ) - maxrange );
		    result->castFar = true;
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

	if (( result = locateTargetObject( ch, arg, buf ) ))
	    return result;
	    
	buf.str( "" );
	buf << "Произнести заклинание.. на кого?" << endl;
	return null;
    }

    if (( result = locateTargetObject( ch, arg, buf ) ))
	return result;
    
    if (buf.str( ).empty( ))
	buf << "Произнести заклинание... на кого?" << endl;

    return null;
}

SpellTarget::Pointer
DefaultSpell::locateTargetObject( Character *ch, const DLString &arg, std::ostringstream &buf )
{
    char carg[MAX_STRING_LENGTH];
    Object *obj;
    SpellTarget::Pointer null;
    
    strcpy( carg, arg.c_str( ) );
    
    if (ch->is_npc( ))
	return null;
    
    if (target.isSet( TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM|TAR_OBJ_WORLD )) {
	obj = NULL;

	if (arg.empty( )) {
	    buf << "Произнести заклинание на что?" << endl;
	    return null;
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
	    SpellTarget::Pointer result( NEW );

	    result->type = SpellTarget::OBJECT;
	    result->obj = obj;
	    return result;
	}

	if (target.isSet( TAR_OBJ_INV|TAR_OBJ_EQUIP ))
	    buf << "У тебя нет этого." << endl;
	else if (target.isSet( TAR_OBJ_ROOM ))
	    buf << "Ты не видишь здесь такого предмета." << endl;
	else if (target.isSet( TAR_OBJ_WORLD ))
	    buf << "В Dream Land нет ничего похожего на это." << endl;
    }

    return null;
}

bool DefaultSpell::checkPosition( Character *ch ) const
{
    if (ch->position < position.getValue( )) {
	ch->println("Ты не можешь сконцентрироваться.");
	return false;
    }
    
    return true;
}

bool DefaultSpell::isPrayer( Character *caster ) const
{
    if (!isCasted( ))
	return false;

    if (getSkill( )->getGroup( ) == group_clan)
	return false;

    if (getSkill( )->getGroup( ) == -1)
	return false;
    
    return caster->getTrueProfession( )->getFlags( caster ).isSet(PROF_DIVINE);
}

/*
 * spellbane
 * - prayers:
 *	+ no reaction if casted nearby or ranged
 *	+ block everything defensive targeted on you w/o damage
 *      + pass everyting offensive w/o messages
 *
 * - magical spells:
 *	+ deflect everything defensive on you, with msg and small damage on pk
 *	+ slightly damage pk if anything is casted nearby but not on you 
 *	  or your foe, or your clanmate
 *	+ throw dice to deflect offensive on you, results: 
 *	  pass all, partial, partial with spellbane, none with spellbane
 *	+ partially block (with serious damage on pk) all offensive directed to 
 *	  the victim you or your group is fighting with, near and ranged
 *
 *  - pets of btr block all defensive casts
 */
void DefaultSpell::baneMessage( Character *ch, Character *vch ) const
{
    if (isPrayer( ch )) {
	act("Твои боги неблагосклонны к $C3.", ch, 0, vch, TO_CHAR);
	act("Боги $c2 неблагосклонны к тебе.", ch, 0, vch, TO_VICT);
	act("Боги $c2 неблагосклонны к $C3.", ch, 0, vch, TO_NOTVICT);
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
    damage_nocatch( vch, ch, vch->applyCurse( dam ), gsn_spellbane, DAM_NEGATIVE, true, DAMF_SPELL );
}

void DefaultSpell::baneAround( Character *ch, int failChance, int dam ) const
{
    if (ch->is_npc( ) && !IS_AFFECTED(ch, AFF_CHARM))
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


/*
bool 
DefaultSpell::spellbane( Character *ch, Character *victim ) const
{    
    bool fAttack = false;
    bool fPrayer = false;

    if (target.isSet( TAR_CHAR_WORLD ))
	return false;

    if (victim == 0) 
	return false;
    
    if (!victim->isAffected( gsn_spellbane ))
	return false;
    
    if (!ch->is_npc( )) {
	if (ch->getProfession( ) == prof_cleric || ch->getProfession( ) == prof_paladin) 
	    if (getSkill( )->getGroup( ) != GROUP_CLAN 
		&& getSkill( )->getGroup( ) != -1)
	    {
		fPrayer = true;
	    }
    }
    else {
	if (IS_SET(ch->act, ACT_CLERIC))
	    fPrayer = true;
    }

    
    if (type == SPELL_OFFENSIVE) {
	int chance;
	
	if (fPrayer)
	    return false;

	chance = gsn_spellbane->getEffective( victim ); 

	if (!victim->is_npc( )) {
	    chance = 2 * chance / 3;

	    if (ch->is_npc( ))
		chance = chance / 2;
	}
	
	if (number_percent( ) > chance) {
	    gsn_spellbane->improve( victim, false, ch );
	    return false;
	}
	else 
	    gsn_spellbane->improve( victim, true, ch );

	fAttack = true;
    }

    if (ch == victim) {
	act("Твоя магическая защита (spellbane) отклоняет заклинание!", ch,0,0,TO_CHAR);
	act("Магическая защита (spellbane) $c2 отклоняет заклинание!", ch,0,0,TO_ROOM);
	damage( victim, ch, 3 * victim->getModifyLevel(), gsn_spellbane,DAM_NEGATIVE, true, DAMF_SPELL);
    }
    else {
	if (fPrayer) {
	    act("Твои боги неблагосклонны к $C3.", ch, 0, victim, TO_CHAR);
	    act("Боги $c2 неблагосклонны к тебе.", ch, 0, victim, TO_VICT);
	    act("Боги $c2 неблагосклонны к $C3.", ch, 0, victim, TO_NOTVICT);
	}
	else {
	    act("$C1 отклоняет твое заклинание!",ch,0,victim,TO_CHAR);
	    act("Ты отклоняешь заклинание $c2!",ch,0,victim,TO_VICT);
	    act("$C1 отклоняет заклинание $c2!",ch,0,victim,TO_NOTVICT);
	}
	
	if (fAttack) {
	    if (!IS_SLAIN( victim )) 
		damage( victim, ch, 3 * victim->getModifyLevel( ), gsn_spellbane, DAM_NEGATIVE, true, DAMF_SPELL );

	    if (!is_safe_nomessage( victim, ch )) 
		multi_hit( victim, ch );
	}
    }

    return true;
}
*/

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
SkillPointer DefaultSpell::getSkill( ) const
{
    return skill;
}
bool DefaultSpell::isCasted( ) const
{
    return casted.getValue( );
}

