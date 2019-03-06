/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "mixedpetshop.h"
#include "shoptrader.h"

#include "attract.h"
#include "occupations.h"

#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "interp.h"
#include "act.h"
#include "mercdb.h"

WEARLOC(none);
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader );

/*----------------------------------------------------------------------
 * MixedPetShopRoom
 *---------------------------------------------------------------------*/
bool MixedPetShopRoom::command( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    if (ch->is_npc( ))
        return false;
    
    if (cmdName == "list") {
        doList( ch );
        return true;
    }

    if (cmdName == "buy" ) {
        doBuy( ch, cmdArgs );
        return true;
    }

    return false;
}


static bool __mixed_entry_cmp__( const struct MixedEntry &a, const struct MixedEntry &b )
{
    return a.level < b.level;
}

MixedEntry::MixedEntry( )
{
}

MixedEntry::MixedEntry( Pet::Pointer pet, Character *client )
{
    level = pet->getLevel( client );
    cost = pet->toSilver( client );
    short_descr = pet->getChar( )->getNPC( )->getShortDescr( );
    name = pet->getChar( )->getName( );
    this->pet = true;
}

MixedEntry::MixedEntry( Object *obj, int cost )
{
    level = obj->level;
    this->cost = cost;
    short_descr = obj->getShortDescr( );
    name = obj->getName( );
    pet = false;
}

void MixedPetShopRoom::createMixedList( MixedList &list, Character *client )
{
    PetShopStorage::Pointer storage = getStorage( );

    if (storage) {
        Room *room = storage->getRoom( );

        for (Character *rch = room->people; rch; rch = rch->next_in_room) {
            Pet::Pointer pet = storage->getPetBehavior( rch );

            if (pet) 
                list.push_back( MixedEntry( pet, client ) );
        }
    }

    ShopTrader::Pointer trader = find_attracted_mob_behavior<ShopTrader>( client, OCC_SHOPPER );

    if (trader) {
        NPCharacter *keeper = trader->getChar( );
        int cost;

        for (Object *obj = keeper->carrying; obj; obj = obj->next_content) 
            if (obj->wear_loc == wear_none 
                && ( cost = get_cost( keeper, obj, true, trader ) ) > 0 
                && client->can_see( obj ))  
            {
                list.push_back( MixedEntry( obj, cost ) );
            }
    }

    list.sort( __mixed_entry_cmp__ );

    int n_pets = 0, n_items = 0;
    for (MixedList::iterator i = list.begin( ); i != list.end( ); i++) {
        i->pet ? n_pets++ : n_items++;
        i->pos = i->pet ? n_pets : n_items;
    }
}

void MixedPetShopRoom::doList( Character *client )
{
    MixedList list;
    MixedList::iterator i;
    ostringstream buf;
    int cnt = 0;
    
    createMixedList( list, client );

    if (list.empty( )) {
        client->println( "Магазин не работает." );
        return;
    }

    buf << "[ Ном.| Ур.  Цена ] Товар" << endl;
    
    for (i = list.begin( ); i != list.end( ); i++)
        buf << dlprintf( "[ {Y%3d{x |%3d %5d ] %s\n\r",
                         ++cnt, i->level, i->cost, i->short_descr.ruscase( '1' ).c_str( ) );

    client->send_to( buf );
}

bool MixedPetShopRoom::lookupMixedList( MixedList &list, MixedEntry &e, Character *client, DLString &arg )
{
    MixedList::iterator i;

    if (arg.isNumber( )) {
        int number = arg.toInt( );

        for (i = list.begin( ); i != list.end( ) && number > 1; i++)
            number--;
        
        if (i == list.end( )) 
            return false;

        e = *i;
        return true;
    }
    else {
        int number = arg.getNumberArgument( );
        int count = 0;

        for (i = list.begin( ); i != list.end( ); i++) {
            if (!is_name( arg.c_str( ), i->short_descr.ruscase( '7' ).c_str( ) )
                && !is_name( arg.c_str( ), i->name.c_str( ) ))
                continue;

            if (++count == number) {
                e = *i;
                return true;
            }
        }

        return false;
    }
}


void MixedPetShopRoom::doBuy( Character *client, const DLString &constArguments )
{
    MixedList list;
    MixedEntry e;
    int quantity;
    DLString arguments;

    createMixedList( list, client );

    arguments = constArguments;
    quantity = arguments.getMultArgument( );
    
    if (lookupMixedList( list, e, client, arguments )) {
        DLString newArg;
        
        /* XXX 'buy <number>.<string>' doesn't work for services */
        if (e.pet)
            newArg = e.short_descr.colourStrip( ).ruscase( '1' );
        else
            newArg = DLString( e.pos ) + "." + e.name;
        
        if (quantity > 1) 
            newArg = DLString( quantity ) + "*" + newArg;

        if (e.pet) {
            PetShopStorage::Pointer storage = getStorage( );
            
            if (storage) {
                storage->doBuy( client, newArg );
                return;
            }
        } else {
            interpret_cmd( client, "buy", newArg.c_str( ) );
            return;
        }
    }
    
    client->println( "Извини, ты не можешь купить этого здесь." );
}

