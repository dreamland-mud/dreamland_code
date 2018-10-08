/* $Id$
 *
 * ruffina, 2004
 */
#include "druidic_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"
#include "affecthandlertemplate.h"

#include "pcharacter.h"
#include "object.h"
#include "affect.h"

#include "follow_utils.h"
#include "fight.h"
#include "act.h"
#include "loadsave.h"
#include "mercdb.h"
#include "def.h"

GSN(animal_spirits);
GSN(snake_spirit);
GSN(fox_spirit);
GSN(boar_spirit);
GSN(wolverine_spirit);
GSN(forest_faery_spirit);
GSN(forest_troll_spirit);
GSN(dryad_spirit);
GSN(druidic);

#if 0
bool AnimalSpiritWE::run( PCharacter *ch, Object *obj ) const
{
    AnimalSpiritComponent::Pointer comp;
    
    if (!obj->behavior
        || !(comp = obj->behavior.getDynamicPointer<AnimalSpiritComponent>( )))
    {
        ch->println("Это слово нужно произносить на специальный компонент.");
        return false;
    }
    
    comp->runEffect( ch );
    return true;
}

bool AnimalSpiritComponent::applyNegativeAffect( PCharacter *ch, Affect *paf ) const
{
    bool rc = false;
    DruidSpiritAffectHandler::Pointer daf = getAffectHandler( paf );
    
    if (!daf)
        return rc;

    ch->pecho( daf->msgSelf, ch, obj );

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (!is_same_group( ch, rch )
              && rch->fighting 
              && is_same_group( ch, rch->fighting )) 
        {
            if (!rch->isAffected( paf->type )) {
                rch->pecho( daf->msgVict, ch, obj, rch );
                affect_to_char( rch, paf );
                rc = true;
            }
        } else {
            rch->pecho( daf->msgOther, ch, obj, rch );
        }

    return rc;
}

bool AnimalSpiritComponent::applyPositiveAffect( PCharacter *ch, Affect *paf ) const
{
    bool rc = false;
    DruidSpiritAffectHandler::Pointer daf = getAffectHandler( paf );

    if (!daf)
        return rc;

    ch->pecho( daf->msgSelf, ch );

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (is_same_group( ch, rch )) {
            if (!rch->isAffected( paf->type )) {
                if (rch != ch)
                    rch->pecho( daf->msgVict, ch, rch );
                affect_to_char( rch, paf );
                rc = true;
            }
        } else {
            rch->pecho( daf->msgOther, ch, rch );
        }

    return rc;
}

DruidSpiritAffectHandler::Pointer AnimalSpiritComponent::getAffectHandler( Affect *paf )
{
    DruidSpiritAffectHandler::Pointer daf;

    if (!paf->type->getAffect( ) 
        || !(daf = paf->type->getAffect( ).getDynamicPointer<DruidSpiritAffectHandler>( )))
    {
        bug("druids: invalid affect handler for %s", paf->type->getName( ).c_str( ));
    }

    return daf;
}

bool AnimalSpiritComponent::checkComponent( PCharacter *ch, Object *comp ) const
{
    return true;
}

int AnimalSpiritComponent::getChance( PCharacter *ch ) const
{
    return max( 1, gsn_druidic->getEffective( ch ) ); 
}

void SnakeSpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_snake_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = af.level / 6 + number_range( 5, 10 );
    af.location = APPLY_MANA_GAIN;

    if (good) {
        af.modifier = number_range( 100, 200 ) + getChance( ch );
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = 100 - getChance( ch ) / number_range( 2, 3 );
        rc = applyNegativeAffect( ch, &af );
    }

    af.location = APPLY_LEVEL;
    af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}

void FoxSpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_fox_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = number_range( 10, 15 );
    af.location = APPLY_DEX;
    af.modifier = number_range( 1, getChance( ch ) / 20 );

    if (good) {
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = -af.modifier;
        rc = applyNegativeAffect( ch, &af );
    }

    af.location = APPLY_LEVEL;
    af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}

void BoarSpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_boar_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = af.level / 10 + number_range( 2, 5 );
    af.location = APPLY_HIT;
    af.modifier = af.level * getChance( ch ) / 40;

    if (good) {
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = -af.modifier;
        rc = applyNegativeAffect( ch, &af );
    }

    af.location = APPLY_LEVEL;
    af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}

void WolverineSpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_wolverine_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = af.level / 10 + number_range( 5, 10 );
    af.location = APPLY_HITROLL;
    af.modifier = af.level / (20 - getChance( ch ) / 10);

    if (good) {
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = -af.modifier;
        rc = applyNegativeAffect( ch, &af );
    }

    af.location = APPLY_LEVEL;
    af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}

void ForestFaerySpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_forest_faery_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = af.level / 15 + number_range( 2, 5 );
    af.location = APPLY_LEVEL;
    af.modifier = 1 + af.level / 33;

    if (good) {
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = -af.modifier;
        rc = applyNegativeAffect( ch, &af );
    }
    
    if (good)
        af.modifier = -af.modifier;
    else
        af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}

void ForestTrollSpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_forest_troll_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = af.level / 6 + number_range( 5, 10 );
    af.location = APPLY_DAMROLL;
    af.modifier = af.level * getChance( ch ) / 500;

    if (good) {
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = -af.modifier;
        rc = applyNegativeAffect( ch, &af );
    }
    
    af.location = APPLY_LEVEL;
    af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}

void DryadSpiritComponent::run( PCharacter *ch )
{
    bool rc;
    Affect af;

    af.type = gsn_dryad_spirit;
    af.level = ch->getModifyLevel( );
    af.pcOwner = ch->getName( );
    af.duration = af.level / 10 + number_range( 5, 10 );
    af.location = APPLY_SAVES_SPELL;
    af.modifier = af.level / (20 - getChance( ch ) / 10);

    if (good) {
        rc = applyPositiveAffect( ch, &af );
    }
    else {
        af.modifier = -af.modifier;
        rc = applyNegativeAffect( ch, &af );
    }

    af.location = APPLY_LEVEL;
    af.modifier = -1;
    if (rc) affect_join_char( ch, &af );
}
#endif

/*
 * Remove the spirit affects when either druid leaves the group
 * or affected character does.
 */
void DruidSpiritAffectHandler::stopfol( Character *ch, Affect *paf )
{
    Character *druid, *leader;
    
    /* only the last affect will do the job */
    if (paf->next && paf->next->type == paf->type)
        return;

    /* i was just a follower, not in some group */
    if (!(leader = ch->leader))
        return;
    
    /* druid left long ago.. */
    if (!(druid = paf->getOwner( ))) {
        affect_strip( ch, paf->type );
        return;
    }
    
    /* i'm the caster, cleanup my former groupmates */
    if (ch == druid) {
        GroupMembers oldgroup, newgroup;

        oldgroup = group_members_world( ch );
        ch->leader = NULL;
        newgroup = group_members_world( ch );
        ch->leader = leader;

        for (GroupMembers::iterator g = oldgroup.begin( ); g != oldgroup.end( ); g++)
            if (!newgroup.count( *g ))
                affect_strip( *g, paf->type );
    }
    else { /* i'm an ordinar group member */
        ch->leader = NULL;

        if (!is_same_group( ch, druid )) 
            affect_strip( ch, paf->type );

        ch->leader = leader;
    }
}

/*
 * periodically check if we are in the same group with the caster
 */
void DruidSpiritAffectHandler::update( Character *ch, Affect *paf )
{
    Character *druid;

    /* only the last affect will do the job */
    if (paf->next && paf->next->type == paf->type)
        return;
    
    /* clean the affect if i'm no longer member of caster group */
    if (!( druid = paf->getOwner( ) )
        || !is_same_group( ch, druid ))
    {
        affect_strip( ch, paf->type );
    }
}

