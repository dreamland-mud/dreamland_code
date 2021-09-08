/* $Id$
 *
 * ruffina, 2009
 */
#include "khuzdul_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "object.h"
#include "affect.h"

#include "act.h"
#include "loadsave.h"
#include "wearloc_utils.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

GSN(ancient_rage);
GSN(enchant_weapon);
GSN(fireproof);
GSN(corrosion);
GSN(inaction);

bool FireproofWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;
    Affect af;

    af.bitvector.setTable(&extra_flags);
    af.type      = gsn_fireproof;
    af.level     = ch->getModifyLevel( );
    af.bitvector.setValue(ITEM_BURN_PROOF);

    for (obj = victim->carrying; obj; obj = obj->next_content) {
        if (obj->wear_loc == wear_none)
            continue;
            
        if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF))
            continue;

        af.duration  = number_range( af.level, 200 );
        affect_to_obj( obj, &af);
    }

    oldact("{CОбмундирование на $c6 вспыхивает ослепительным блеском.{x", victim, 0, 0, TO_ROOM );
    oldact("{CТвое обмундирование вспыхивает ослепительным блеском.{x", victim, 0, 0, TO_CHAR );
    return true;
}

bool EnchantWeaponWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;
    Affect af;

    obj = get_eq_char( victim, wear_wield );

    if (!obj) {
        if (ch != victim) {
            victim->pecho( "Ты чувствуешь легкое покалывание ладони." );
            oldact("Слово не достигло цели - $C1 ничем не вооруж$Gено|ен|ена.", ch, 0, victim, TO_CHAR );
        }
        else {
            oldact("Слово не достигло цели - ты ничем не вооруж$gено|ен|ена.", ch, 0, 0, TO_CHAR );
        }

        return false;
    }
    
    affect_enchant( obj );

    af.type      = gsn_enchant_weapon;
    af.level     = ch->getModifyLevel( );
    af.duration  = number_range( af.level, 200 );
    af.modifier  = number_range( 1 + af.level / 10, 
                                 1 + af.level / 7 );

    af.location = APPLY_DAMROLL;
    affect_enhance( obj, &af );

    af.location = APPLY_HITROLL;
    affect_enhance( obj, &af );

    victim->hitroll += af.modifier;
    victim->damroll += af.modifier;

    oldact("{CСекреты кузнецов древности преображают $o4!{x", ch, obj, 0, TO_ALL );
    return true;
}

bool BerserkWE::run( PCharacter *ch, Character *victim ) const
{
    Affect af;
    
    if ( IS_AFFECTED(ch,AFF_CALM) || victim->isAffected( gsn_inaction ) )
    {
        victim->pecho( "Ты слишком миролюбив{Sfа{Sx для древней ярости." );
        victim->recho("%1$^C1 слишком миролюбив%1$Gо||а для древней ярости.", victim);
        return false;
    } 

    if (victim->isAffected( gsn_ancient_rage )) {
        victim->pecho("Пламя древней ярости уже горит в тебе.");
        victim->recho("Пламя древней ярости уже горит в %C6.", victim);
        return false;
    }

    af.type         = gsn_ancient_rage;
    af.level         = ch->getModifyLevel( );
    af.duration         = number_fuzzy( af.level / 8 );
    af.location         = (number_bits( 1 ) ? APPLY_HITROLL : APPLY_DAMROLL);
    af.modifier = max( 1, number_range( af.level / 6, af.level / 5 ) );

    victim->pecho( "{CПламя древней ярости вспыхивает в тебе!{x" );
    victim->recho( "{CПламя древней ярости вспыхивает в %C6!{x", victim);
    affect_join( victim, &af );
    return true;
}

bool MendingWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;

    for (obj = victim->carrying; obj; obj = obj->next_content) {
        if (obj->wear_loc == wear_none)
            continue;

        obj->condition += number_range( 30, 50 );
        obj->condition = min( 100, obj->condition );
        affect_strip(obj, gsn_corrosion, true);
    }

    oldact("{CСекреты древних кузнецов улучшают облик твоего обмундирования.{x", victim, 0, 0, TO_CHAR );
    oldact("{CСекреты древних кузнецов улучшают облик обмундирования $c2.{x", victim, 0, 0, TO_ROOM );
    return true;
}


