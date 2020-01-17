/* $Id$
 *
 * ruffina, 2004
 */
#include <functional>

#include "petshopstorage.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "act.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*----------------------------------------------------------------------
 * PetShopStorage
 *---------------------------------------------------------------------*/
bool PetShopStorage::isCommon( )
{
    return false;
}

bool PetShopStorage::canEnter( Character * )
{
    return false;
}

NPCharacter * PetShopStorage::getKeeper( ) const
{
    return NULL;
}

bool PetShopStorage::canServeClient( Character *client )
{
    return true;
}

void PetShopStorage::msgListEmpty( Character *client )
{
    client->println( "Извини, все животные давно разбежались." );
}

void PetShopStorage::msgListBefore( Character *client )
{
    client->println( "Существа на продажу:" );
}

void PetShopStorage::msgListAfter( Character *client )
{
}

void PetShopStorage::msgListRequest( Character *client )
{
}

void PetShopStorage::msgBuyRequest( Character * )
{
}

struct PetSortItem {
    PetSortItem(Character *client, Pet::Pointer &pet)
        : pet(pet)
    {
        level = pet->getLevel( client );
    }
    static bool compare( PetSortItem &a, PetSortItem &b )
    {
        return a.level <= b.level;
    }
    int level;
    Pet::Pointer pet;
};
typedef list<PetSortItem> PetSortList;

void PetShopStorage::toStream( Character *client, ostringstream &buf )
{
    Character *rch;
    PetSortList pets;
    PetSortList::const_iterator p;

    for (rch = room->people; rch; rch = rch->next_in_room) {
        Pet::Pointer pet = getPetBehavior( rch );
        
        if (pet)
            pets.push_back( PetSortItem( client, pet ) );
    }
    
    pets.sort( PetSortItem::compare );

    for (p = pets.begin( ); p != pets.end( ); p++) {
        p->pet->toStream( client, buf );
        buf << endl;
    }
}

void PetShopStorage::msgArticleNotFound( Character *client )
{
    client->println( "Извини, ты не можешь купить этого здесь." );
}

void PetShopStorage::msgArticleTooFew( Character *client, Article::Pointer )
{
    client->println( "Ты можешь купить только одного питомца." );
}

Article::Pointer PetShopStorage::findArticle( Character *client, DLString &arguments )
{
    return getPetBehavior( get_char_room( client, room, arguments ) );
}

Pet::Pointer PetShopStorage::getPetBehavior( Character *pet ) const
{
    Pet::Pointer null;
    
    if (!pet)
        return null;

    if (!pet->is_npc( ))
        return null;

    if (IS_CHARMED(pet) || pet->master)
        return null;

    if (!pet->getNPC( )->behavior)
        return null;
        
    return pet->getNPC( )->behavior.getDynamicPointer<Pet>( );
}

