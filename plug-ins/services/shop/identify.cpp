#include <cmath>

#include "skillreference.h"
#include "commandtemplate.h"
#include "spell.h"
#include "spelltarget.h"
#include "pcharacter.h"
#include "core/object.h"
#include "attract.h"
#include "loadsave.h"

#include "act.h"
#include "merc.h"
#include "def.h"

GSN(identify);

NPCharacter * find_mob_with_act( Room *room, bitstring_t act );
bool can_afford(Character *ch, int gold, int silver, int number);
void deduct_cost(Character *ch, int cost);

CMDRUN( identify )
{
    Object *obj;
    Character *rch;
    int cost = 20;
    DLString arg = constArguments;

    if ( ch->is_npc( ) ) {
        ch->pecho("У тебя же лапки!!!");
        return;
    }
    
    if (arg.empty()) {
        ch->pecho("Что именно ты хочешь опознать?");
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg.c_str() ) ) == 0 )
    {
       ch->pecho("У тебя нет этого.");
       return;
    }

    rch = find_mob_with_act( ch->in_room, ACT_SAGE );

    if (!rch)
    {
       ch->pecho("Тут никто ничего толкового не скажет об этой вещи.");
       return;
    }
   
    int remorts = ch->getPC()->getRemorts( ).size( );
    //add guru checks?
    if ( remorts == 0) {
        cost = round ((ch->getRealLevel( ) - cost) * 0.66);
        cost = URANGE (0, cost, 20);
    }


    if (ch->is_immortal( )) {
        oldact("$c1 смотрит на тебя!\n\r", rch, obj, ch, TO_VICT);
    }
    else if (!can_afford(ch, cost, 0, 1)) {
        tell_fmt("У тебя даже %3$d золот%3$Iого|ых|ых нету, чтобы мне заплатить!", ch, rch, cost );
        return;
    }
    else {
       deduct_cost(ch, cost * 100);
       if ( cost > 0 ) ch->pecho("Твой кошелек становится значительно легче.");
    }

    oldact("$c1 изучающе смотрит на $o4.", rch, obj, 0, TO_ROOM);

    bool hadMagic = ch->detection.isSet(DETECT_MAGIC);
    ch->detection.setBit(DETECT_MAGIC);

    if (gsn_identify->getSpell( ))
        gsn_identify->getSpell( )->run( ch, SpellTarget::Pointer(NEW, obj), 0 );

    if (!hadMagic)
        ch->detection.removeBit(DETECT_MAGIC);
}



