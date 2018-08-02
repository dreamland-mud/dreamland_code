/* $Id$
 *
 * ruffina, 2004
 */
#ifndef QUESTMODELS_IMPL_H
#define QUESTMODELS_IMPL_H

#include "questmodels.h"
#include "quest-impl.h"
#include "questexceptions.h"

#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "handler.h"
#include "mercdb.h"

/*--------------------------------------------------------------------
 * ItemQuestModel 
 *--------------------------------------------------------------------*/
template<typename T> 
inline void ItemQuestModel::assign( Object *obj )
{
    ::Pointer<T> behavior( NEW );
    
    behavior->setHeroName( charName );
    behavior->setObj( obj );
    obj->behavior.setPointer( *behavior );
}

template<typename T> 
inline Object * ItemQuestModel::createItem( int vnum )
{
    if (vnum <= 0)
	throw QuestCannotStartException( "createItem: Invalid vnum " + DLString( vnum ) );
    
    return createItem<T>( get_obj_index( vnum ) );
}

template<typename T> 
inline Object * ItemQuestModel::createItem( OBJ_INDEX_DATA *pObjIndex )
{
    Object *obj;

    if (!pObjIndex)
	throw QuestCannotStartException( "createItem: zero obj index data" );
    
    obj = create_object( pObjIndex, 0 );
    assign<T>( obj );
    return obj;
}

template<typename T> 
inline Object * ItemQuestModel::getItemWorld( )
{
    for (Object *obj = object_list; obj; obj = obj->next)
	if (check<T>( obj ))
	    return obj;

    return NULL;
}

template<typename T> 
inline Object * ItemQuestModel::getItemList( Object *list )
{
    for (Object *obj = list; obj; obj = obj->next_content)
	if (check<T>( obj ))
	    return obj;

    return NULL;
}

template<typename T>
inline void ItemQuestModel::destroyItems( )
{
    Object *obj, *obj_next;

    for (obj = object_list; obj; obj = obj_next) {
	obj_next = obj->next;
	
	if (check<T>( obj )) 
	    destroy( obj );
    }
}

template<typename T>
inline void ItemQuestModel::clearItems( )
{
    Object *obj, *obj_next;

    for (obj = object_list; obj; obj = obj_next) {
	obj_next = obj->next;
	
	if (check<T>( obj )) 
	    destroy( obj );
    }
}

template<typename T>
inline void ItemQuestModel::destroyItem( )
{
    destroy( getItemWorld<T>( ) );
}

template<typename T>
inline void ItemQuestModel::clearItem( )
{
    clear( getItemWorld<T>( ) );
}

/*--------------------------------------------------------------------
 * MobileQuestModel 
 *--------------------------------------------------------------------*/
template<typename T>
inline void MobileQuestModel::clearMobiles( )
{
    for (Character * wch = char_list; wch; wch = wch->next) 
	if (check<T>( wch )) 
	    clear( wch->getNPC( ) );
}

template<typename T>
inline void MobileQuestModel::clearMobile( )
{
    clear( getMobileWorld<T>( ) );
}

template<typename T>
inline void MobileQuestModel::destroyMobiles( )
{
    Character *wch, *wch_next;
    
    for (wch = char_list; wch; wch = wch_next) {
	wch_next = wch->next;

	if (check<T>( wch )) 
	    destroy( wch->getNPC( ) );
    }
}

template<typename T>
inline void MobileQuestModel::destroyMobile( )
{
    destroy( getMobileWorld<T>( ) );
}

template<typename T> 
inline NPCharacter * MobileQuestModel::getMobileWorld( )
{
    for (Character *wch = char_list; wch; wch = wch->next) 
	if (check<T>( wch ))        
	    return wch->getNPC( );

    return NULL;
}

template<typename T> 
inline NPCharacter * MobileQuestModel::getMobileRoom( Room *room )
{
    for (Character *rch = room->people; rch; rch = rch->next_in_room)
	if (check<T>( rch ))        
	    return rch->getNPC( );

    return NULL;
}

template<typename T> 
inline Room * MobileQuestModel::findMobileRoom( )
{
    NPCharacter *mob = getMobileWorld<T>( );

    return (mob ? mob->in_room : NULL);
}

template<typename T> 
inline NPCharacter * MobileQuestModel::createMobile( int vnum )
{
    if (vnum <= 0)
	throw QuestCannotStartException( "createMobile: Invalid vnum " + DLString( vnum ) );
    
    return createMobile<T>( get_mob_index( vnum ) );
}

template<typename T> 
inline NPCharacter * MobileQuestModel::createMobile( mob_index_data *pMobIndex )
{
    NPCharacter *mob;

    if (!pMobIndex)
	throw QuestCannotStartException( "createMobile: zero mob index data" );
    
    mob = create_mobile( pMobIndex );
    assign<T>( mob );
    return mob;
}

template<typename T> 
inline void MobileQuestModel::assign( NPCharacter *mob )
{
    ::Pointer<T> behavior( NEW );
    
    behavior->setHeroName( charName );
    behavior->setChar( mob );
    mob->behavior.setPointer( *behavior );
}

#endif
