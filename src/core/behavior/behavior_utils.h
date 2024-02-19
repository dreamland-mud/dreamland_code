/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __BEHAVIOR_UTILS_H__
#define __BEHAVIOR_UTILS_H__

#include "mobilebehavior.h"
#include "npcharacter.h"
#include "room.h"
#include "merc.h"


#define BEHAVIOR_CALL( var, method, args...) \
        if (var && var->behavior) { \
            if (var->behavior->method( args )) \
                return true; \
        }

#define BEHAVIOR_VOID_CALL( var, method, args...) \
        if (var && var->behavior) { \
            var->behavior->method( args ); \
        }

extern MOB_INDEX_DATA   * mob_index_hash          [MAX_KEY_HASH];
extern OBJ_INDEX_DATA   * obj_index_hash          [MAX_KEY_HASH];

template <typename Bhv>
::Pointer<Bhv> find_people_behavior( Room * room )
{
    Character *ch;
    NPCharacter *mob;
    ::Pointer<Bhv> behavior;

    for (ch = room->people; ch; ch = ch->next_in_room)
        if (ch->is_npc( )) {
            mob = ch->getNPC( );

            if (mob->behavior) {
                behavior = mob->behavior.getDynamicPointer<Bhv>( );

                if (behavior)
                    return behavior;
            }
        }

    return behavior;
}

template<typename T>
static inline std::vector<MOB_INDEX_DATA *> find_mob_indexes( )
{
    int iHash;
    std::vector<MOB_INDEX_DATA *> v;
    MOB_INDEX_DATA *pMobIndex;
    
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next) 
            if (pMobIndex->behavior) {
                XMLNode::Pointer node = pMobIndex->behavior->getFirstNode( );
                
                if (T::MOC_TYPE == node->getAttribute( XMLNode::ATTRIBUTE_TYPE ) )
                    v.push_back( pMobIndex );
            }

    return v;
}

template<typename T>
static inline MOB_INDEX_DATA * find_mob_unique_index( )
{
    int iHash;
    MOB_INDEX_DATA *pMobIndex;
    
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next) 
            if (pMobIndex->behavior) {
                XMLNode::Pointer node = pMobIndex->behavior->getFirstNode( );
                
                if (T::MOC_TYPE == node->getAttribute( XMLNode::ATTRIBUTE_TYPE ) )
                    return pMobIndex;
            }

    return NULL;
}

template<typename T>
static inline std::vector<OBJ_INDEX_DATA *> find_obj_indexes( )
{
    int iHash;
    std::vector<OBJ_INDEX_DATA *> v;
    OBJ_INDEX_DATA *pObjIndex;
    
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (pObjIndex = obj_index_hash[iHash]; pObjIndex; pObjIndex = pObjIndex->next) 
            if (pObjIndex->behavior) {
                XMLNode::Pointer node = pObjIndex->behavior->getFirstNode( );
                
                if (T::MOC_TYPE == node->getAttribute( XMLNode::ATTRIBUTE_TYPE ) )
                    v.push_back( pObjIndex );
            }

    return v;
}

template<typename T>
static inline OBJ_INDEX_DATA * find_obj_random_index( )
{
    int iHash;
    std::vector<OBJ_INDEX_DATA *> v;
    OBJ_INDEX_DATA *pObjIndex;
    
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (pObjIndex = obj_index_hash[iHash]; pObjIndex; pObjIndex = pObjIndex->next) 
            if (pObjIndex->behavior) {
                XMLNode::Pointer node = pObjIndex->behavior->getFirstNode( );
                
                if (T::MOC_TYPE == node->getAttribute( XMLNode::ATTRIBUTE_TYPE ) )
                    v.push_back( pObjIndex );
            }
    
    if (v.empty( ))
        return NULL;

    return v [number_range( 0, v.size( ) - 1 )];
}

template<typename T>
static inline OBJ_INDEX_DATA * find_obj_unique_index( )
{
    int iHash;
    OBJ_INDEX_DATA *pObjIndex;
    
    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (pObjIndex = obj_index_hash[iHash]; pObjIndex; pObjIndex = pObjIndex->next) 
            if (pObjIndex->behavior) {
                XMLNode::Pointer node = pObjIndex->behavior->getFirstNode( );
                
                if (T::MOC_TYPE == node->getAttribute( XMLNode::ATTRIBUTE_TYPE ) )
                    return pObjIndex;
            }

    return NULL;
}

#endif
