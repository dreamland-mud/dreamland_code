/* $Id: wrappersplugin.cpp,v 1.1.4.13.6.6 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */


#include "wrappersplugin.h"
#include "root.h"
#include "idcontainer.h"
#include "guts.h"
#include "nativeext.h"
#include "mobindexwrapper.h"
#include "objindexwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "affectwrapper.h"
#include "commandwrapper.h"
#include "tableswrapper.h"
#include "validatetask.h"
#include "structwrappers.h"

#include "class.h"
#include "core/fenia/feniamanager.h"
#include "dlscheduler.h"
#include "character.h"
#include "room.h"
#include "object.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#include "subr.h"

using namespace Scripting;

void 
WrappersPlugin::linkTargets()
{
    for (Character *ch = char_list; ch; ch = ch->next) 
        if (ch->wrapper) 
            wrapper_cast<CharacterWrapper>(ch->wrapper)->setTarget( ch );

    for (Character *ch = newbie_list; ch; ch = ch->next) 
        if (ch->wrapper)  
            wrapper_cast<CharacterWrapper>(ch->wrapper)->setTarget( ch );
    
    for (::Object *obj = object_list; obj; obj = obj->next) 
        if (obj->wrapper)
            wrapper_cast<ObjectWrapper>(obj->wrapper)->setTarget( obj );
        
    for (Room *room = room_list; room; room = room->rnext)
        if (room->wrapper)
            wrapper_cast<RoomWrapper>(room->wrapper)->setTarget( room );

    for (int i = 0; i < MAX_KEY_HASH; i++)
        for(MOB_INDEX_DATA *mndx = mob_index_hash[i]; mndx; mndx = mndx->next)
            if(mndx->wrapper)
                wrapper_cast<MobIndexWrapper>(mndx->wrapper)->setTarget( mndx );
            
    for (int i = 0; i < MAX_KEY_HASH; i++)
        for(OBJ_INDEX_DATA *ondx = obj_index_hash[i]; ondx; ondx = ondx->next)
            if(ondx->wrapper)
                wrapper_cast<ObjIndexWrapper>(ondx->wrapper)->setTarget( ondx );
}

void
WrappersPlugin::initialization( ) 
{
    Class::regMoc<Root>( );
    Class::regMoc<RoomWrapper>( );
    Class::regMoc<ObjectWrapper>( );
    Class::regMoc<CharacterWrapper>( );
    Class::regMoc<MobIndexWrapper>( );
    Class::regMoc<ObjIndexWrapper>( );
    Class::regMoc<AffectWrapper>( );
    Class::regMoc<CommandWrapper>( );
    Class::regMoc<TablesWrapper>( );
    Class::regMoc<TableWrapper>( );
    Class::regMoc<HometownWrapper>( );
    Class::regMoc<AreaWrapper>( );
    Class::regMoc<ClanWrapper>( );
    Class::regMoc<ProfessionWrapper>( );
    Class::regMoc<CraftProfessionWrapper>( );
    Class::regMoc<BonusWrapper>( );
    Class::regMoc<RaceWrapper>( );
    Class::regMoc<LiquidWrapper>( );
    Class::regMoc<SkillWrapper>( );
    Class::regMoc<FeniaSkill>( );
    
    FeniaManager::getThis( )->recover( );
    
    DLScheduler::getThis()->putTaskNOW( ValidateTask::Pointer(NEW) );

    linkTargets();

    // Dump API to disk on every plugin load.
    traitsAPIDump<CharacterWrapper>("char", true, true);     
    traitsAPIDump<ObjectWrapper>("obj", true, true);     
    traitsAPIDump<RoomWrapper>("room", true, true);     
    traitsAPIDump<MobIndexWrapper>("mob_index", false, false);     
    traitsAPIDump<ObjIndexWrapper>("obj_index", false, false);     
    traitsAPIDump<Root>("root", true, true);     
    traitsAPIDump<AffectWrapper>("affect", false, false);     
    traitsAPIDump<CommandWrapper>("command", false, false);     
    traitsAPIDump<AreaWrapper>("area", false, false);     
    traitsAPIDump<HometownWrapper>("hometown", false, false);     
    traitsAPIDump<ProfessionWrapper>("profession", false, false);     
    traitsAPIDump<RaceWrapper>("race", false, false);     
    traitsAPIDump<ClanWrapper>("clan", false, false);     
    traitsAPIDump<CraftProfessionWrapper>("craftprofession", false, false);     
    traitsAPIDump<BonusWrapper>("bonus", false, false);     
    traitsAPIDump<LiquidWrapper>("liquid", false, false);     
    traitsAPIDump<SkillWrapper>("skill", false, false);     
    traitsAPIDump<FeniaSkill>("feniaskill", false, false);     
    traitsAPIDump<FeniaString>("string", false, false);     
}

void WrappersPlugin::destruction( ) {
    DLScheduler::getThis()->slay( ValidateTask::Pointer(NEW) );

    Scripting::gc = false;
    FeniaManager::getThis( )->backup( );

    Class::unregMoc<LiquidWrapper>( );
    Class::unregMoc<FeniaSkill>( );
    Class::unregMoc<SkillWrapper>( );
    Class::unregMoc<HometownWrapper>( );
    Class::unregMoc<AreaWrapper>( );
    Class::unregMoc<ClanWrapper>( );
    Class::unregMoc<CraftProfessionWrapper>( );
    Class::unregMoc<BonusWrapper>( );
    Class::unregMoc<ProfessionWrapper>( );
    Class::unregMoc<RaceWrapper>( );
    Class::unregMoc<TablesWrapper>( );
    Class::unregMoc<TableWrapper>( );
    Class::unregMoc<CommandWrapper>( );
    Class::unregMoc<AffectWrapper>( );
    Class::unregMoc<ObjIndexWrapper>( );
    Class::unregMoc<MobIndexWrapper>( );
    Class::unregMoc<CharacterWrapper>( );
    Class::unregMoc<ObjectWrapper>( );
    Class::unregMoc<RoomWrapper>( );
    Class::unregMoc<Root>( );
}
