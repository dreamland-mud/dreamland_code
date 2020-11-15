/* $Id$
 *
 * ruffina, 2009
 */
#include "elvish_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "object.h"
#include "affect.h"
#include "race.h"

#include "fight.h"
#include "act.h"
#include "loadsave.h"
#include "wearloc_utils.h"
#include "mercdb.h"
#include "def.h"

GSN(bless);
GSN(secret_of_sidhe);

bool ResistIronWE::run( PCharacter *ch, Character *victim ) const
{
    Affect af;

    if (!victim->getRace( )->getVuln( ).isSet( VULN_IRON )) {
        if (victim == ch)
            act("Ты не страдаешь врожденной уязвимостью к железу.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 не страдает врожденной уязвимостью к железу.", ch, 0, victim, TO_CHAR);
        
        return false;
    }
 
    af.where     = TO_RESIST;
    af.type      = gsn_secret_of_sidhe;
    af.level     = ch->getModifyLevel( );
    af.duration  = af.level / 6;
    af.bitvector = RES_IRON;
    
    affect_join( victim, &af );
   
    act("{CСекретное знание Сидхов теперь защищает $c4.{x", victim, 0, 0, TO_ROOM);
    act("{CСекретное знание Сидхов теперь защищает тебя.{x", victim, 0, 0, TO_CHAR);
    return true;
}

bool BlessEquipWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;
    Affect af;

    af.where     = TO_OBJECT;
    af.type      = gsn_bless;
    af.location         = APPLY_SAVES;
    af.bitvector = ITEM_BLESS;
    af.level     = ch->getModifyLevel( );
    
    for (obj = victim->carrying; obj; obj = obj->next_content) {
        if (obj->wear_loc == wear_none)
            continue;
        
        if (IS_OBJ_STAT(obj, ITEM_BLESS))
            continue;
        
        af.modifier = -1 * number_range( 1, 3 );
        af.duration = number_range( 6 + af.level / 2, 200 );
        affect_to_obj( obj, &af);
        affect_modify(victim, &af, true);
    }

    act( "{CОбмундирование на $c6 на мгновение загорается священным огнем.{x", victim, 0, 0, TO_ROOM );
    act( "{CТвое обмундирование на мгновение загорается священным огнем.{x", victim, 0, 0, TO_CHAR );
    return true;
}

bool RestoringWE::run( PCharacter *ch, Character *victim ) const
{
    victim->hit  = min( (int)victim->max_hit,  victim->hit  + victim->max_hit  / 2 );
    victim->mana = min( (int)victim->max_mana, victim->mana + victim->max_mana / 2 );
    victim->move = victim->max_move;
    update_pos( victim );

    if (ch != victim)
        act( "{CТаинственное мелодичное слово пронизывает $C4 теплом.{x", ch, 0, victim, TO_CHAR );

    victim->println( "{CТаинственное мелодичное слово пронизывает тебя теплом.{x" );
    return true;
}

/*
 * ElvishEffectsPlugin
 */
void ElvishEffectsPlugin::initialization( )
{
    Class::regMoc<ResistIronWE>( );
    Class::regMoc<BlessEquipWE>( );
    Class::regMoc<RestoringWE>( );
}

void ElvishEffectsPlugin::destruction( )
{
    Class::unregMoc<ResistIronWE>( );
    Class::unregMoc<BlessEquipWE>( );
    Class::unregMoc<RestoringWE>( );
}

