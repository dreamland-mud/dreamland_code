/* $Id$
 *
 * ruffina, 2004
 */
#include "skillreference.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"

#include "dreamland.h"
#include "onehit.h"
#include "damage_impl.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "magic.h"
#include "fight.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"

#include "def.h"

GSN(rear_kick);

/*----------------------------------------------------------------------------
 * Rear kick 
 *---------------------------------------------------------------------------*/
class RearKickOneHit: public OneHit, public SkillDamage {
public:
    RearKickOneHit( Character *ch, Character *victim );
    
    virtual void init( );
    virtual void calcTHAC0( );
    virtual void calcDamage( );
    void damBase( );
};

RearKickOneHit::RearKickOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0 ), OneHit( ch, victim ),
              SkillDamage( ch, victim, gsn_rear_kick, 0, 0, DAMF_WEAPON )
{
}

void RearKickOneHit::init( )
{
    dam_type = attack_table[DAMW_HOOVES].damage;
    skill = 20 + ch->getSkill( sn );
}

void RearKickOneHit::damBase( )
{
    int ave, level = ch->getModifyLevel();
    
         if (level >= 100) ave = level - 12;
    else if (level >= 40)  ave = level - 10;
    else if (level >= 35)  ave = level -  8;
    else if (level >= 30)  ave = level -  6;
    else if (level >= 25)  ave = level -  5;
    else                   ave = level;
    
    dam = ave * skill / 100;                   // as weapon with skill bonus
}

void RearKickOneHit::calcDamage( )
{
    int level = ch->getModifyLevel();

    damBase( ); 
    damapply_class(ch, dam);
    damApplyPosition( );
    dam = (level < 50)
        ? (level / 10 + 1) * dam + level
        : (level / 10 ) * dam + level;
    damApplyDamroll( );

    OneHit::calcDamage( );
}

void RearKickOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_rear_kick->getEffective( ch ));
}

/*
 * 'rear kick' skill command
 */
SKILL_DECL( rearkick );
SKILL_APPLY( rearkick )
{
    if (!IS_AWAKE( victim ))
        return false;

    if (number_percent( ) > 33
        || number_percent( ) >= gsn_rear_kick->getEffective( victim )) 
    {
        gsn_rear_kick->improve( victim, false, ch );
        return false;
    }
   
    try {
        RearKickOneHit( victim, ch ).hit( );
        gsn_rear_kick->improve( victim, true, ch );
        
        yell_panic( victim, ch,
                    "Помогите! Кто-то ударил меня копытом по голове!",
                    "Помогите! %1$^C1 удари%1$Gло|л|ла меня копытом по голове!" );
    }
    catch (const VictimDeathException& e) {                                     
    }

    return true;
}

