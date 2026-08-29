/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "defaultwearlocation.h"
#include "wearloc_codes.h"
#include "player_utils.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "core/behavior/behavior_utils.h"
#include "skillreference.h"
#include "room.h"
#include "affect.h"
#include "character.h"
#include "core/object.h"

#include "damage_impl.h"
#include "damageflags.h"
#include "skill_utils.h"
#include "../loadsave/behavior_utils.h"
#include "interp.h"
#include "save.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "material.h"
#include "l10n.h"

WEARLOC(none);
GSN(spellbane);

/*-------------------------------------------------------------------
 * DefaultWearlocation
 *------------------------------------------------------------------*/
DefaultWearlocation::DefaultWearlocation( ) 
                      : itemType( 0, &item_table ),
                        itemWear( 0, &wear_flags ),
                        needRib( true )

{
}

const DLString & DefaultWearlocation::getName( ) const
{
    return Wearlocation::getName( );
}

bool DefaultWearlocation::isValid( ) const
{
    return true;
}

bool DefaultWearlocation::givesAffects() const
{
    return true;
}

void DefaultWearlocation::setName( const DLString &name )
{
    this->name = name;
}

void DefaultWearlocation::loaded( )
{
    wearlocationManager->registrate( Pointer( this ) );
}

void DefaultWearlocation::unloaded( )
{
    wearlocationManager->unregistrate( Pointer( this ) );
}

/*-------------------------------------------------------------------
 * misc. utils 
 *------------------------------------------------------------------*/
void DefaultWearlocation::saveDrops( Character *ch )
{
    if (ch->is_npc( )
            && !IS_CHARMED(ch)
            && ch->in_room != 0)
    {
        save_mobs( ch->in_room );
    }
}

/*-------------------------------------------------------------------
 * matches 
 *------------------------------------------------------------------*/
bool DefaultWearlocation::matches( Character *ch )
{
    return needRib && ch->getWearloc( ).isSet( this );
}

bool DefaultWearlocation::matches( Object *obj )
{
    if (obj->item_type == itemType.getValue( ))
        return true;
        
    if (obj->can_wear( itemWear.getValue( ) ))
        return true;

    return false;
}

/*-------------------------------------------------------------------
 * reset 
 *------------------------------------------------------------------*/
void DefaultWearlocation::reset( Object *obj )
{
    Character *ch = obj->carried_by;

    triggersOnEquip( ch, obj );

    affectsOnEquip( ch, obj );
}

/*-------------------------------------------------------------------
 * find 
 *------------------------------------------------------------------*/
Object * DefaultWearlocation::find( Character *ch )
{
    Object *obj;

    for (obj = ch->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc == this)
            return obj;

    return 0;
}

/*-------------------------------------------------------------------
 * equip 
 *------------------------------------------------------------------*/
bool DefaultWearlocation::equip( Object *obj )
{
    Character *ch = obj->carried_by;

    if (!canEquip( ch, obj ))
        return false;
    
    if (find( ch ))
        LogStream::sendError( ) << "Equip_char: " << ch->getNameC() << " already equipped (" << getName( ) << ")." << endl;
    
    obj->wear_loc.assign( this );
    
    triggersOnEquip( ch, obj );

    affectsOnEquip( ch, obj );
    
    saveDrops( ch );

    return true;
}

// Rebuild the wearer's eqAffects cache from scratch: the union of grant-typed
// affects (a real skill type, index > 0) on every worn item's PROTOTYPE affect
// list. Cheap -- worn items x their few affects -- and always correct, so it runs
// after any equip or unequip (the only events that change the worn set).
// Perma-affects derived model, #2758 phase 2.
//
// Two deliberate restrictions, both correctness-critical (fable review
// 2026-08-29):
//  * givesAffects() only -- skips wear_none and the flavor slots (hair/personal/
//    tail), whose items apply nothing; scanning them would let a grant item parked
//    in the purchasable personal slot confer its grant for free.
//  * pIndexData->affected only -- BOTH grant channels (<grant> in area XML and
//    oedit addgrant) write PROTOTYPE affects. Instance affects legitimately carry
//    a real skill type as long-standing bookkeeping (WeaponGenerator types every
//    generated weapon's affects with its weapon-class skill; ruler items; ~17
//    runObj spell affects). Those are NOT grants; unioning them would make wearers
//    isAffected(katana/bless/curse/...) and break real gates (katana craft
//    cooldown, "already blessed", etc.). So the instance list is never scanned.
static void rebuild_eq_affects( Character *ch )
{
    ch->eqAffects.clear( );

    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content) {
        if (!obj->wear_loc->givesAffects( ))
            continue;

        for (auto &paf: obj->pIndexData->affected) {
            Skill *g = paf->type.getElement( );
            if (g != 0 && g->getIndex( ) > 0)
                ch->eqAffects.set( g->getIndex( ) );
        }
    }
}

void DefaultWearlocation::affectsOnEquip( Character *ch, Object *obj )
{
    for (auto &paf: obj->pIndexData->affected)
        affect_modify( ch, paf, true );

    for (auto &paf: obj->affected)
        affect_modify( ch, paf, true );

    rebuild_eq_affects( ch );
}

static bool oprog_cant_equip( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "CantEquip", "C", ch )
    FENIA_NDX_CALL( obj, "CantEquip", "OC", obj, ch )
    return false;
}

static bool oprog_equip( Object *obj, Character *ch )
{
    if (behavior_trigger(obj, "Equip", "OC", obj, ch))
        return true;

    FENIA_CALL( obj, "Equip", "C", ch )
    FENIA_NDX_CALL( obj, "Equip", "OC", obj, ch )
    BEHAVIOR_VOID_CALL( obj, equip, ch )
    return false;
}

static bool oprog_wear( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Wear", "C", ch )
    FENIA_NDX_CALL( obj, "Wear", "OC", obj, ch )
    BEHAVIOR_VOID_CALL( obj, wear, ch )
    return false;
}

static bool oprog_remove( Object *obj, Character *ch )
{
    if (behavior_trigger(obj, "Remove", "OC", obj, ch))
        return true;

    FENIA_CALL( obj, "Remove", "C", ch )
    FENIA_NDX_CALL( obj, "Remove", "OC", obj, ch )
    BEHAVIOR_VOID_CALL( obj, remove, ch )
    return false;
}

bool DefaultWearlocation::canEquip( Character *ch, Object *obj )
{
    if (obj->isAntiAligned( ch )) {
        echo_master(ch, _("Твоя натура не позволяет тебе носить %O4."), obj);
        ch->recho(_("%1$^C1 безуспешно пытается надеть %2$O4."), ch, obj);
        return false;
    }
    
    if (obj->getRealShortDescr( ).empty() && obj->wasAntiAligned( ch )) {
        echo_master(ch, _("Твоя натура все еще не позволяет тебе носить %O4, сперва нужно изменить внешний вид этой вещи."), obj);
        ch->recho(_("%1$^C1 безуспешно пытается надеть %2$O4."), ch, obj);
        return false;
    }

    if (IS_SET( obj->extra_flags, ITEM_MAGIC ) && ch->isAffected( gsn_spellbane )) {
        int dam = URANGE( 0, ch->hit - 1, ch->max_hit / 5 );
        oldact(_("Магия $o2 аннигилирует с твоим спеллбаном!"), ch, obj, 0, TO_CHAR);
        oldact(_("Магия $o2 аннигилирует со спеллбаном $c2!"), ch, obj, 0, TO_ROOM);
        SkillDamage( ch, ch, gsn_spellbane, DAM_NEGATIVE, dam, DAMF_MAGIC ).hit( false );
        interpret_raw( ch, "cb", "Меня ударило магической вещью!" );
        return false;
    }

    int badMaterials = material_types_forbidden( ch );
    if (badMaterials != 0 && material_is_typed( obj, badMaterials )) {
        echo_master(ch, _("Ты не сможешь надеть %O4: друиды не носят ничего металлического."), obj);
        ch->recho(_("%1$^C1 безуспешно пытается надеть %2$O4."), ch, obj);
        return false;    
    }
  
    if (!obj_owner_enforce( obj, ch ))
        return false;

    if (obj->behavior && !obj->behavior->canEquip( ch ))
        return false;
    
    if (oprog_cant_equip( obj, ch ))
        return false;

    return true;
}

void DefaultWearlocation::triggersOnEquip( Character *ch, Object *obj )
{
    oprog_equip( obj, ch );

    switch (obj->item_type) {
    case ITEM_LIGHT:
        if (ch->in_room != 0)
            ch->in_room->updateLight();
        break;

    case ITEM_ARMOR:
        for (int i = 0; i < 4; i++)
            ch->armor[i] -= armorCoef * obj->valueByIndex(i);
        break;
    }
}

/*-------------------------------------------------------------------
 * unequip 
 *------------------------------------------------------------------*/
void DefaultWearlocation::unequip( Object *obj )
{
    Character *ch = obj->carried_by;

    obj->wear_loc.assign( wear_none );
    
    affectsOnUnequip( ch, obj );
    
    triggersOnUnequip( ch, obj );

    saveDrops( ch );
}

void DefaultWearlocation::affectsOnUnequip( Character *ch, Object *obj )
{
    for (auto &paf: obj->pIndexData->affected) {
        affect_modify( ch, paf, false );
        affect_check(ch, paf);
    }

    for (auto &paf: obj->affected) {
        affect_modify( ch, paf, false );
        affect_check(ch, paf);
    }

    rebuild_eq_affects( ch );
}

void DefaultWearlocation::triggersOnUnequip( Character *ch, Object *obj )
{
    oprog_remove( obj, ch );

    switch (obj->item_type) {
    case ITEM_LIGHT:
        if (ch->in_room != 0)
            ch->in_room->updateLight();
        break;

    case ITEM_ARMOR:
        for (int i = 0; i < 4; i++)
            ch->armor[i] += armorCoef * obj->valueByIndex(i);
        break;
    }
}

/*-------------------------------------------------------------------
 * remove 
 *------------------------------------------------------------------*/
const DLString MSG_SELF = "Ты снимаешь %2$O4.";

const DLString &DefaultWearlocation::getMsgSelfRemove(Object *obj) const
{
    if (!msgSelfRemove.empty())
        return msgSelfRemove;

    return MSG_SELF;
}

const DLString MSG_ROOM = "%1$^C1 снимает %2$O4.";

const DLString &DefaultWearlocation::getMsgRoomRemove(Object *obj) const
{
    if (!msgRoomRemove.empty())
        return msgRoomRemove;

    return MSG_ROOM;
}

void DefaultWearlocation::echoRemoveMessages( Character *ch, Object *obj ) const
{
    ch->recho( _(getMsgRoomRemove(obj)), ch, obj );
    ch->pecho( _(getMsgSelfRemove(obj)), ch, obj );
}

bool DefaultWearlocation::remove( Object *obj, int flags )
{
    Character *ch = obj->carried_by;

    if (!canRemove( ch, obj, flags ))
        return false;

    if (IS_SET(flags, F_WEAR_VERBOSE))
        echoRemoveMessages( ch, obj );
    
    unequip( obj );

    if (waitstateRemove > 0)
        ch->setWait( waitstateRemove );

    return true;
}

bool DefaultWearlocation::remove( Character *ch, int flags )
{
    Object *obj;
    
    if (IS_SET(flags, F_WEAR_REPLACE))
        if (pair->canWear( ch, flags ))
            return true;
    
    if (!matches( ch ))
        return false;

    if (!( obj = find( ch ) ))
        return true;
    
    return remove( obj, flags );
}

bool DefaultWearlocation::canRemove( Character *ch, Object *obj, int flags )
{
    if (IS_SET(obj->extra_flags, ITEM_NOREMOVE)) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            echo_master(ch, _("Ты не можешь снять %O4, на этой вещи проклятие."), obj);
        }
        return false;
    }
    
    return true;
}

bool DefaultWearlocation::canRemove( Character *ch, int flags )
{
    Object *obj;

    if (!matches( ch ))
        return false;

    if (!( obj = find( ch ) ))
        return true;

    return canRemove( ch, obj, flags );
}

/*-------------------------------------------------------------------
 * wear 
 *------------------------------------------------------------------*/
int DefaultWearlocation::wear( Object *obj, int flags )
{
    int rc;
    Character *ch = obj->carried_by;
    
    if (( rc = canWear( ch, obj, flags ) ) != RC_WEAR_OK)
        return rc;
    
    if (IS_SET(flags, F_WEAR_REPLACE)
        && !remove( ch, flags )
        && !pair->remove( ch, flags & ~F_WEAR_REPLACE ))
        return RC_WEAR_NOREPLACE;

    if (wearAtomic( ch, obj, flags ) || pair->wearAtomic( ch, obj, flags ))
        return RC_WEAR_OK;

    return RC_WEAR_NOREPLACE;
}

void DefaultWearlocation::triggersOnWear( Character *ch, Object *obj )
{
    oprog_wear( obj, ch );
}

const DLString &DefaultWearlocation::getMsgSelfWear(Character *ch, Object *obj) const
{
    return msgSelfWear.getForLang( Player::lang( ch ) );
}

const DLString &DefaultWearlocation::getMsgRoomWear(Object *obj) const
{
    return msgRoomWear;
}

bool DefaultWearlocation::wearAtomic( Character *ch, Object *obj, int flags )
{
    if (canWear( ch, flags )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho( getMsgSelfWear(ch, obj).c_str(), ch, obj );
            // The room line goes to every onlooker at once, so it has to resolve
            // per recipient rather than in the wearer's language -- same treatment
            // the remove path already gets. msgSelfWear above is a different case:
            // it reaches one reader, so the XMLMultiString lookup is enough.
            ch->recho( _(getMsgRoomWear(obj)), ch, obj );
        }

        triggersOnWear(ch, obj);
        equip( obj );
        return true;
    }

    return false;
}

bool DefaultWearlocation::canWear( Character *ch, int flags )
{
    return matches( ch ) && !find( ch );
}

int DefaultWearlocation::canWear( Character *ch, Object *obj, int flags )
{
    int wear_level = get_wear_level( ch, obj );

    if (wear_level > ch->getRealLevel( )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            echo_master(ch, _("Тебе необходимо достичь %d уровня, чтобы использовать %O4."), wear_level, obj);
            ch->recho( _("%1$^C3 не хватает опыта, чтобы использовать %2$O4."), ch, obj );
            // A personal hero reward stranded above its owner's level (typically
            // after a remort) can be refit down at the quest-trader.
            if (obj->isPersonalQuestReward( ) && obj->hasOwner( ch ))
                ch->pecho( _("Это твоя геройская вещь -- перекуй ее под свой уровень у {hh125квестового торговца{x: {y{hcквест купить перековка{x.") );
        }
        return RC_WEAR_YOUNG;
    }

    if(obj->pIndexData->limit >= 0 && ch->getModifyLevel( ) > obj->level + 20){
         if (IS_SET(flags, F_WEAR_VERBOSE)) {
            echo_master(ch,_("Твой уровень слишком велик, чтобы использовать %1$O4."), obj );
            ch->recho( _("Уровень %1$^C2 слишком велик, чтобы использовать %2$O4."), ch, obj );
         }
        return RC_WEAR_YOUNG;
    }

    if (!matches( ch ) && !pair->matches( ch )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho( msgSelfNoRib.getForLang( Player::lang( ch ) ).c_str( ), ch, obj );
            if (!msgRoomNoRib.empty())
                ch->recho( msgRoomNoRib.c_str( ), ch, obj );
            else
                ch->recho(_("%^C1 пытается надеть %O4 на несуществующую часть тела."), ch, obj);
        }
        return RC_WEAR_NORIB;
    }

    if (conflict->find( ch )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) 
            echo_master(ch, msgSelfConflict.c_str(), ch, obj);
        return RC_WEAR_CONFLICT;
    }
    
    if (IS_SET(flags, F_WEAR_REPLACE) 
        && !canRemove( ch, flags )
        && !pair->canRemove( ch, flags ))
        return RC_WEAR_NOREPLACE;

    return RC_WEAR_OK;
}

/*-------------------------------------------------------------------
 * display 
 *------------------------------------------------------------------*/
void DefaultWearlocation::display( Character *ch, Wearlocation::DisplayList &eq, lang_t lang )
{
    bool found = false;

    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc == this) {
            eq.push_back( make_pair( displayLocation(ch, obj, lang), obj ) );
            found = true;
        }

    if (!found && displayAlways && matches( ch ))
        eq.push_back( make_pair( displayLocation(ch, NULL, lang), (Object *)NULL ) );
}

DLString DefaultWearlocation::displayLocation(Character *ch, Object *obj, lang_t lang)
{
    // 'lang' is the viewer's language; the slot label must NOT depend on the
    // equipment owner's own config (ch here is that owner).
    return msgDisplay.getForLang( lang );
}
