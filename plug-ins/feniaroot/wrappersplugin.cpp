/* $Id: wrappersplugin.cpp,v 1.1.4.13.6.6 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include <jsoncpp/json/json.h>
#include "iconvmap.h"

#include "wrappersplugin.h"
#include "root.h"
#include "idcontainer.h"
#include "guts.h"
#include "nativeext.h"
#include "mobindexwrapper.h"
#include "objindexwrapper.h"
#include "areaindexwrapper.h"
#include "spellwrapper.h"
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
#include "skillmanager.h"
#include "dlscheduler.h"
#include "character.h"
#include "room.h"
#include "object.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#include "subr.h"

using namespace Scripting;

static IconvMap koi2utf("koi8-r", "utf-8");

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
        
    for (auto &room: roomInstances)
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

    for (auto &pArea: areaIndexes)
        if (pArea->wrapper)
            wrapper_cast<AreaIndexWrapper>(pArea->wrapper)->setTarget( pArea );

    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        Spell::Pointer spell = skill->getSpell();

        if (spell && spell->wrapper) {
            LogStream::sendNotice() << "Fenia spell: setting target for " << skill->getName() << endl;
            wrapper_cast<SpellWrapper>(spell->wrapper)->setTarget(*spell);
        }
    }
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
    Class::regMoc<AreaIndexWrapper>( );
    Class::regMoc<SpellWrapper>( );
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
    Class::regMoc<ReligionWrapper>( );
    Class::regMoc<LanguageWrapper>( );
    Class::regMoc<RaceWrapper>( );
    Class::regMoc<LiquidWrapper>( );
    Class::regMoc<SkillWrapper>( );
    Class::regMoc<FeniaSkill>( );
    
    FeniaManager::getThis( )->recover( );
    
    DLScheduler::getThis()->putTaskNOW( ValidateTask::Pointer(NEW) );

    linkTargets();

    if (dreamland->hasOption(DL_BUILDPLOT))
        return;

    LogStream::sendNotice() << "Dumping Fenia API to disk." << endl;

    Json::Value apiDump;
    traitsAPIJson<CharacterWrapper>("char", apiDump, true);     
    traitsAPIJson<ObjectWrapper>("obj", apiDump, true);     
    traitsAPIJson<RoomWrapper>("room", apiDump, true);     
    traitsAPIJson<MobIndexWrapper>("mob_index", apiDump, false);     
    traitsAPIJson<ObjIndexWrapper>("obj_index", apiDump, false);     
    traitsAPIJson<AreaIndexWrapper>("area_index", apiDump, false);    
    traitsAPIJson<Root>("root", apiDump, true);     
    traitsAPIJson<AffectWrapper>("affect", apiDump, false);     
    traitsAPIJson<CommandWrapper>("command", apiDump, false);     
    traitsAPIJson<AreaWrapper>("area", apiDump, false);     
    traitsAPIJson<HometownWrapper>("hometown", apiDump, false);     
    traitsAPIJson<ProfessionWrapper>("profession", apiDump, false);     
    traitsAPIJson<RaceWrapper>("race", apiDump, false);     
    traitsAPIJson<ClanWrapper>("clan", apiDump, false);     
    traitsAPIJson<CraftProfessionWrapper>("craftprofession", apiDump, false);     
    traitsAPIJson<BonusWrapper>("bonus", apiDump, false);     
    traitsAPIJson<ReligionWrapper>("religion", apiDump, false);     
    traitsAPIJson<LiquidWrapper>("liquid", apiDump, false);     
    traitsAPIJson<SkillWrapper>("skill", apiDump, false);     
    traitsAPIJson<FeniaSkill>("feniaskill", apiDump, false);
    traitsAPIJson<SpellWrapper>("spell", apiDump, false);
    traitsAPIJson<FeniaString>("string", apiDump, false);

    Json::FastWriter writer;
    DLFileStream("/tmp", "feniaapi", ".json").fromString(
        koi2utf(
            writer.write(apiDump))
    );
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
    Class::unregMoc<LanguageWrapper>( );
    Class::unregMoc<ReligionWrapper>( );
    Class::unregMoc<ProfessionWrapper>( );
    Class::unregMoc<RaceWrapper>( );
    Class::unregMoc<TablesWrapper>( );
    Class::unregMoc<TableWrapper>( );
    Class::unregMoc<CommandWrapper>( );
    Class::unregMoc<AffectWrapper>( );
    Class::unregMoc<AreaIndexWrapper>( );
    Class::unregMoc<SpellWrapper>( );
    Class::unregMoc<ObjIndexWrapper>( );
    Class::unregMoc<MobIndexWrapper>( );
    Class::unregMoc<CharacterWrapper>( );
    Class::unregMoc<ObjectWrapper>( );
    Class::unregMoc<RoomWrapper>( );
    Class::unregMoc<Root>( );
}

