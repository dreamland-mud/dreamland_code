/* $Id: class_ranger.cpp,v 1.1.2.25.4.25 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "logstream.h"
#include "summoncreaturespell.h"
#include "objthrow.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "spelltarget.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "pcharactermanager.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "string_utils.h"
#include "dreamland.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "magic.h"
#include "fight.h"
#include "weapongenerator.h"
#include "stats_apply.h"
#include "directions.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "merc.h"
#include "loadsave.h"
#include "save.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

PROF(ranger);
GSN(ambush);
GSN(blue_arrow);
GSN(bow);
GSN(camouflage);
GSN(green_arrow);
GSN(make_arrow);
GSN(make_bow);
GSN(mastering_bow);
GSN(red_arrow);
GSN(track);
GSN(white_arrow);

static Object * create_arrow( int color, int level );
static Object * find_arrow( Character *ch, Object *quiver );


#define OBJ_VNUM_RANGER_STAFF        28
#define OBJ_VNUM_RANGER_ARROW        6
#define OBJ_VNUM_RANGER_BOW          7

/*
 * 'track' skill command
 */
static bool oprog_track( Object *obj, Character *ch, const char *target, int door )
{
    FENIA_CALL( obj, "Track", "Csi", ch, target, door );
    FENIA_NDX_CALL( obj, "Track", "OCsi", obj, ch, target, door );
    return false;
}

SKILL_RUNP( track )
{
    DLString arg( argument );
    EXIT_DATA *pexit;
    int d;

    if (arg.empty( )) {
        ch->pecho( "Кого ты хочешь выследить?" );
        return;
    }

    oldact("$c1 всматривается в землю в поисках следов.",ch,0,0,TO_ROOM);

    int slevel;
    slevel = gsn_track->getEffective( ch ) + skill_level_bonus(*gsn_track, ch);  
    
    if (number_percent() < slevel)
        if (( d = ch->in_room->history.went( arg, false ) ) != -1) {
            if (( pexit = ch->in_room->exit[d] )) {

                gsn_track->improve( ch, true );

                for (Object *obj = ch->carrying; obj; obj = obj->next_content)
                    if (oprog_track(obj, ch, arg.c_str(), d))
                        return;

                ch->pecho("Следы %N2 ведут %s.", arg.c_str(), dirs[d].leave);
                
                if (IS_SET(pexit->exit_info, EX_CLOSED)) 
                    open_door_extra( ch, d, pexit );
                
                move_char(ch, d );
                return;
            }
        }
    
    ch->pecho("Ты не видишь здесь следов.");
    gsn_track->improve( ch, false );
}

/*
 * find an arrow in the quiver
 */
static Object * find_arrow( Character *ch, Object *quiver )
{
    Object *arrow = NULL;

    for (Object *obj = quiver->contains; obj != 0; obj = obj->next_content)
        if (obj->item_type == ITEM_WEAPON && obj->value0() == WEAPON_ARROW) {
            arrow = obj;
            break; 
        }

    if (!arrow) {
        oldact("В $o6 закончились стрелы.", ch, quiver, 0, TO_CHAR);
        return NULL;
    }

    if (ch->getModifyLevel( ) + 10 < arrow->level) {
        ch->pecho("Тебе не хватает опыта воспользоватся этой стрелой.");
        return NULL;
    }

    obj_from_obj( arrow );
    return arrow;
}

/*
 * 'shoot' command for skill 'bow' 
 */

SKILL_RUNP( shoot )
{
    Character *victim;
    Object *wield;
    Object *arrow;
    Object *quiver;
    bool success;
    int chance,direction;
    int range, range0 = ( ch->getModifyLevel() / 10) + 1;
    int master_shoots = 2;
    DLString argDoor, argVict;
    ostringstream errbuf;

    if ( ch->fighting )
    {
            ch->pecho("Сражаясь, ты не можешь прицелиться.");
            return;
    }

    if (!direction_range_argument(argument, argDoor, argVict, direction)) {
        ch->pecho("Выстрелить в каком направлении и в кого?");
        return;
    }

    range = range0;
    if ( ( victim = find_char( ch, argVict.c_str(), direction, &range, errbuf ) ) == 0 )
    {
            ch->send_to(errbuf);
            return;
    }

    if ( !victim->is_npc() && victim->desc == 0 )
    {
            ch->pecho("Ты не можешь сделать этого.");
            return;
    }

    if ( victim == ch )
    {
            ch->pecho("Это невозможно!");
            return;
    }

    if (is_safe(ch,victim))
    {
            ch->pecho("Боги покровительствуют %C3.", victim);
            return;
    }

    
    wield = get_eq_char(ch, wear_wield);
    quiver = get_eq_char(ch, wear_hold);

    if ( !wield
            || wield->item_type != ITEM_WEAPON
            || wield->value0() != WEAPON_BOW )
    {
            ch->pecho("Для того, чтобы стрелять тебе нужен лук!");
            return;            
    }

    if (!ch->is_npc( ) 
        && (get_eq_char(ch,wear_second_wield)
            || get_eq_char(ch,wear_shield)) )
    {
            ch->pecho("Твоя вторая рука должна быть свободна!");
            return;            
    }

    if (!ch->is_npc( ) && !quiver)
    {
            ch->pecho("У тебя в руках ничего нет!");
            return;            
    }        
    
    if (!ch->is_npc( ) && 
        (quiver->item_type != ITEM_CONTAINER || !IS_SET(quiver->value1(), CONT_FOR_ARROW)))
    {
            ch->pecho("Возьми в руки колчан.");
            return;
    }

    if ( ch->in_room == victim->in_room )
    {
            ch->pecho("Ты не можешь стрелять из лука в упор.");
            return;
    }

    if (ch->is_npc( )) {
        arrow = create_arrow( 0, ch->getModifyLevel( ) );
        arrow->timer = 1;
    }
    else 
        arrow = find_arrow( ch, quiver );
    
    if (!arrow)
        return;

    chance = skill_level_bonus(*gsn_bow, ch) + (gsn_bow->getEffective( ch ) - 50) * 2;
    if ( victim->position == POS_SLEEPING )
            chance += 40;
    if ( victim->position == POS_RESTING )
            chance += 10;
    if ( victim->position == POS_FIGHTING )
            chance -= 40;
    chance += ch->hitroll;
    
    ch->pecho( "%1$^O1, посланн%1$Gое|ый|ая|ые тобой, улете%1$nла|ли %2$s.", arrow, dirs[ direction ].leave );
    ch->recho( "%1$^O1, посланн%1$Gое|ый|ая|ые %3$C5, улете%1$nла|ли %2$s.", arrow, dirs[ direction ].leave, ch );

    set_violent( ch, victim, false );

    // calculate bow bonus damage: diceroll + bow damroll
    int bow_bonus = 0;    
    for (auto &paf: wield->affected)
    {
        if ( paf->location == APPLY_DAMROLL )
            bow_bonus += paf->modifier;
    } 
    bow_bonus += dice( wield->value1(), wield->value2() );
    
    try {
        success = send_arrow( ch, victim, arrow, direction, chance, bow_bonus );
    } catch (const VictimDeathException &e) {
        return;
    }
    
    gsn_bow->improve( ch, success, victim );
    
    yell_panic( ch, victim,
                "Помогите! Меня кто-то обстреливает!",
                "Помогите! Меня обстреливает %1$C1!" );
    
    if (ch->is_npc( ))
        return;

    if (number_percent() >= gsn_mastering_bow->getEffective( ch )) {
        gsn_mastering_bow->improve( ch, false, victim );
        return;
    }
    
    for (int i = 0; i < master_shoots; i++) {
        range = range0;
        if (find_char( ch, argVict.c_str(), direction, &range, errbuf) != victim) {
            ch->send_to(errbuf);
            return; 
        }
        
        if (!( arrow = find_arrow( ch, quiver ) ))
            return;
        
        try {
            success = send_arrow( ch, victim, arrow, direction, chance, bow_bonus );
        } catch (const VictimDeathException &e) {
            return;
        }

        gsn_mastering_bow->improve( ch, success, victim );
    }
}



/*
 * 'make arrow' skill 
 */
static Object * create_arrow( int color, int level )
{
    Object *arrow;
    Affect tohit, todam;
    const char *str_long, *str_short, *str_name;

    arrow = create_object(get_obj_index(OBJ_VNUM_RANGER_ARROW), 0 );
    arrow->level = level;

    tohit.type               = gsn_make_arrow;
    tohit.level              = level;
    tohit.duration           = -1;
    tohit.location = APPLY_HITROLL;
    tohit.modifier           = level / 10 + 1;
    affect_to_obj( arrow, &tohit);

    todam.type               = gsn_make_arrow;
    todam.level              = level;
    todam.duration           = -1;
    todam.location = APPLY_DAMROLL;
    todam.modifier           = level / 10 + 1;
    affect_to_obj( arrow, &todam);

    if (color != 0 && color != gsn_make_arrow)
    {
        Affect saf;

        saf.bitvector.setTable(&weapon_type2);
        saf.type               = color;
        saf.level              = level;
        saf.duration           = -1;
        
        saf.modifier           = 0;

        if ( color == gsn_green_arrow )
        {
            saf.bitvector.setValue(WEAPON_POISON);
            str_name = "green зеленая";
            str_long = "{GЗеленая";
            str_short = "{Gзелен|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 12);
            arrow->value2(4 + level / 10);
        }
        else if (color == gsn_red_arrow)
        {
            saf.bitvector.setValue(WEAPON_FLAMING);
            str_name = "red красная";
            str_long = "{RКрасная";
            str_short = "{Rкрасн|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 15);
            arrow->value2(4 + level / 30);
        }
        else if (color == gsn_white_arrow)
        {
            saf.bitvector.setValue(WEAPON_FROST);
            str_name = "white белая";
            str_long = "{WБелая";
            str_short = "{Wбел|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 15);
            arrow->value2(4 + level / 30);
        }
        else
        {
            saf.bitvector.setValue(WEAPON_SHOCKING);
            str_name = "blue голубая";
            str_long = "{CГолубая";
            str_short = "{Cголуб|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 15);
            arrow->value2(4 + level / 30);
        }

        affect_to_obj( arrow, &saf);
    }
    else
    {
        str_name = "wooden деревянная";
        str_long = "{yДеревянная";
        str_short = "{yдеревянн|ая|ой|ой|ую|ой|ой";
        arrow->value1(4 + level / 12);
        arrow->value2(4 + level / 10);
    }

    arrow->timer = Date::SECOND_IN_WEEK;

    DLString kw = String::toString(arrow->getKeyword());
    arrow->setKeyword( fmt(0, kw.c_str(), str_name ).c_str());
    arrow->setShortDescr( fmt(0, arrow->getShortDescr(LANG_DEFAULT).c_str(), str_short ), LANG_DEFAULT);        
    arrow->setDescription( fmt(0, arrow->getDescription(LANG_DEFAULT).c_str(), str_long ), LANG_DEFAULT);        
    
    return arrow;
}

/*
 * 'make arrow' skill command
 */

SKILL_RUNP( makearrow )
{
}



/*
 * 'make bow' skill command
 */

SKILL_RUNP(makebow)
{
}


/*----------------------------------------------------------------------------
 * Ambush 
 *---------------------------------------------------------------------------*/
class AmbushOneHit: public SkillWeaponOneHit {
public:
    AmbushOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

AmbushOneHit::AmbushOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_ambush )
{
}

void AmbushOneHit::calcDamage( ) 
{
    damBase( );
    damapply_class(ch, dam);
    damApplyPosition( );
    damApplyDamroll( );
    dam *= 3;

    WeaponOneHit::calcDamage( );
}    

void AmbushOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_ambush->getEffective( ch ));
}

/*
 * 'ambush' skill command
 */
SKILL_RUNP( ambush )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if ( MOUNTED(ch) )
    {
            ch->pecho("Ты не можешь быть в засаде верхом!");
            return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
            if (ch->ambushing.empty())
            {
                    ch->pecho("Засаду кому?");
                    return;
            }
            else
            {
                    ch->pecho("Ты сидишь в засаде на '%s'.", ch->ambushing.c_str());
                    return;
            }
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
            if ( !IS_AFFECTED(ch,AFF_CAMOUFLAGE) )
            {
                    ch->pecho("Сначала тебе стоит замаскироваться.");
                    return;
            }
            ch->pecho("Ты готовишься к засаде.");
            ch->ambushing = arg;
            return;
    }

    if ( victim == ch )
    {
            ch->pecho("Засаду себе? Это еще как?!");
            return;
    }

    if ( !IS_AFFECTED(ch,AFF_CAMOUFLAGE) )
    {
            ch->pecho("Сначала тебе стоит замаскироваться.");
            return;
    }   

    if ( victim->can_see(ch) )
    {
            ch->pecho("Твоя жертва тебя видит.");
            return;
    }

    if ( is_safe( ch, victim ) )
            return;

    AmbushOneHit amb( ch, victim );
    
    try {
        if ( !IS_AWAKE(victim)
                || ch->is_npc()
                || number_percent( ) < gsn_ambush->getEffective( ch ) + skill_level_bonus(*gsn_ambush, ch) )
        {
                gsn_ambush->improve( ch, true, victim );
                amb.hit( );
        }
        else
        {
                gsn_ambush->improve( ch, false, victim );
                amb.miss( );
        }

        yell_panic( ch, victim,
                    "Помогите! На меня кто-то напал из засады!",
                    "Помогите! На меня из засады напа%1$Gло|л|ла %1$C1!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}

SKILL_APPLY( ambush )
{
    Character *vch, *vch_next;

    if (ch->ambushing.empty())
        return false;
    if (!IS_AWAKE(ch))
        return false;
    if (ch->fighting)
        return false;
    if (!IS_AFFECTED(ch, AFF_CAMOUFLAGE))
        return false;

    for (vch = ch->in_room->people; vch; vch = vch_next) {
        vch_next = vch->next_in_room;

        if (ch != vch
                && ch->can_see(vch)
                && !vch->can_see(ch)
                && !is_safe_nomessage(ch,vch)
                && is_name(ch->ambushing.c_str(), vch->getNameC()))
        {
            ch->pecho( "{YТЫ ВЫСКАКИВАЕШЬ ИЗ ЗАСАДЫ!{x" );
            run( ch, const_cast<char *>(ch->ambushing.c_str()) );
            return true;
        }
    }

    return false;
}   


/*
 * 'camouflage' skill command
 */

SKILL_RUNP( camouflage )
{
        if ( MOUNTED(ch) )
        {
                ch->pecho("Ты не можешь замаскироваться, когда ты в седле.");
                return;
        }

        if ( RIDDEN(ch) )
        {
                ch->pecho("Ты не можешь замаскироваться, когда ты оседлан%Gо||а.", ch);
                return;
        }

        if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
        {
                ch->pecho("Ты не можешь замаскироваться, когда светишься.");
                return;
        }

        if (!RoomUtils::isNature(ch->in_room))
        {
                ch->pecho("Ты можешь замаскироваться только в дикой местности.");
                oldact("$c1 пытается замаскироваться, но не может найти укрытия.",ch,0,0,TO_ROOM);
                return;
        }

        int k = ch->getLastFightDelay( );

        if ( k >= 0 && k < FIGHT_DELAY_TIME )
                k = k * 100 /        FIGHT_DELAY_TIME;
        else
                k = 100;

        if ( IS_AFFECTED(ch, AFF_CAMOUFLAGE) )
        {
                REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
                ch->ambushing.clear();
        }

        if ( ch->is_npc()
                || number_percent( ) < ( gsn_camouflage->getEffective( ch ) + skill_level_bonus(*gsn_camouflage, ch) ) * k / 100 )
        {
                ch->pecho("Ты маскируешься на местности.");
                SET_BIT(ch->affected_by, AFF_CAMOUFLAGE);
                gsn_camouflage->improve( ch, true );
        }
        else {
                ch->pecho("Твоя попытка замаскироваться закончилась неудачей.");
                gsn_camouflage->improve( ch, false );
        }

        return;
}

