/* $Id$
 *
 * ruffina, 2004
 */
#include "moneychanger.h"
#include "npcharacter.h"
#include "dreamland.h"
#include "handler.h"
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "def.h"


void MoneyChanger::bribe( Character *vch, int gold, int silver )
{        
    int a_gold = 0, a_silver = 0;
    int o_gold = vch->gold, o_silver = vch->silver;

    if (!ch->can_see( vch )) {
        tell_dim( vch, ch, "Я не торгую с тем, кого не вижу. Забирай свои деньги." );
        act( "$c1 роняет на пол кучку монет.", ch,0,0,TO_ROOM);
        ch->silver         -= silver;
        ch->gold        -= gold;
        obj_to_room( create_money( gold, silver ), vch->in_room );
        return;
    }

    if ( silver > 0 ) {
        a_gold = 95 * silver / 10000;
        a_silver = silver * 95 / 100 - a_gold * 100;

        if (a_gold < 1)
        {
            tell_dim( vch, ch, "Извини, но эта сумма слишком мала для обмена." );
            interpret_raw( ch, "give", "%d silver %s", silver, vch->getNameP( ) );
            return;
        }

        ch->silver -= silver;
        ch->silver += silver * 5 / 100 ;
        dreamland->putToMerchantBank( ch->silver / 100 );
        dreamland->save(false);
        ch->silver %= 100;
    }
    else {
        a_silver = 95 * gold;
        ch->gold -= gold;
        
        dreamland->putToMerchantBank( ch->gold );
        dreamland->save(false);
        ch->gold = 0;
    }
  
    vch->gold += a_gold;
    vch->silver += a_silver;
        
    if (vch->getCarryWeight( ) + a_gold + a_silver / 10 > vch->canCarryWeight( ))
    {
        vch->gold = o_gold;
        vch->silver = o_silver;
        
        act_p( "$c1 пытается дать тебе деньги, но ты не можешь их удержать.",
                ch, 0, vch, TO_VICT,POS_RESTING );
        act( "$c1 роняет на пол кучку монет.", vch,0,0,TO_ROOM);
        obj_to_room( create_money( a_gold, a_silver ), vch->in_room );
    }
    else if ( !ch->can_see( vch ) )
    {
        vch->gold = o_gold;
        vch->silver = o_silver;

        act_p( "$c1 пытается дать тебе деньги, но промахивается и роняет их на пол.",
                ch, 0, vch, TO_VICT,POS_RESTING );
        act( "$c1 роняет на пол кучку монет.", ch,0,0,TO_ROOM);
        obj_to_room( create_money( a_gold, a_silver ), vch->in_room );
    }
    else
    {
        vch->gold = o_gold;
        vch->silver = o_silver;
        ch->gold += a_gold;
        ch->silver += a_silver;
        
        if ( a_gold > 0 )
        {
            interpret_raw( ch, "give", "%d gold %s", a_gold, vch->getNameP( ) );
        }
        
        if ( a_silver > 0 )
        {
            interpret_raw( ch, "give", "%d silver %s", a_silver, vch->getNameP( ) );
        }

        tell_dim( vch, ch, "Спасибо, приходи еще!" );
    }
}

