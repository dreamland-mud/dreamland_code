/* $Id: wrappermanager.cpp,v 1.1.2.5.6.2 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */
#include "wrappermanager.h"
#include "characterwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "mobindexwrapper.h"
#include "objindexwrapper.h"
#include "areaindexwrapper.h"
#include "spellwrapper.h"
#include "affecthandlerwrapper.h"
#include "affectwrapper.h"
#include "skillcommandwrapper.h"
#include "commandwrapper.h"
#include "areaquestwrapper.h"
#include "autoquestwrapper.h"
#include "questregistrator.h"
#include "behaviorwrapper.h"
#include "wordeffectwrapper.h"
#include "wordeffect.h"
#include "subr.h"
#include "fenia/register-impl.h"

#include "wrappedcommand.h"
#include "class.h"
#include "spell.h"
#include "affecthandler.h"
#include "skill.h"
#include "skillcommand.h"
#include "room.h"
#include "character.h"
#include "object.h"
#include "affect.h"
#include "behavior.h"
#include "merc.h"
#include "dreamland.h"
#include "npcharacter.h"
#include "logstream.h"

#include "def.h"

using namespace Scripting;

void WrapperManager::initialization( )
{
    FeniaManager::wrapperManager.setPointer( this );
}

void WrapperManager::destruction( )
{
    FeniaManager::wrapperManager.clear( );
}

WrapperManager * WrapperManager::getThis( )
{
    return static_cast<WrapperManager *>( FeniaManager::wrapperManager.getPointer( ) );
}
/*
 * wrapping methods
 */
Scripting::Register WrapperManager::getWrapper( Character *ch )
{
    if (!ch)
        return Scripting::Register( );

    return wrapperAux<CharacterWrapper>( ch->getID( ), ch ); 
}

Scripting::Register WrapperManager::getWrapper( ::Object *obj )
{
    if (!obj)
        return Scripting::Register( );

    return wrapperAux<ObjectWrapper>( obj->getID( ), obj );
}

Scripting::Register WrapperManager::getWrapper( Room *room )
{
    if (!room)
        return Scripting::Register( );

    return wrapperAux<RoomWrapper>( ROOM_VNUM2ID( room->vnum ), room );
}

Scripting::Register WrapperManager::getWrapper( mob_index_data *mob )
{
    if (!mob)
        return Scripting::Register( );

    return wrapperAux<MobIndexWrapper>( MOB_VNUM2ID( mob->vnum ), mob );
}

Scripting::Register WrapperManager::getWrapper( obj_index_data *obj )
{
    if (!obj)
        return Scripting::Register( );

    return wrapperAux<ObjIndexWrapper>( OBJ_VNUM2ID( obj->vnum ), obj );
}

Scripting::Register WrapperManager::getWrapper( AreaIndexData *pArea )
{
    if (!pArea)
        return Scripting::Register( );

    return wrapperAux<AreaIndexWrapper>( AREA_VNUM2ID(pArea->min_vnum), pArea );
}

Scripting::Register WrapperManager::getWrapper(Spell *spell) 
{
    if (!spell)
        return Scripting::Register();

    return wrapperAux<SpellWrapper>(spell->getID(), spell);
}

Scripting::Register WrapperManager::getWrapper(AffectHandler *ah) 
{
    if (!ah)
        return Scripting::Register();
    
    return wrapperAux<AffectHandlerWrapper>(ah->getID(), ah);
}

Scripting::Register WrapperManager::getWrapper(Affect *paf) 
{
    if (!paf)
        return Scripting::Register();
   
    return AffectWrapper::wrap(*paf); 
}

Scripting::Register WrapperManager::getWrapper(SkillCommand *cmd) 
{
    if (!cmd)
        return Scripting::Register();
    
    return wrapperAux<SkillCommandWrapper>(cmd->getID(), cmd);
}

Scripting::Register WrapperManager::getWrapper(WrappedCommand *cmd) 
{
    if (!cmd)
        return Scripting::Register();
    
    return wrapperAux<FeniaCommandWrapper>(cmd->getID(), cmd);
}

Scripting::Register WrapperManager::getWrapper(AreaQuest *q) 
{
    if (!q)
        return Scripting::Register();
    
    return wrapperAux<AreaQuestWrapper>(q->getID(), q);
}

Scripting::Register WrapperManager::getWrapper(Behavior *bhv) 
{
    if (!bhv)
        return Scripting::Register();
    
    return wrapperAux<BehaviorWrapper>(bhv->getID(), bhv);
}

Scripting::Register WrapperManager::getWrapper(QuestRegistratorBase *reg)
{
    if (!reg)
        return Scripting::Register();

    return wrapperAux<AutoQuestWrapper>(reg->getID(), reg);
}

Scripting::Register WrapperManager::getWrapper(WordEffect *effect)
{
    if (!effect)
        return Scripting::Register();

    return wrapperAux<WordEffectWrapper>(effect->getID(), effect);
}



template <typename WrapperType, typename TargetType>
Scripting::Register WrapperManager::wrapperAux( long long id, TargetType t )
{
    if (!t->wrapper) {
        typename WrapperType::Pointer wrapper( NEW );

        wrapper->setTarget( t );
        t->wrapper = &Scripting::Object::manager->allocate( );
        t->wrapper->setHandler( wrapper );
    }

    return Scripting::Register( t->wrapper );
}

/*
 * link methods
 */

/*
 * Saved rooms and mobiles can carry two live copies with the same ID (2026-10-12:
 * two vodka bottles in saved room 13034). Linking both to one wrapper repoints it
 * at the second copy, so when the wrapper is freed only that copy forgets it and
 * the first keeps a dangling pointer. The later copy gets a fresh ID instead.
 */
template <typename WrapperType, typename TargetType>
static bool wrapper_taken_by_other( long long id, TargetType t )
{
    WrapperManagerBase::WrapperMap::iterator i = WrapperManagerBase::map.find( id );

    if (i == WrapperManagerBase::map.end( ))
        return false;

    try {
        TargetType owner = wrapper_cast<WrapperType>( i->second )->getTarget( );
        return owner != 0 && owner != t;
    } catch (const ::Exception &) {
        return false; // offline or zombie: nobody holds it
    }
}

void WrapperManager::linkWrapper( Character * ch )
{
    if (ch->is_npc( ) && wrapper_taken_by_other<CharacterWrapper>( ch->getID( ), ch )) {
        long long oldID = ch->getID( );
        ch->setID( dreamland->genID( ) );
        LogStream::sendError( ) << "fenia: duplicate mob id " << oldID
            << " [" << ch->getNPC( )->pIndexData->vnum << "], given new id " << ch->getID( ) << endl;
        return;
    }

    linkAux<CharacterWrapper>( ch->getID( ), ch );
}
void WrapperManager::linkWrapper( ::Object * obj )
{
    if (wrapper_taken_by_other<ObjectWrapper>( obj->getID( ), obj )) {
        long long oldID = obj->getID( );
        obj->setID( dreamland->genID( ) );
        LogStream::sendError( ) << "fenia: duplicate object id " << oldID
            << " [" << obj->pIndexData->vnum << "], given new id " << obj->getID( ) << endl;
        return;
    }

    linkAux<ObjectWrapper>( obj->getID( ), obj );
}
void WrapperManager::linkWrapper( Room *room )
{
    linkAux<RoomWrapper>( ROOM_VNUM2ID( room->vnum ), room );
}
void WrapperManager::linkWrapper( mob_index_data *mob )
{
    linkAux<MobIndexWrapper>( MOB_VNUM2ID( mob->vnum ), mob );
}
void WrapperManager::linkWrapper( obj_index_data *obj )
{
    linkAux<ObjIndexWrapper>( OBJ_VNUM2ID( obj->vnum ), obj );
}
void WrapperManager::linkWrapper( AreaIndexData *pArea )
{
    linkAux<AreaIndexWrapper>( AREA_VNUM2ID(pArea->min_vnum), pArea );
}

void WrapperManager::linkWrapper(Spell *spell) 
{
    linkAux<SpellWrapper>(spell->getID(), spell);
}

void WrapperManager::linkWrapper(AffectHandler *ah) 
{
    linkAux<AffectHandlerWrapper>(ah->getID(), ah);
}

void WrapperManager::linkWrapper(Affect *paf) 
{
    /* DO NOTHING */
}

void WrapperManager::linkWrapper(SkillCommand *cmd) 
{
    linkAux<SkillCommandWrapper>(cmd->getID(), cmd);
}

void WrapperManager::linkWrapper(WrappedCommand *cmd) 
{
    linkAux<FeniaCommandWrapper>(cmd->getID(), cmd);
}

void WrapperManager::linkWrapper(AreaQuest *q) 
{
    linkAux<AreaQuestWrapper>(q->getID(), q);
}

void WrapperManager::linkWrapper(Behavior *bhv)
{
    linkAux<BehaviorWrapper>(bhv->getID(), bhv);
}

void WrapperManager::linkWrapper(WordEffect *effect)
{
    linkAux<WordEffectWrapper>(effect->getID(), effect);
}

void WrapperManager::linkWrapper(QuestRegistratorBase *reg)
{
    linkAux<AutoQuestWrapper>(reg->getID(), reg);
}

void WrapperManager::getTarget( const Scripting::Register &reg, Character *& ch )
{
    ch = wrapper_cast<CharacterWrapper>( reg )->getTarget( );
}

void WrapperManager::getTarget( const Scripting::Register &reg, ::Object *& obj )
{
    obj = wrapper_cast<ObjectWrapper>( reg )->getTarget( );
}

void WrapperManager::getTarget( const Scripting::Register &reg, Room *& room )
{
    room = wrapper_cast<RoomWrapper>( reg )->getTarget( );
}

template <typename WrapperType, typename TargetType>
void WrapperManager::linkAux( long long id, TargetType t )
{
    WrapperMap::iterator i;
    
    i = map.find( id );

    if (i == map.end( ))
        return;

    t->wrapper = i->second;
    wrapper_cast<WrapperType>(t->wrapper)->setTarget( t );
}

