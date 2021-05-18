/* $Id$
 *
 * ruffina, 2004
 */
#include "eating.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "clanreference.h"
#include "skillreference.h"
#include "desire.h"
#include "affect.h"

#include "raceflags.h"
#include "../anatolia/handler.h"
#include "magic.h"
#include "damage_impl.h"
#include "skill_utils.h"
#include "fight.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);
GSN(manacles);
GSN(poison);
DESIRE(full);
DESIRE(hunger);
RACE(cat);
RACE(felar);
RACE(fish);
RACE(mouse);
RACE(rat);
RACE(rodent);

static bool oprog_eat( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Eat", "C", ch );
    FENIA_NDX_CALL( obj, "Eat", "OC", obj, ch );
    return false;
}

COMMAND(CEat, "eat")
{
    Object *obj;
    DLString args = constArguments, arg;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
            ch->pecho("Съесть что?");
            return;
    }
    
    if ( ( obj = get_obj_carry( ch, arg ) ) == 0 )
    {
        Character *mob;

        if (( mob = get_char_room( ch, arg ) ) && mob->is_npc( )) {
            eatCarnivoro( ch, mob->getNPC( ) );
            return;
        }
        
        ch->pecho("У тебя нет этого.");
        return;
    }

    if ( !ch->is_immortal() )
    {
            if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
            {
                    ch->pecho("Это несъедобно.");
                    return;
            }

            if ( ch->isAffected(gsn_manacles)
                    && obj->item_type == ITEM_PILL )
            {
                    ch->pecho("Ты не можешь принимать снадобья в кандалах.");
                    return;
            }

            if(!ch->is_npc( ) 
                && ch->getClan( ) == clan_battlerager 
                && !ch->is_immortal( )
                && obj->item_type == ITEM_PILL)
            {
                ch->pecho("Воинам клана Ярости это ни к чему!");
                return;
            }


            if (!ch->is_npc( ))
                for (int i = 0; i < desireManager->size( ); i++)
                    if (!desireManager->find( i )->canEat( ch->getPC( ) ))
                        return;
    }

    if (get_wear_level(ch, obj) > ch->getRealLevel() && !ch->is_immortal() )
    {
            ch->pecho("Тебе надо подрасти, чтобы заглотить это.");
            return;
    }

    oldact("$c1 ест $o4.",  ch, obj, 0, TO_ROOM);
    oldact("Ты ешь $o4.", ch, obj, 0, TO_CHAR);
    if ( ch->fighting != 0 )
             ch->setWaitViolence( 3 );

    switch ( obj->item_type )
    {
    case ITEM_FOOD:
            eatFood( ch, obj->value0()*2, obj->value1()*2, obj->value3() );
            break;

    case ITEM_PILL:
            spell_by_item( ch, obj );
            break;
    }

    if ( ch->is_adrenalined() || ch->fighting )
    {
             ch->setWaitViolence( 2 );
    }

    if (oprog_eat( obj, ch ))
        return;

    extract_obj( obj );
}

void CEat::eatFood( Character *ch, int cFull, int cHunger, int cPoison )
{
    if ( !ch->is_npc() )
    {
        PCharacter *pch = ch->getPC( );
        
        desire_hunger->eat( pch, cHunger );
        desire_full->eat( pch, cFull );
    }

    if (cPoison != 0)
    {
            /* The food was poisoned! */
            Affect af;

            oldact("$c1 хватается за горло и задыхается.", ch, 0, 0, TO_ROOM);
            ch->pecho("Ты хватаешься за горло и задыхаешься.");

            af.bitvector.setTable(&affect_flags);
            af.type      = gsn_poison;
            af.level          = number_fuzzy( cFull / 2 );
            af.duration  = cFull;
            af.bitvector.setValue(AFF_POISON);
            affect_join( ch, &af );
    }
}

void CEat::eatCarnivoro( Character *ch, NPCharacter *mob )
{
    bool isCat, isRodent, isFish;
    bool wasPoisoned;
    int diff, dam, gain;
    
    if (ch->fighting) {
        ch->pecho("Сейчас ты сражаешься -- тебе не до охоты!");
        return;
    }
    
    isCat = (IS_SET( ch->form, FORM_FELINE ));
    isRodent = (mob->getRace( ) == race_mouse || mob->getRace( ) == race_rat || mob->getRace( ) == race_rodent);
    isFish = (mob->getRace( ) == race_fish);
    
    if (!isCat) {
        if (!isRodent && !isFish) {
            ch->pecho("Это животное не сделало тебе ничего плохого!");
        }
        else {
            oldact("Вообразив себя ко$gтом|том|шкой, $c1 пытается изловить и сожрать $C4, но опыта явно не хватает.", ch, 0, mob, TO_ROOM);
            oldact("На миг вообразив себя ко$gтом|том|шкой, ты пытаешься изловить и сожрать $C4, но опыта явно не хватает.", ch, 0, mob, TO_CHAR);
        }

        return;
    }
    else {
        if (!isRodent && !isFish) {
            oldact("$c1, похоже, приня$gло|л|ла $C4 за маааленького грызунчика.", ch, 0, mob, TO_ROOM);
            oldact("Это не грызун! Даже и не думай за $Y гоняться.", ch, 0, mob, TO_CHAR);
            return;
        }
    }
    

    if (mob->master) {
        oldact("$c1 с аппетитом клацает зубами при виде $C2.", ch, 0, mob, TO_ROOM);
        oldact("Ты с аппетитом клацаешь зубами при виде $C2.", ch, 0, mob, TO_CHAR);
        
        if (mob->master == ch) {
            oldact("$C1 с ужасом смотрит на $c4.", ch, 0, mob, TO_ROOM);
            oldact("$C1 с ужасом смотрит на тебя.", ch, 0, mob, TO_CHAR);
        }
        else if (mob->master->in_room == mob->in_room) {
            oldact("$C1 шустро прячется за спину хозя$gина|ина|йки!", mob->master, 0, mob, TO_ROOM);
            oldact("$C1 шустро прячется за твою спину!", mob->master, 0, mob, TO_CHAR);  
        }
        else
            oldact("$C1 вжимается в пол, закрыв глаза лапами.", ch, 0, mob, TO_ALL);
        
        return;
    }
    
    if (!ch->is_npc( ))
         for (int i = 0; i < desireManager->size( ); i++)
            if (!desireManager->find( i )->canEat( ch->getPC( ) ))
                 return;
   
    oldact("$c1 с громким мяуканьем вцепляется зубами и когтями в $C4!", ch, 0, mob, TO_ROOM);
    oldact("Ты с громким мяуканьем вцепляешься зубами и когтями в $C4!", ch, 0, mob, TO_CHAR);

    diff = max( 1, ch->getRealLevel( ) - mob->getRealLevel( ) );
    dam = diff * 10;
    gain = mob->getRealLevel( ); 
    wasPoisoned = (IS_AFFECTED(mob, AFF_POISON)); 

    if (dam >= mob->hit) {
        Object *obj, *obj_next;
        
        death_cry( mob, 99 );
        oldact("Ты ешь $C4.", ch, 0, mob, TO_CHAR);
        oldact("$c1 ест $C4.", ch, 0, mob, TO_ROOM);

        for (obj = mob->carrying; obj; obj = obj_next) {
            obj_next = obj->next_content;
            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
        }
        
        extract_char( mob );
        eatFood( ch, gain, gain, wasPoisoned );
    }
    else {
        RawDamage( ch, mob, DAM_PIERCE, dam ).hit( true );

        if (mob->position >= POS_FIGHTING)
            multi_hit( mob, ch );
    }
}

