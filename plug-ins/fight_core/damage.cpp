/* $Id: damage.cpp,v 1.1.2.16 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "damage.h"
#include "damageflags.h"

#include "logstream.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "skill.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "loadsave.h"
#include "interp.h"
#include "act.h"
#include "wiznet.h"
#include "occupations.h"
#include "mercdb.h"
#include "merc.h"
#include "vnum.h"
#include "deathmanager.h"
#include "fight_safe.h"
#include "fight_position.h"
#include "fight_exception.h"
#include "immunity.h"
#include "material.h"
#include "def.h"

GSN(resistance);
GSN(dark_shroud);
GSN(protection_heat);
GSN(protection_cold);
GSN(prayer);
GSN(stardust);

void do_visible( Character * );

/*-----------------------------------------------------------------------------
 * Damage 
 *----------------------------------------------------------------------------*/
Damage::Damage( )
          : ch( NULL ), victim( NULL ), killer( NULL ),
	    dam_type( 0 ), dam( 0 ),  immune( false ), dam_flag( 0 )
{
}

Damage::Damage( Character *ch, Character *victim, int dam_type, int dam, bitstring_t dam_flag )
{
    this->ch = ch;
    this->victim = victim;
    this->dam_type = dam_type;
    this->dam = dam;
    this->dam_flag = dam_flag;
    
    killer = ch;
    immune = false;
}

bool Damage::hit( bool show )
{
    if (!canDamage( ))
	return false;

    calcDamage( );
    priorDamageEffects( );

    if (show)
	message( );

    if (dam == 0)
	return false;

    inflictDamage( );
    postDamageEffects( );
    handlePosition( );
    checkRetreat( );
    return true;
}

void Damage::priorDamageEffects( )
{
}

void Damage::postDamageEffects( )
{
}

bool Damage::canDamage( )
{
    if (victim->position == POS_DEAD)
	return false;

    if (victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_NO_DAMAGE)) 
	return false;
    
    if (is_safe( ch, victim ))
	return false;

    adjustPosition( );
    adjustFighting( );
    
    if (adjustMasterAttack( ))
	return false;

    adjustFollowers( );
    adjustDeathTime( );
    adjustAdrenaline( );
    adjustVisibility( );
    return true;
}

void Damage::adjustPosition( )
{
    if (ch != victim && victim->position > POS_STUNNED) {
	if (victim->timer <= 4 && ch->in_room == victim->in_room)
	    victim->position = POS_FIGHTING;
    }
}

static bool mprog_attack( Character *rch, Character *ch, Character *victim )
{
    FENIA_CALL( rch, "Attack", "CC", ch, victim );
    FENIA_NDX_CALL( rch->getNPC( ), "Attack", "CCC", rch, ch, victim );
    return false;
}

static void rprog_attack( Character *ch, Character *victim )
{
    Character *rch, *rch_next;
    
    for (rch = ch->in_room->people; rch; rch = rch_next) {
	rch_next = rch->next_in_room;
	mprog_attack( rch, ch, victim );	
    }

    if (ch->in_room != victim->in_room)
	for (rch = victim->in_room->people; rch; rch = rch_next) {
	    rch_next = rch->next_in_room;
	    mprog_attack( rch, ch, victim );	
	}
}

void Damage::adjustFighting( )
{
    if (ch != victim && victim->position > POS_STUNNED) {
	if (victim->fighting == 0)
	    set_fighting( victim, ch );
	
	if (ch->fighting == 0) {
	    set_fighting( ch, victim );
	    rprog_attack( ch, victim );
	}
    }

    if (ch->isDead( ) || victim->isDead( ))
	throw VictimDeathException( );
}

void Damage::adjustFollowers( )
{
    if (ch == victim)
	return;

    if (victim->master == ch)
	victim->stop_follower( );

    if (ch->master == victim)
	ch->stop_follower( );

    if (victim->mount == ch) 
	victim->dismount( );
}

/* 
 * Remove (P)rotected 
 */
void Damage::adjustDeathTime( )
{
    UNSET_DEATH_TIME(ch);
}

void Damage::adjustAdrenaline( )
{
    set_violent( ch, victim, false );
}

/*
 * No one in combat can sneak, hide, or be invis or camoed.
 */
void Damage::adjustVisibility( )
{
    do_visible( ch );
}

/*
 * If victim is charmed, ch might attack victim's master.
 */
bool Damage::adjustMasterAttack( )
{
    if (victim == ch)
	return false;

    if (victim->position <= POS_STUNNED)
	return false;

    if (ch->is_npc( )
	&& victim->is_npc( )
	&& IS_AFFECTED(victim, AFF_CHARM)
	&& victim->master != 0
	&& victim->master->in_room == ch->in_room
	&& number_bits( 3 ) == 0)
    {
	stop_fighting( ch, false );
	set_fighting( ch, victim->master );
	return true;
    }
    
    return false;
}

/*-----------------------------------------------------------------------------
 * Damage modifiers.
 *----------------------------------------------------------------------------*/
void Damage::protectMaterial( Object *obj )
{
    switch (material_immune( obj, victim )) {
    case RESIST_VULNERABLE:
	dam += dam / 2;
	break;
    case RESIST_RESISTANT:
	dam -= dam / 3;
	break;
    case RESIST_IMMUNE:
	dam = 0;
	immune = true;
	break;
    }
}
void Damage::protectSanctuary( ) 
{
    switch (dam_type) {
    case DAM_POISON:
    case DAM_DISEASE:
    case DAM_DROWNING:
	return;
    }

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
	dam /= 2;
    }
    else if (victim->isAffected(gsn_stardust)) {
	dam /= 2;
    }
    else if (victim->isAffected(gsn_dark_shroud)) {
	if (victim->is_npc( ) 
		&& victim->getNPC( )->behavior
		&& IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)))
	    dam /= 2;
	else
	    dam = dam * 6 / 10;
    }
    else 
	protectResistance( );
}

void Damage::protectResistance( )
{
    if (victim->isAffected(gsn_resistance)) {
	dam -= victim->applyCurse( dam * 2 / 5 );
    }
}

void Damage::protectAlign( )
{
    if (   (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
	|| (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))) 
    {
	dam -= dam / 4;
    }
}

void Damage::protectTemperature( )
{
    if ( victim->isAffected(gsn_protection_heat)
	&& (dam_type == DAM_FIRE) )
	dam -= dam / 4;

    if ( victim->isAffected(gsn_protection_cold)
	&& ( dam_type == DAM_COLD) )
	dam -= dam / 4;
}

void Damage::protectImmune( )
{
    switch(immune_check(victim, dam_type, dam_flag)) {
    case RESIST_IMMUNE:
	immune = true;
	dam = 0;
	break;
    case RESIST_RESISTANT:    
	dam -= dam/3;
	break;
    case RESIST_VULNERABLE:
	dam += dam/2;
	break;
    }
}

void Damage::protectRazer( )
{
    //By Razer - damage redusing
    if (dam > 10) dam = dam * 4 / 5;
}

void Damage::protectPrayer( )
{
    if (victim->isAffected( gsn_prayer )) 
	dam -= dam * victim->getModifyLevel( ) / 1200;
}

void Damage::calcDamage( )
{
    protectSanctuary( );
    protectAlign( );
    protectTemperature( );
    protectPrayer( );
    protectImmune( );
    protectRazer( ); // >8)
}

/*-----------------------------------------------------------------------------
 * Inflict actual damage
 *----------------------------------------------------------------------------*/
void Damage::inflictDamage( )
{
    undig(victim);

    // log damages
    if (!ch->is_immortal( ) && !ch->is_npc( ) && dam > 2000) 
	wiznet( WIZ_DAMAGES, 0, 0, "%^C1 : повреждения более 2000 : %d", ch, dam );
    
    victim->hit -= dam;
    
    if (victim->is_immortal( ))
	victim->hit = max( (int)victim->hit, 1 );
}

/*-----------------------------------------------------------------------------
 * Inform the victim of his new state.
 *----------------------------------------------------------------------------*/
void Damage::reportState( )
{
    switch( victim->position.getValue( ) ) {
    case POS_MORTAL:
	act_p( "$c1 смертельно ране$gно|н|на и скоро умрет, если $m не помогут.",
	    victim, 0, 0, TO_ROOM,POS_RESTING);
	act_p( "Ты смертельно ране$gно|н|на и скоро умрешь, если тебе не помогут.",
	    victim, 0, 0, TO_CHAR,POS_DEAD);
	break;
    case POS_INCAP:
	act_p( "$c1 совершенно беспомощ$gно|ен|на и скоро умрет, если $m не помогут.",
	    victim, 0, 0, TO_ROOM,POS_RESTING);
	act_p( "Ты совершенно беспомощ$gно|ен|на и скоро умрешь, если тебе не помогут.",
	    victim, 0, 0, TO_CHAR,POS_DEAD);
	break;
    case POS_STUNNED:
	act_p( "$c1 без сознания, но возможно придет в себя.",
	    victim, 0, 0, TO_ROOM,POS_RESTING);
	victim->send_to("Ты без сознания, но еще можешь придти в себя.\n\r");
	break;
    default:
	if ( dam > victim->max_hit / 4 )
	    victim->send_to("Это действительно было БОЛЬНО!\n\r");
	if ( victim->hit < victim->max_hit / 4 )
	    victim->send_to("Ты истекаешь {RКРОВЬЮ{x!\n\r");
	break;
    }
}

void Damage::handlePosition( )
{
    update_pos( victim );
    
    if (victim->position == POS_DEAD) {
	handleDeath( );
	throw VictimDeathException( );
    }
    
    reportState( );

    /*
     * Sleep spells and extremely wounded folks.
     */
    if (!IS_AWAKE(victim)) 
	stop_fighting( victim, false );
}

/*-----------------------------------------------------------------------------
 * Take care of link dead people. See if we need to wimp out.
 *----------------------------------------------------------------------------*/
 bool Damage::checkRetreat( )
 {
     if (victim == killer)
	 return false;

    if (  victim != 0 && !victim->is_npc() && victim->desc == 0 )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    if( victim->getModifyLevel() < 11 )
		interpret_raw( victim, "recall" );
	    else
		interpret_raw( victim, "flee" );

	    return true;
	}
    }

    if ( victim != 0 && !victim->is_npc()
	&& victim->hit > 0
	&& ( victim->hit <= victim->wimpy || CAN_DETECT(victim, ADET_FEAR) )
	&& victim->wait < dreamland->getPulseViolence( ) / 2 )
	interpret_raw( victim, "flee" );

    return false;
}

/*-----------------------------------------------------------------------------
 * Payoff for killing things 
 *----------------------------------------------------------------------------*/
void Damage::handleDeath( ) 
{
    act_p( "$c1 уже {RТРУП{x!!", victim, 0, 0, TO_ROOM,POS_RESTING);
    victim->send_to("Тебя {RУБИЛИ{x!!\n\r\n\r");
    
    DeathManager::getThis( )->handleDeath( killer, victim );
}


/*-----------------------------------------------------------------------------
 * Damage reporting: new messages
 *----------------------------------------------------------------------------*/

static void drawRushechki(char *buf, const char *msg, const char *art, char color)
{
    const char *s;

    *buf++ = '{';
    *buf++ = color;

    for(s = art; *s; s++)
	*buf++ = *s;

    for(s = msg; *s; s++)
	*buf++ = *s;

    *buf++ = '{';
    *buf++ = color;

    for(s = art+strlen(art)-1; s >= art; s--, buf++)
	switch (*s) {
	case '>': *buf = '<'; break;
	case '<': *buf = '>'; break;
	case ')': *buf = '('; break;
	case '(': *buf = ')'; break;
	default:  *buf = *s;  break;
	}

    *buf++ = '{';
    *buf++ = 'x';

    *buf++ = 0;
}

void Damage::msgNewFormat( bool vs, char *buf )
{
    static const struct {
	int dam;
	char color;
	const char *art;
    } absTable [] = 
    {
      {   0,  'w', " "                     },
      {   4,  'w', " "                     },
      {   8,  'w', " "                     },
      {  12,  'w', " "                     },
      {  16,  'w', " "                     },
      {  20,  'w', " "                     },
      {  24,  'w', " "                     },
      {  28,  'w', " - "                   },
      {  32,  'w', " -- "                  },
      {  36,  'w', " --- "                 },
      {  42,  'M', " "                     },
      {  52,  'M', " "                     },
      {  65,  'M', " ^ "                   },
      {  80,  'M', " ^^ "                  },
      { 100,  'M', " ^^^ "                 },
      { 130,  'G', " *** "                 }, 
      { 175,  'G', " **** "                }, 
      { 250,  'B', " === "                 }, 
      { 325,  'B', " ==== "                }, 
      { 400,  'R', " <*> <*> "             }, 
      { 500,  'R', " <*>!<*> "             }, 
      { 650,  'R', " <*><*><*> "           }, 
      { 800,  'R', " (<*>)!(<*>) "         }, 
      { 1000, 'R', " (*)!(*)!(*) "         }, 
      { 1250, 'R', " (*)!<*>!(*) "         }, 
      { 1500, 'R', " <*>!(*)!<*>> "        }, 
      {   -1, 'R', " \007=<*) (*>= ! "     }, 
    };

    static const struct {
	int perc;
	const char *vs, *vp;
    } relTable [] =
    {
      {   0,  "слегка царапаешь",            "слегка царапает"       },
      {   1,  "царапаешь",                   "царапает"	         },
      {   2,  "слегка задеваешь",            "слегка задевает"       },
      {   4,  "задеваешь",                   "задевает"              },
      {   5,  "слегка повреждаешь",          "слегка повреждает"     },
      {   6,  "повреждаешь",                 "повреждает"            },
      {   7,  "слегка ранишь",               "слегка ранит"          },
      {   8,  "ранишь",                      "ранит"	         }, 
      {   9,  "сильно ранишь",               "сильно ранит"          },
      {  10,  "сокрушаешь",                  "сокрушает"		 }, 
      {  11,  "разрушаешь",                  "разрушает"		 },
      {  12,  "отбрасываешь",                "отбрасывает"           },
      {  13,  "калечишь",                    "калечит"		 },
      {  14,  "уродуешь",                    "уродует"		 },
      {  15,  "СОКРУШАЕШЬ",                    "СОКРУШАЕТ"		 }, 
      {  17,  "РАЗРУШАЕШЬ",                    "РАЗРУШАЕТ"		 },
      {  20,  "РАСЧЛЕНЯЕШЬ",                   "РАСЧЛЕНЯЕТ"	 	 },
      {  25,  "ОПОТРОШАЕШЬ",                   "ОПОТРОШАЕТ"		 },
      {  30,  "ТРАВМИРУЕШЬ",                   "ТРАВМИРУЕТ"		 },
      {  35,  "КАЛЕЧИШЬ",                      "КАЛЕЧИТ" 		 },
      {  40,  "РАЗИШЬ",                        "РАЗИТ" 	                 },
      {  50,  "ПОТРОШИШЬ",                     "ПОТРОШИТ"		 },
      {  75,  "ПРЕВРАЩАЕШЬ В КРОВАВОЕ МЕСИВО", "ПРЕВРАЩАЕТ В КРОВАВОЕ МЕСИВО"},
      { 100,  "СМЕРТЕЛЬНО РАНИШЬ",             "СМЕРТЕЛЬНО РАНИТ"	 },
      { -1,   "{DУБИВАЕШЬ{x",                  "{D! УБИВАЕТ !{x"	 },
    };
    
    int i, j;
    int perc = dam * 100 / victim->max_hit;

    for (i = 0; absTable[i].dam >= 0 && dam > absTable[i].dam; i++)
	;
    for (j = 0; relTable[j].perc >= 0 && perc > relTable[j].perc; j++)
	;

    if(dam == 0)
	strcpy(buf, vs ? " промахиваешься мимо " : " промахивается мимо ");
    else
	drawRushechki(buf, 
		vs ? relTable[j].vs : relTable[j].vp, 
		absTable[i].art, absTable[i].color);
}

/*-----------------------------------------------------------------------------
 * Damage reporting: old messages
 *----------------------------------------------------------------------------*/
void Damage::msgOldFormat( bool vs, char *buf )
{
    static const struct {
        int dam;
        char color;
        const char *art;
        const char *vs, *vp;
    } msgTable [] =
    {
     {    0, 'w', " ", "промахиваешься мимо",  "промахивается мимо" },
     {    1, 'w', " ", "слегка царапаешь",     "слегка царапает" },
     {    4, 'w', " ", "царапаешь",            "царапает" },
     {    8, 'w', " ", "задеваешь",            "задевает" },
     {   12, 'w', " ", "больно задеваешь",     "больно задевает" },
     {   16, 'w', " ", "немного ранишь",       "немного ранит" },
     {   20, 'w', " ", "ранишь",               "ранит" },
     {   24, 'w', " ", "сильно ранишь",        "сильно ранит" },
     {   28, 'w', " ", "разрушаешь",           "разрушает" },
     {   32, 'w', " ", "уродуешь",             "уродует" },
     {   36, 'w', " ", "калечишь",             "калечит" },
     {   42, 'M', " ", "УРОДУЕШЬ",             "УРОДУЕТ" },
     {   52, 'M', " ", "УНИЧТОЖАЕШЬ",          "УНИЧТОЖАЕТ" },
     {   65, 'M', " ", "РАСЧЛЕНЯЕШЬ",          "РАСЧЛЕНЯЕТ" },
     {   80, 'M', " ", "ОПОТРОШАЕШЬ",          "ОПОТРОШАЕТ" },
     {  100, 'M', " ", "ПЕРЕМАЛЫВАЕШЬ",        "ПЕРЕМАЛЫВАЕТ"	},
      
     {  130, 'G', " *** ",         "СОКРУШАЕШЬ",          "СОКРУШАЕТ"	},
     {  175, 'G', " *** ",         "СНОСИШЬ ВСЕ НА ПУТИ", "СНОСИТ ВСЕ НА ПУТИ" },
     {  250, 'B', " === ",         "СТИРАЕШЬ В ПОРОШОК",  "СТИРАЕТ В ПОРОШОК" },
     {  325, 'B', " === ",         "РАСПЫЛЯЕШЬ НА АТОМЫ", "РАСПЫЛЯЕТ НА АТОМЫ" },
     {  400, 'R', " <*> <*> ",     "АННИГИЛИРУЕШЬ",       "АННИГИЛИРУЕТ" },
     {  500, 'R', " <*>!<*> ",     "ВЫРЫВАЕШЬ С КОРНЕМ",  "ВЫРЫВАЕТ С КОРНЕМ" },
     {  650, 'R', " <*><*><*> ",   "ELECTRONIZE",         "ELECTRONIZES" },
     {  800, 'R', " (<*>)!(<*>) ", "РАЗРЫВАЕШЬ НА КУСКИ", "РАЗРЫВАЕТ НА КУСКИ" },
     { 1000, 'R', " (*)!(*)!(*) ", "NUKE",                "NUKES" },
     { 1250, 'R', " (*)!<*>!(*) ", "TERMINATE",           "TERMINATES" },
     { 1500, 'R', " <*>!(*)!<*>> ","TEAR UP",             "TEARS UP" },
     {   -1, 'R', " =<*) (*>= ! ", "\007POWER HIT",       "\007POWER HITS" },
    };

    int i;

    for (i = 0; msgTable[i].dam >= 0 && dam > msgTable[i].dam; i++)
	;

    drawRushechki(buf, 
	    vs ? msgTable[i].vs : msgTable[i].vp, 
	    msgTable[i].art, msgTable[i].color);
}

/*-----------------------------------------------------------------------------
 * Damage reporting utilities
 *----------------------------------------------------------------------------*/
char Damage::msgPunct( )
{
    if( victim->getModifyLevel() < 20 )    
	return (dam <= 24) ? '.' : '!';
    else if( victim->getModifyLevel() < 50)  
	return (dam <= 50 ) ? '.' : '!';
    else 
	return (dam <= 75) ? '.' : '!';
}

int Damage::msgNoSpamBit( )
{
    return CONFIG_FIGHTSPAM;
}

void Damage::msgEcho(Character *to, const char *f, va_list va)
{
    if (!to->getPC( ))
	return;

    if (dam == 0
	&& !to->is_npc( )
	&& !IS_SET(to->getPC( )->config, msgNoSpamBit( )))
	return;
    
    if (!IS_AWAKE(to) || !to->can_sense( ch ) || !to->can_sense( victim ))
        return;

    ostringstream buf;
    const char *v;
    char damVs[256], damVp[256];
    
    if(!to->is_npc() && IS_SET(to->getPC()->config, CONFIG_NEWDAMAGE)) {
	msgNewFormat(true, damVs);
	msgNewFormat(false, damVp);
    } else {
	msgOldFormat(true, damVs);
	msgOldFormat(false, damVp);
    }

    for(;*f;f++)
	switch(*f) {
	case '\5':
	    for(v=damVs;*v;v++)
		buf << *v;
	    break;
	case '\6':
	    for(v=damVp;*v;v++)
		buf << *v;
	    break;
	default:
	    buf << *f;
	}

    buf << msgPunct();
    to->vpecho(buf.str().c_str(), va);
}

void Damage::msgChar( const char *fmt, ... )
{
    va_list va;

    va_start(va, fmt);
    msgEcho(ch, fmt, va);
    va_end(va);
}

void Damage::msgVict( const char *fmt, ... )
{
    va_list va;

    va_start(va, fmt);
    msgEcho(victim, fmt, va);
    va_end(va);
}

void Damage::msgRoom( const char *fmt, ... )
{
    Character *rch;
    va_list va;
    
    va_start(va, fmt);

    for(rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if(rch != victim && rch != ch) 
	    msgEcho(rch, fmt, va);
	    
    va_end(va);
}

