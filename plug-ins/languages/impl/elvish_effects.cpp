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
            act("Ты не страдаешь врожденной уязвимостью к железу.", ch, 0, 0,TO_CHAR);
        else
            act("%2$^C1 не страдает врожденной уязвимостью к железу.", ch, victim, 0,TO_CHAR);
        
        return false;
    }
 
    af.bitvector.setTable(&res_flags);
    af.type      = gsn_secret_of_sidhe;
    af.level     = ch->getModifyLevel( );
    af.duration  = af.level / 6;
    af.bitvector.setValue(RES_IRON);
    
    affect_join( victim, &af );
   
    act("{CСекретное знание Сидхов теперь защищает %C4.{x", victim, 0, 0,TO_ROOM);
    act("{CСекретное знание Сидхов теперь защищает тебя.{x", victim, 0, 0,TO_CHAR);
    return true;
}

bool BlessEquipWE::run( PCharacter *ch, Object *obj ) const
{

    if (IS_OBJ_STAT(obj, ITEM_BLESS)){
        ch->pecho("%1$^O1 уже благословле%1$Gно|н|на|ны.", obj);
        return false;
    }

    Affect af;

    af.bitvector.setTable(&extra_flags);
    af.type      = gsn_bless;
    af.location = APPLY_SAVES;
    af.bitvector.setValue(ITEM_BLESS);
    af.level     = ch->getModifyLevel( );
    af.duration = number_range( 100, 600 );
    af.modifier = -1 * number_range( 1, 3 );
    
    affect_to_obj( obj, &af);

    if (obj->carried_by && obj->wear_loc->givesAffects())
        affect_modify(obj->carried_by, &af, true);

    act("{C%3$O1 на мгновение загорается огнем нездешних звёзд.{x", ch, 0, obj,TO_ALL);
    return true;
}

bool RestoringWE::run( PCharacter *ch, Character *victim ) const
{
    victim->hit  = min( (int)victim->max_hit,  victim->hit  + victim->max_hit  / 2 );
    victim->mana = min( (int)victim->max_mana, victim->mana + victim->max_mana / 2 );
    victim->move = victim->max_move;
    update_pos( victim );

    if (ch != victim)
        act("{CТаинственное мелодичное слово пронизывает %2$C4 теплом.{x", ch, victim, 0,TO_CHAR);

    victim->pecho( "{CТаинственное мелодичное слово пронизывает тебя теплом.{x" );
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

