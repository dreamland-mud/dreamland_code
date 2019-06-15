/* $Id$
 *
 * ruffina, 2009
 */
#include "arcadian_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "race.h"
#include "liquid.h"
#include "liquidflags.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "act.h"
#include "handler.h"
#include "effects.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

GSN(arcadian);
GSN(calm);
GSN(beer_armor);
GSN(sleep);
GSN(frenzy);
LIQ(water);

/*
 * LiquidWEBase
 */
bool LiquidWEBase::checkItemType( PCharacter *ch, Object *obj ) const
{
    if (obj->item_type != ITEM_DRINK_CON) {
        ch->pecho("%1$^O1 мало похож%1$Gе||а на емкость для жидкости.", obj);
        return false;
    }

    return true;
}
    
bool LiquidWEBase::checkVolume( PCharacter *ch, Object *obj ) const
{
    if (obj->value[1] == 0) {
        act("Слово эхом отозвалось в пустоте $o2.", ch, obj, 0, TO_CHAR);
        return false;
    }
    
    if (obj->value[1] > gsn_arcadian->getEffective( ch ) * 10) {
        act("В $o6 налито слишком много жидкости.", ch, obj, 0, TO_CHAR);
        return false;
    }
    
    return true;
}

bool LiquidWEBase::checkWater( PCharacter *ch, Object *obj ) const
{
    if (obj->value[2] != liq_water) {
        act("Это слово действует только на воду.", ch, 0, 0, TO_CHAR);
        return false;
    }
    
    return true;
}


/*
 * WaterToWineWE
 */
bool WaterToWineWE::run( PCharacter *ch, Object *obj ) const
{
    Liquid *wine;

    if (!(checkItemType( ch, obj )
            && checkVolume( ch, obj )
            && checkWater( ch, obj )))
        return false;

    wine = liquidManager->random( LIQF_WINE );
    obj->value[2] = wine->getIndex( );
    ch->pecho( "Вода в %O6 начинает шипеть и искриться, постепенно окрашиваясь {1%N5{2 цветом.{x", 
                obj, wine->getColor( ).c_str( ) );
    return true;
}

/*
 * WaterToBeerWE
 */
bool WaterToBeerWE::run( PCharacter *ch, Object *obj ) const
{
    Liquid *beer;

    if (!(checkItemType( ch, obj )
            && checkVolume( ch, obj )
            && checkWater( ch, obj )))
        return false;

    beer = liquidManager->random( LIQF_BEER );
    obj->value[2] = beer->getIndex( );
    ch->pecho( "Вода в %O6 начинает бурлить и пениться, постепенно окрашиваясь {1%N5{2 цветом.{x", 
                obj, beer->getColor( ).c_str( ) );
    return true;
}

/*
 * DrinkContainerWEBase
 */
DrinkContainerWEBase::DrinkContainerWEBase( )
{
}

void DrinkContainerWEBase::setupBehavior( PCharacter *ch, Object *obj ) const
{
    if (obj->behavior) 
        obj->behavior->unsetObj( );

    ArcadianDrinkBehavior::Pointer bhv( NEW );
    bhv->setEffectName( Pointer( this ) );
    bhv->setQuality( ch );
    bhv->setObj( obj );
    obj->behavior.setPointer( *bhv );
}

bool DrinkContainerWEBase::checkContainer( PCharacter *ch, Object *obj ) const
{
    ArcadianDrinkBehavior::Pointer bhv;

    if (!obj->behavior)
        return true;

    if (!( bhv = obj->behavior.getDynamicPointer<ArcadianDrinkBehavior>( ) )) {
        act("Повлиять на эту емкость у тебя не получится.", ch, obj, 0, TO_CHAR);
        return false;
    }

    if (bhv->isActive( )) {
        act("Жидкость в $o6 уже обладает необычными свойствами.", ch, obj, 0, TO_CHAR);
        return false;
    }

    return true;
}

bool DrinkContainerWEBase::goodQuality( ArcadianDrinkBehavior::Pointer bhv ) const
{
    return number_percent( ) < bhv->getQuality( );
}

bool DrinkContainerWEBase::goodVolume( int amount ) const
{
    return amount >= minEffectiveVolume.getValue( );
}

/*
 * WineContainerWEBase
 */
bool WineContainerWEBase::run( PCharacter *ch, Object *obj ) const
{
    Liquid *liq;
    
    if (!(checkItemType( ch, obj )
            && checkVolume( ch, obj )
            && checkContainer( ch, obj )))
        return false;
    
    liq = liquidManager->find( obj->value[2] );
    if (!liq->getFlags( ).isSet( LIQF_WINE )) {
        act("То, что налито в $o4, мало похоже на вино.", ch, obj, 0, TO_CHAR);
        return false;
    }

    setupBehavior( ch, obj );
    ch->pecho( "Разноцветные искры пробегают по поверхности %N2.", 
                liq->getShortDescr( ).c_str( ) );
    return true;
}

/*
 * BeerContainerWEBase
 */
bool BeerContainerWEBase::run( PCharacter *ch, Object *obj ) const
{
    Liquid *liq;

    if (!(checkItemType( ch, obj )
            && checkVolume( ch, obj )
            && checkContainer( ch, obj )))
        return false;
    
    liq = liquidManager->find( obj->value[2] );
    if (!liq->getFlags( ).isSet( LIQF_BEER )) {
        act("То, что налито в $o4, мало похоже на пиво.", ch, obj, 0, TO_CHAR);
        return false;
    }

    setupBehavior( ch, obj );
    ch->pecho( "Радужные блики играют в пенных пузырьках %N2.", 
                liq->getShortDescr( ).c_str( ) );
    return true;
}


/*
 * WineRefreshWE
 */
void WineRefreshWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
}

void WineRefreshWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, Character *victim, int amount ) const
{
}

void WineRefreshWE::onDrink( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    int level = ch->getModifyLevel( ) * bhv->getQuality( ) / 100;

    ch->hit += level * 5 + number_range( 60, 100 ); 
    ch->hit = min( ch->hit, ch->max_hit );
    update_pos( ch );

    act( "$c1 выглядит посвежевш$gим|им|ей.", ch, 0, 0, TO_ROOM );
    act( "Напиток освежает тебя.", ch, 0, 0, TO_CHAR );
}

/*
 * WineSleepWE
 */
void WineSleepWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
}

void WineSleepWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, Character *victim, int amount ) const
{
    Affect af;

    if (!goodQuality( bhv )
            || !goodVolume( amount )
            || !IS_AWAKE( victim )
            || victim->fighting
            || is_safe( ch, victim ))
    {
        return;
    }

    set_violent( ch, victim, false );

    act( "Ты чувствуешь внезапный неодолимый приступ сонливости.", victim, 0, 0, TO_CHAR );
    act( "$c1 зевает во всю пасть и засыпает.", victim, 0, 0, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = gsn_sleep;
    af.level     = ch->getModifyLevel( ) * bhv->getQuality( ) / 100;
    af.duration  = 1 + af.level / 50;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );
    
    victim->position = POS_SLEEPING;
    update_pos( victim );
}

void WineSleepWE::onDrink( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    ch->pecho( "Ты чувствуешь мимолетную сонливость." );
}

/*
 * WineAwakeWE
 */
void WineAwakeWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
}

void WineAwakeWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, Character *victim, int amount ) const
{
    int slevel = 0, scount = 0;
    
    if (!goodVolume( amount ) || IS_AWAKE( victim )) {
        victim->pecho( "Ты чувствуешь мимолетную бодрость." );
        return;
    }

    for (Affect *paf = victim->affected; paf; paf = paf->next)
        if (paf->where == TO_AFFECTS && IS_SET(paf->bitvector, AFF_SLEEP)) {
            slevel += paf->level;
            scount++;
        }

    if (scount)
        slevel /= scount;

    if (number_percent( ) > slevel * bhv->getQuality( ) / 100) {
        act_p( "Ты ворочаешься во сне.", victim, 0, 0, TO_CHAR, POS_SLEEPING );
        act( "$c1 ворочается во сне.", victim, 0, 0, TO_ROOM );
        return;
    }

    affect_bit_strip( victim, TO_AFFECTS, AFF_SLEEP );
    victim->position = POS_RESTING;
    update_pos( victim );

    act( "Сон как рукой сняло!", victim, 0, 0, TO_CHAR );
    act( "$c1 вздрагивает и просыпается.", victim, 0, 0, TO_ROOM );
}

void WineAwakeWE::onDrink( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    ch->pecho( "Напиток слегка взбадривает тебя." );
}


/*
 * WineCalmWE
 */
void WineCalmWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
}

void WineCalmWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, Character *victim, int amount ) const
{
    if (!goodQuality( bhv ) || !goodVolume( amount )) {
        act( "Ты чувствуешь мимолетное умиротворение.", victim, 0, 0, TO_CHAR );
        act( "$c1 на мгновение кажется более умиротворенн$gым|ым|ой.", victim, 0, 0, TO_ROOM );
        return;
    }
    
    act( "Ты чувствуешь удивительное спокойствие.", victim, 0, 0, TO_CHAR );
    act( "$c1 выглядит невероятно умиротворенн$gым|ым|ой и спокойн$gым|ым|ой.", victim, 0, 0, TO_ROOM );
    
    if (IS_AFFECTED( victim, AFF_BERSERK ))
        affect_bit_strip( victim, TO_AFFECTS, AFF_BERSERK );

    if (victim->isAffected( gsn_frenzy ))
        affect_strip( victim, gsn_frenzy );
    
    if (victim->fighting && IS_AWAKE(victim))
        stop_fighting( victim, false );
    
    if (!IS_AFFECTED( victim, AFF_CALM )) {
        Affect af;

        af.where     = TO_AFFECTS;
        af.type      = gsn_calm;
        af.level     = bhv->getQuality( );
        af.duration  = af.level / 4;
        af.location  = APPLY_HITROLL;
        af.modifier  = -5;
        af.bitvector = AFF_CALM;
        affect_to_char(victim, &af);

        af.location = APPLY_DAMROLL;
        affect_to_char(victim, &af);
    }
}

void WineCalmWE::onDrink( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    ch->pecho( "Ты чувствуешь мимолетное умиротворение." );
}


/*
 * BeerArmorWE
 */
void BeerArmorWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
}

void BeerArmorWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, Character *victim, int amount ) const
{
    Affect af;

    if (!goodVolume( amount ))
        return;

    af.where         = TO_AFFECTS;
    af.type      = gsn_beer_armor;
    af.level         = ch->getModifyLevel( ) * bhv->getQuality( ) / 100;
    af.duration  = 10 + af.level / 5;
    af.location  = APPLY_AC;
    
    if (victim->isAffected( gsn_beer_armor )) {
        act( "Пивная пленка на твоей коже становится прочнее.", victim, 0, 0, TO_CHAR );
        act( "Пивная пленка на коже $c2 становится прочнее.", victim, 0, 0, TO_ROOM );
        af.modifier = 0;
    }
    else {
        act( "Твоя кожа покрывается защитной пивной пленкой.", victim, 0, 0, TO_CHAR );
        act( "Кожа $c2 покрывается защитной пивной пленкой.", victim, 0, 0, TO_ROOM );
        af.modifier  = -(af.level * 3 / 2);
    }

    affect_join( victim, &af );
}

void BeerArmorWE::onDrink( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    ch->pecho( "Ты чувствуешь терпкий привкус." );
}

/*
 * BeerElementalWE
 */
void BeerElementalWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    Object *pool = get_obj_room_vnum( ch->in_room, OBJ_VNUM_POOL );

    if (ch->is_npc( ))
        return;

    if (goodVolume( amount ) && goodQuality( bhv )) {
        NPCharacter *mob = createElemental( ch->getPC( ), bhv );

        if (pool) {
            act( "$o1 начинает бурлить, рождая из себя $C4!", ch, pool, mob, TO_ALL );
            extract_obj( pool );
        }
        else
            act( "Из пенных брызг рождается $C1!", ch, 0, mob, TO_ALL );

        if (ch->fighting) 
            multi_hit( mob, ch->fighting );
    }
    else {
        if (pool)
            act( "$o1 яростно булькает, но ничего не происходит.", ch, pool, 0, TO_ALL );
    }
}

void BeerElementalWE::onPourOut( ArcadianDrinkBehavior::Pointer bhv, Character *ch, Character *victim, int amount ) const
{
}

void BeerElementalWE::onDrink( ArcadianDrinkBehavior::Pointer bhv, Character *ch, int amount ) const
{
    ch->pecho( "Напиток бурлит у тебя в животе!" );
}

NPCharacter * BeerElementalWE::createElemental( PCharacter *ch, ArcadianDrinkBehavior::Pointer bhv ) const
{
    NPCharacter *mob;
    int level = ch->getModifyLevel( ) * bhv->getQuality( ) / 100;

    mob = create_mobile( get_mob_index( MOB_VNUM_BEER_ELEMENTAL ) );

    for (int i = 0; i < stat_table.size; i++)
        mob->perm_stat[i] = ch->getCurrStat( i );

    mob->hit = mob->max_hit = 10 * ch->perm_hit + 40 * level;
    mob->mana = mob->max_mana = 100;
    mob->setLevel( level );
    
    for (int i = 0; i < 4; i++)
        mob->armor[i] = interpolate( mob->getRealLevel( ), 100, -100 );
            
    mob->damage[DICE_NUMBER] = level / 10 + 3;
    mob->damage[DICE_TYPE] = mob->damage[DICE_NUMBER];
    mob->damage[DICE_BONUS] = level / 2 + 10;
    mob->hitroll = level;

    SET_BIT(mob->affected_by, AFF_CHARM);
    char_to_room(mob,ch->in_room);
    mob->master = mob->leader = ch;
    
    return mob;
}

