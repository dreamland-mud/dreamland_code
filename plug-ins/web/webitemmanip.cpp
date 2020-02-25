/* $Id$
 *
 * ruffina, 2018
 */
#include "webmanipcommandtemplate.h"
#include "maniplist.h"

#include <map>
#include <list>
#include <sstream>

#include "logstream.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "command.h"
#include "commandmanager.h"
#include "mobilebehavior.h"
#include "behavior_utils.h"

#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "descriptor.h"
#include "comm.h"
#include "gsn_plugin.h"
#include "attract.h"
#include "occupations.h"
#include "shoptrader.h"
#include "move_utils.h"
#include "act_lock.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

NPCharacter * find_mob_with_act( Room *room, bitstring_t act );
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader );

/*
 * Hold the list of all commands and their arguments that are applicable
 * to the item in given context.
 */
struct ItemManipList : public ManipList {
    Object *target;

    ItemManipList( Object *target, const DLString &descr ) {
        this->target = target;
        this->descr = descr;
    }

    // Add commands to the list that is going to be shown
    // below the divider and passed in the 'l' attribute.
    void addLocal( const DLString &cmdName ) {
        locals.push_back( Manip( cmdName, THIS ) );
    }

    void addLocal( const DLString &cmdName, const DLString &arg ) {
        locals.push_back( Manip( cmdName, arg ) );
    }

    // Add commands to the main list, with various arguments.
    void add( const DLString &cmdName ) {
        manips.push_back( Manip( cmdName, THIS ) );
    }

    void add( const DLString &cmdName, const DLString &arg1 ) {
        manips.push_back( Manip( cmdName, arg1 + " " + THIS ) );
    }

    void add( const DLString &cmdName, Object *obj1, Object *obj2 ) {
        DLString args = (obj1 == target ? THIS : DLString( obj1->getID( ) ));
        args += " ";
        args += (obj2 == target ? THIS : DLString( obj2->getID( ) ));
        manips.push_back( Manip( cmdName, args ) );
    }

    void add( const DLString &cmdName, Object *obj1, Object *obj2, const DLString &pocket ) {
        DLString args = (obj1 == target ? THIS : DLString( obj1->getID( ) ));
        args += " ";
        args += (obj2 == target ? THIS : DLString( obj2->getID( ) ));
        args += ":" + pocket;
        manips.push_back( Manip( cmdName, args ) );
    }

    // For pocket commands such as: get all <container>:<pocket>, l in <container>:<pocket>
    void add( const DLString &cmdName, const DLString &arg1, const DLString &pocket ) {
        DLString args = arg1 + " " + THIS + ":" + pocket;
        manips.push_back( Manip( cmdName, args ) );
    }

    // addAll methods will create commands in the format: 
    // get все.'item names'[ <container>[:<pocket>]]
    void addAll( const DLString &cmdName, Object *obj1, Object *obj2 = NULL ) {
        DLString args = "все.'" + DLString(obj1->getName( )) + "'";
        if (obj2) {
            args += " " + DLString( obj2->getID( ) ); 
        }
        manips.push_back( Manip( cmdName, args ) );
    }

    void addAll( const DLString &cmdName, Object *obj1, Object *obj2, const DLString &pocket ) {
        DLString args = "все.'" + DLString(obj1->getName( )) + "'";
        args += " ";
        args += (obj2 == target ? THIS : DLString( obj2->getID( ) ));
        args += ":" + pocket;
        manips.push_back( Manip( cmdName, args ) );
    }

    void add( const DLString &cmdName, Object *obj1, Character *ch1 ) {
        DLString args = (obj1 == target ? THIS : DLString( obj1->getID( ) ));
        args += " ";
        args += DLString( ch1->getID( ) );
        manips.push_back( Manip( cmdName, args ) );
    }

    void add( const DLString &cmdName, Character *ch1, Object *obj1 ) {
        DLString args = DLString( ch1->getID( ) );
        args += " ";
        args += DLString( obj1->getID( ) );
        manips.push_back( Manip( cmdName, args ) );
    }

    virtual DLString getID( ) const {
        return DLString( target->getID( ) );
    }
};

static bool has_trigger_close( Object *obj )
{

    switch (obj->item_type) {
         case ITEM_PORTAL:
             return IS_SET(obj->value1(), EX_ISDOOR)
                    && !IS_SET(obj->value1(), EX_NOCLOSE|EX_CLOSED);

         case ITEM_DRINK_CON:
             return IS_SET(obj->value3(), DRINK_CLOSE_CORK|DRINK_CLOSE_NAIL|DRINK_CLOSE_KEY)
                    && !IS_SET(obj->value3(),  DRINK_CLOSED);

         case ITEM_CONTAINER:
             return IS_SET(obj->value1(), CONT_CLOSEABLE) 
                    && !IS_SET(obj->value1(), CONT_CLOSED);

         default:
             return false;
    }
}

static bool has_trigger_lock( Object *obj, Character *ch )
{

    switch (obj->item_type) {
         case ITEM_PORTAL:
             return IS_SET(obj->value1(), EX_ISDOOR)
                    && IS_SET(obj->value1(), EX_CLOSED)
                    && !IS_SET(obj->value1(), EX_NOCLOSE|EX_NOLOCK|EX_LOCKED)
                    && obj->value4() > 0;

         case ITEM_CONTAINER:
             return IS_SET(obj->value1(), CONT_CLOSED)
                    && IS_SET(obj->value1(), CONT_CLOSEABLE)
                    && !IS_SET(obj->value1(), CONT_LOCKED)
                    && (obj->value2() > 0
                            || (obj->behavior && obj->behavior->canLock( ch )));

         default:
             return false;
    }
}

static bool has_trigger_unlock( Object *obj, Character *ch )
{

    switch (obj->item_type) {
         case ITEM_PORTAL:
             return IS_SET(obj->value1(), EX_ISDOOR)
                    && IS_SET(obj->value1(), EX_CLOSED)
                    && IS_SET(obj->value1(), EX_LOCKED)
                    && obj->value4() > 0;

         case ITEM_DRINK_CON:
             return IS_SET(obj->value3(), DRINK_CLOSED)
                    && IS_SET(obj->value3(),  DRINK_LOCKED);

         case ITEM_CONTAINER:
             return IS_SET(obj->value1(), CONT_CLOSED)
                    && IS_SET(obj->value1(), CONT_LOCKED) 
                    && (obj->value2() > 0
                            || (obj->behavior && obj->behavior->canLock( ch )));

         default:
             return false;
    }
}

static bool has_trigger_open( Object *obj )
{

    switch (obj->item_type) {
         case ITEM_PORTAL:
             return IS_SET(obj->value1(), EX_ISDOOR)
                    && IS_SET(obj->value1(), EX_CLOSED)
                    && !IS_SET(obj->value1(), EX_LOCKED);

         case ITEM_DRINK_CON:
             return IS_SET(obj->value3(), DRINK_CLOSED)
                    && !IS_SET(obj->value3(),  DRINK_LOCKED);

         case ITEM_CONTAINER:
             return IS_SET(obj->value1(), CONT_CLOSED)
                    && IS_SET(obj->value1(), CONT_CLOSEABLE) 
                    && !IS_SET(obj->value1(), CONT_LOCKED);

         default:
             return false;
    }
}

// Return true if standing by a shop keeper.
static bool has_trade_triggers( Object *obj, Character *ch )
{
    // Idiotic interface...
    NPCharacter *keeper;
    ShopTrader::Pointer trader;

    trader = find_attracted_mob_behavior<ShopTrader>( ch, OCC_SHOPPER );
   
    if (!trader)
        return false;

    keeper = trader->getChar( );
    return get_cost( keeper, obj, false, trader ) > 0;
}

static bool has_trigger_auction( Object *obj )
{
    switch (obj->item_type) {
        case ITEM_MONEY:
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
        case ITEM_TATTOO:
            return false;
    }

    if (obj->timer != 0)
        return false;

    if (IS_OBJ_STAT(obj, ITEM_NOSELL))
        return false;

    if (auction->item)
        return false;

    return true;
}

static bool has_trigger_listen( Object *obj )
{
    // See if it has a sound defined in the area file.
    if (!obj->pIndexData->sound.empty( ))
        return true;

    // See if wrappers for this item or its index data has a onListen function defined.
    FENIA_HAS_TRIGGER(obj, "Listen");
    FENIA_NDX_HAS_TRIGGER(obj, "Listen");
    return IS_OBJ_STAT(obj, ITEM_HUM);
}

static bool has_trigger_smell( Object *obj )
{
    // See if it has a smell defined in the area file.
    if (!obj->pIndexData->smell.empty( ))
        return true;

    // See if wrappers for this item or its index data has a onSmell function defined.
    FENIA_HAS_TRIGGER(obj, "Smell");
    FENIA_NDX_HAS_TRIGGER(obj, "Smell");
    return false;
}

static bool has_trigger_use( Object *obj )
{
    // See if wrappers for this item or its index data has a onUse function defined.
    FENIA_HAS_TRIGGER(obj, "Use");
    FENIA_NDX_HAS_TRIGGER(obj, "Use");
    
    // TODO skillevent handlers (not implemented anywhere yet)

    // See if item's behavior declares itself as handling 'use' command.
    if (obj_has_trigger( obj, "use" ))
        return true;

    return false;
}

static bool has_trigger_examine( Object *obj )
{
    // See if wrappers for this item or its index data has a onExamine function defined.
    FENIA_HAS_TRIGGER(obj, "Examine");
    FENIA_NDX_HAS_TRIGGER(obj, "Examine");
    
    // See if item's behavior declares itself as handling 'examine' command.
    if (obj_has_trigger( obj, "examine" ))
        return true;

    // Rely on item types that typically allow examining.
    // TODO: different command aliases for containers and everything else.
    switch(obj->item_type) {
    case ITEM_MONEY:
    case ITEM_DRINK_CON:
    case ITEM_CONTAINER:
    case ITEM_KEYRING:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_PORTAL:
        return true;
    }

    return false;
}

/*
 * Decorates each object in a list with manipulations markup,
 * adding tags like <m c='wear,drop,look,lore'>a short sword</m>
 * around each name.
 */
WEBMANIP_RUN(decorateItem)
{
    const ItemManipArgs &myArgs = static_cast<const ItemManipArgs &>( args );
    const DLString &descr = myArgs.descr;
    Object *item = myArgs.item;
    Character *ch = myArgs.target;
    const DLString &pocket = myArgs.pocket;
    int combined = myArgs.combined;

    ItemManipList manips( item, descr );
    bitstring_t wear = item->wear_flags;
    REMOVE_BIT(wear, ITEM_TAKE|ITEM_NO_SAC);

    if (ch->is_immortal())  {
        manips.addLocal("stat", "obj $");
        manips.addLocal("oedit", DLString(item->pIndexData->vnum));
    }

    // My inventory or equipment:
    if (item->carried_by == ch) {
        // Item type-specific commands.
        switch (item->item_type) {
            case ITEM_FOOD:
            case ITEM_PILL:
                if (item->wear_loc == wear_none)
                    manips.add( "eat" );
                break;
            case ITEM_POTION:
                if (item->wear_loc == wear_none)
                    manips.add( "quaff" );
                break;
            case ITEM_SCROLL:
                if (item->wear_loc == wear_none)
                    manips.add( "recite" );
                break;
            case ITEM_DRINK_CON:
                manips.add( "drink" );
                if (item->wear_loc == wear_none) 
                    manips.add( "pour" );
                if (get_obj_room_type( ch, ITEM_FOUNTAIN ))
                    manips.add( "fill" );
                break;
            case ITEM_FOUNTAIN:
                manips.add( "drink" );
                if (item->wear_loc == wear_none) {
                    Object *drink = get_obj_carry_type( ch, ITEM_DRINK_CON );
                    if (drink)
                        manips.add( "fill", drink, item );
                }
                break;
            case ITEM_CONTAINER:
                if (!IS_PIT(item))
                    manips.add( "get", "все" );
                break;
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                manips.add( "get", "все" );
                break;
            case ITEM_WAND:
                if (item->wear_loc == wear_hold) 
                    manips.add("zap");
                break;
            case ITEM_STAFF:
                if (item->wear_loc == wear_hold) 
                    manips.add("brandish");
                break;
        }

        // Inventory only:
        if (item->wear_loc == wear_none) {
            if (wear != 0 || item->item_type == ITEM_LIGHT) 
                manips.add( "wear" );

            if (!IS_SET(item->extra_flags, ITEM_NODROP)) {
                manips.add( "drop" );
                if (combined > 1)
                    manips.addAll( "drop", item );
            }

            if (find_attracted_mob( ch, OCC_REPAIRMAN )) {
                manips.addLocal( "repair" );
                manips.addLocal( "estimate" );
            }

            if (find_mob_with_act( ch->in_room, ACT_SAGE )) 
                manips.addLocal( "identify" );

            if (has_trade_triggers( item, ch )) {
                manips.addLocal( "sell" );
                manips.addLocal( "value" );
            }
        } 
        // Equipment only:
        else {
            manips.add( "remove" );
        }

        if (has_trigger_open( item ))
            manips.add( "open" );
        if (has_trigger_close( item ))
            manips.add( "close" );
        if (has_trigger_lock( item, ch ))
            manips.add( "lock" );
        if (has_trigger_unlock( item, ch ))
            manips.add( "unlock" );

        if (has_trigger_examine( item ))
            manips.add( "examine" );

        manips.add( "look" );

        if (has_trigger_smell( item ))
            manips.add( "smell" );

        if (has_trigger_listen( item ))
            manips.add( "listen" );

        if (has_trigger_use( item ))
            manips.add( "use" );

        if (item->wear_loc == wear_none && gsn_lore->usable( ch )) 
            manips.add( "lore" );
    } 
    // Other character's inventory or equipment:
    else if (item->carried_by != NULL) {
        // Inventory:
        if (item->wear_loc == wear_none) {
            if (gsn_steal->usable( ch ))  {
                manips.add( "steal", item, item->carried_by );
            }
        // Equipment: get <victim> <item>
        } else {
            manips.add( "get", item->carried_by, item );
        }
    }
    // On the floor:
    else if (item->in_room != NULL) {
        if (item->can_wear( ITEM_TAKE )) {
            manips.add( "get" );
            if (combined > 1)
                manips.addAll( "get", item );
        }

        // Item type-specific commands.
        switch (item->item_type) {
            case ITEM_DRINK_CON:
                manips.add( "drink" );
                break;
            case ITEM_FOUNTAIN:
                manips.add( "drink" );
                {
                    Object *drink = get_obj_carry_type( ch, ITEM_DRINK_CON );
                    if (drink)
                        manips.add( "fill", drink, item );

                }
                break;
            case ITEM_PORTAL:
                manips.add( "enter" );
                break;

            case ITEM_CONTAINER:
                if (!IS_PIT(item))
                    manips.add( "get", "все" );
                break;

            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                manips.add( "get", "все" );
                break;

            case ITEM_FURNITURE:
                if (IS_SET(item->value2(),SIT_IN|SIT_ON|SIT_AT))
                    manips.add( "sit" );
                if (IS_SET(item->value2(),REST_IN|REST_ON|REST_AT))
                    manips.add( "rest" );
                if (IS_SET(item->value2(),SLEEP_IN|SLEEP_ON|SLEEP_AT))
                    manips.add( "sleep" );

                break;
        }

        if (has_trigger_open( item ))
            manips.add( "open" );
        if (has_trigger_close( item ))
            manips.add( "close" );
        if (has_trigger_lock( item, ch ))
            manips.add( "lock" );
        if (has_trigger_unlock( item, ch ))
            manips.add( "unlock" );

        if (has_trigger_examine( item ))
            manips.add( "examine" );

        manips.add( "look" );

        if (has_trigger_smell( item ))
            manips.add( "smell" );

        if (has_trigger_listen( item ))
            manips.add( "listen" );

        if (has_trigger_use( item ))
            manips.add( "use" );
    }
    // Inside a container:
    else if (item->in_obj != NULL) {
        if (pocket.empty( )) {
            manips.add( "get", item, item->in_obj );
            if (combined > 1)
                manips.addAll( "get", item, item->in_obj );
        }
        else {
            manips.add( "get", item, item->in_obj, pocket );
            if (combined > 1)
                manips.addAll( "get", item, item->in_obj, pocket );
        }
    }

    buf << manips.toString( );
    return true;
}


WEBMANIP_RUN(decorateShopItem)
{
    const ShopItemManipArgs &myArgs = static_cast<const ShopItemManipArgs &>( args );
    const DLString &descr = myArgs.descr;
    Object *item = myArgs.item;

    ItemManipList manips( item, descr );

    manips.add( "buy" );
    if (IS_OBJ_STAT( item, ITEM_INVENTORY )) 
        manips.add( "properties" );

    buf << manips.toString( );
    return true;
}

WEBMANIP_RUN(decoratePocket)
{
    const PocketManipArgs &myArgs = static_cast<const PocketManipArgs &>( args );
    const DLString &pocket= myArgs.pocket;
    Object *container = myArgs.container;

    ItemManipList manips( container, pocket );

    manips.add( "look", "в", pocket );
    manips.add( "get", "все", pocket );
    buf << manips.toString( );
    return true;
}

