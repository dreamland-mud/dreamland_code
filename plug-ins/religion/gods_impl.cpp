/* $Id: gods_impl.cpp,v 1.1.2.5 2010-09-01 21:20:46 rufina Exp $
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
#include "gods_impl.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "skillcommand.h"

#include "handler.h"
#include "act.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "fight.h"
#include "magic.h"
#include "gsn_plugin.h"
#include "immunity.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"


/*
 * The only religion mark behavior still implemented via behaviors. Waiting on Fenia interface
 * to OneHit classes.
 */

class TricksterGodOneHit: public SkillWeaponOneHit {
public:
    TricksterGodOneHit( Character *ch, Character *victim );
    
    virtual void calcDamage( );
};

TricksterGodOneHit::TricksterGodOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_knife )
{
}

void TricksterGodOneHit::calcDamage( )
{
    damBase( );
    damApplyEnhancedDamage( );
    damApplyPosition( );
    dam = (ch->getModifyLevel( ) / 30 + 1) * dam + ch->getModifyLevel( );
    damApplyDamroll( );
    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}

void ErevanGod::tattooFight( Object *obj, Character *ch ) const 
{
    Character *victim = ch->fighting;
    if (!victim)
        return;

    // Chance 1 in 8 of getting a heal. 
    if (ch->hit < ch->max_hit && chance(12)) {
        ch->pecho("{C%^O1 загорается голубым светом.{x", obj);
        spell(gsn_heal, ch->getModifyLevel(), ch, ch, FSPELL_NOTRIGGER);
        return; 
    }

    if (!IS_AFFECTED(ch, AFF_HASTE) && chance(50)) {
        spell(gsn_haste, ch->getModifyLevel(), ch, ch, FSPELL_NOTRIGGER );
        ch->pecho("{WТы внезапно ощущаешь повышенную активность!{x");
        return;
    }

    // Chance 1 in 10-12 of casting a spell (disregard haste).
    if (chance(10)) {
        spell(gsn_colour_spray, ch->getModifyLevel(), ch, victim, FSPELL_NOTRIGGER);
        return;
    }

    // Chance 1 in 25 of additional hit.
    if (chance(15)) {
        try {
            TricksterGodOneHit thit(ch, victim);
            oldact("{CЭреван Илесир внезапно вселяется в тебя.{x", ch, 0, 0, TO_CHAR);
            oldact("{CЭреван Илесир внезапно вселяется в $c4.{x", ch, 0, 0, TO_ROOM);
            thit.hit();
            do_yell(victim, "Помогите! Эреван Илесир пыряет меня ножом!");
        }
        catch (const VictimDeathException &e) {
        }
    }
}

