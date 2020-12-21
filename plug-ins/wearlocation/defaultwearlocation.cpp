/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "defaultwearlocation.h"
#include "wearloc_codes.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "behavior_utils.h"
#include "skillreference.h"
#include "room.h"
#include "affect.h"
#include "character.h"
#include "core/object.h"

#include "damage_impl.h"
#include "damageflags.h"
#include "interp.h"
#include "save.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

WEARLOC(none);
GSN(spellbane);
short get_wear_level( Character *ch, Object *obj );

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
        LogStream::sendError( ) << "Equip_char: " << ch->getNameP() << " already equipped (" << getName( ) << ")." << endl;
    
    obj->wear_loc.assign( this );
    
    triggersOnEquip( ch, obj );

    affectsOnEquip( ch, obj );
    
    saveDrops( ch );

    return true;
}

void DefaultWearlocation::affectsOnEquip( Character *ch, Object *obj )
{
    if (!obj->enchanted)
        for (auto &paf: obj->pIndexData->affected)
            affect_modify( ch, paf, true );

    for (auto &paf: obj->affected)
        affect_modify( ch, paf, true );
}

static bool oprog_cant_equip( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "CantEquip", "C", ch )
    FENIA_NDX_CALL( obj, "CantEquip", "OC", obj, ch )
    return false;
}

static bool oprog_equip( Object *obj, Character *ch )
{
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
    FENIA_CALL( obj, "Remove", "C", ch )
    FENIA_NDX_CALL( obj, "Remove", "OC", obj, ch )
    BEHAVIOR_VOID_CALL( obj, remove, ch )
    return false;
}

bool DefaultWearlocation::canEquip( Character *ch, Object *obj )
{
    if (obj->isAntiAligned( ch )) {
        act( "Твой характер не позволяет тебе носить $o4.", ch, obj, 0, TO_CHAR);
        act( "$o1 соскальзывает с $c2.", ch, obj, 0, TO_ROOM );
        act( "$o1 соскальзывает с тебя.", ch, obj, 0, TO_CHAR );
        return false;
    }
    
    if (!obj->getRealShortDescr( ) && obj->wasAntiAligned( ch )) {
        act("Твой характер все еще не позволяет тебе носить $o4.", ch, obj, 0, TO_CHAR);
        act( "$o1 соскальзывает с $c2.", ch, obj, 0, TO_ROOM );
        act( "$o1 соскальзывает с тебя.", ch, obj, 0, TO_CHAR );
        return false;
    }

    if (IS_SET( obj->extra_flags, ITEM_MAGIC ) && ch->isAffected( gsn_spellbane )) {
        int dam = URANGE( 0, ch->hit - 1, ch->max_hit / 5 );
        act("Магия $o2 аннигилирует с твоим spellbane!", ch, obj, 0, TO_CHAR);
        act("Магия $o2 аннигилирует со spellbane $c2!", ch, obj, 0, TO_ROOM);
        SkillDamage( ch, ch, gsn_spellbane, DAM_NEGATIVE, dam, DAMF_SPELL ).hit( false );
        interpret_raw( ch, "cb", "Меня ударило магической вещью!" );
        return false;
    }

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
    if (!obj->enchanted)
        for (auto &paf: obj->pIndexData->affected) {
            affect_modify( ch, paf, false );
            affect_check(ch, paf);
        }

    for (auto &paf: obj->affected) {
        affect_modify( ch, paf, false );
        affect_check(ch, paf);
    }
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
bool DefaultWearlocation::remove( Object *obj, int flags )
{
    static const char *MSG_SELF = "Ты снимаешь %2$O4.";
    static const char *MSG_ROOM = "%1$^C1 снимает %2$O4.";
    Character *ch = obj->carried_by;
    
    if (!canRemove( ch, obj, flags ))
        return false;
    
    if (IS_SET(flags, F_WEAR_VERBOSE)) {
        ch->recho( msgRoomRemove.empty( ) ? MSG_ROOM : msgRoomRemove.c_str( ), ch, obj );
        ch->pecho( msgSelfRemove.empty( ) ? MSG_SELF : msgSelfRemove.c_str( ), ch, obj );
    }
    
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
        if (IS_SET(flags, F_WEAR_VERBOSE)) 
            act( "Ты не можешь снять $o4.", ch, obj, 0, TO_CHAR);
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

bool DefaultWearlocation::wearAtomic( Character *ch, Object *obj, int flags )
{
    if (canWear( ch, flags )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho( msgSelfWear.c_str( ), ch, obj );
            ch->recho( msgRoomWear.c_str( ), ch, obj );
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
            ch->pecho( "Тебе необходимо достичь %d уровня, чтобы использовать %O4.", wear_level, obj );
            ch->recho( "%1$^C3 не хватает опыта, чтобы использовать %2$O4.", ch, obj );
        }
        return RC_WEAR_YOUNG;
    }

    if(obj->pIndexData->limit >= 0 && ch->getModifyLevel( ) > obj->level + 20){
         if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho( "Твой уровень слишком велик, чтобы использовать %1$O4.", obj );
            ch->recho( "Уровень %1$^C2 слишком велик, чтобы использовать %2$O4.", ch, obj );
         }
        return RC_WEAR_YOUNG;
    }

    if (!matches( ch ) && !pair->matches( ch )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->recho( msgRoomNoRib.c_str( ), ch, obj );
            ch->pecho( msgSelfNoRib.c_str( ), ch, obj );
        }
        return RC_WEAR_NORIB;
    }

    if (!ch->is_npc() && conflict->find( ch )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) 
            ch->pecho( msgSelfConflict.c_str( ), ch, obj );
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
void DefaultWearlocation::display( Character *ch, Wearlocation::DisplayList &eq )
{
    bool found = false;

    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc == this) {
            eq.push_back( make_pair( msgDisplay, obj ) ); 
            found = true;
        }

    if (!found && displayAlways && matches( ch ))
        eq.push_back( make_pair( msgDisplay, (Object *)NULL ) );
}

DLString DefaultWearlocation::displayName(Character *ch, Object *obj)
{
    return obj->getShortDescr('1');
}