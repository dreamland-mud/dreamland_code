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
#include "affecthandlerwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "affectwrapper.h"
#include "skillcommandwrapper.h"
#include "commandwrapper.h"
#include "tableswrapper.h"
#include "validatetask.h"
#include "structwrappers.h"
#include "skillwrapper.h"
#include "feniaskillaction.h"
#include "commandmanager.h"
#include "wrappedcommand.h"
#include "areaquestwrapper.h"
#include "behaviorwrapper.h"
#include "wordeffectwrapper.h"
#include "wordeffect.h"
#include "wrappermanager.h"
#include "language.h"
#include "languagemanager.h"
#include "playerwrapper.h"

#include "class.h"
#include "core/fenia/feniamanager.h"
#include "skillmanager.h"
#include "flagtable.h"
#include "flagtableregistry.h"
#include "dlscheduler.h"
#include "character.h"
#include "room.h"
#include "core/object.h"
#include "autoquestwrapper.h"
#include "questmanager.h"
#include "questregistrator.h"
#include "questwrapper.h"
#include "behavior.h"
#include "merc.h"

#include "def.h"

#include "subr.h"

using namespace Scripting;

static IconvMap koi2utf("koi8-u", "utf-8");

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

    for (auto &q: areaQuests) {
        if (q.second->wrapper) {
            LogStream::sendNotice() << "Area quest: setting target for " << q.first << endl;
            wrapper_cast<AreaQuestWrapper>(q.second->wrapper)->setTarget(q.second);
        }
    }

    // Autoquest types. Empty on a normal boot, but NOT because QuestManager is
    // missing: feniaroot links libquest_core, so quest_core is pulled in as a
    // dependency here at line 9 instead of arriving with the quest plugins at
    // 28-36, and the manager exists by now. It is the quest TYPE plugins that
    // have not registered yet, so the registry is empty and each type links its
    // own wrapper later, from its own initialization().
    //
    // What this loop is really for is `plug reload feniaroot` on a running
    // server, and the findref sweep in cfindref.cpp, which calls straight into
    // here: the registrators survive holding a Scripting::Object whose handler
    // recovery has just rebuilt with a null target, and without re-pointing them
    // every accessor throws "offline". Same reason the areaQuests loop above
    // exists. The null guard covers a stale installed libfeniaroot.xml that
    // predates the dependency, where quest_core is dlopen'd as DT_NEEDED but
    // initialize_quest_core never runs.
    if (QuestManager::getThis()) {
        for (auto &reg: QuestManager::getThis()->all()) {
            if (reg->wrapper) {
                LogStream::sendNotice() << "Autoquest: setting target for " << reg->getName() << endl;
                wrapper_cast<AutoQuestWrapper>(reg->wrapper)->setTarget(reg.getPointer());
            }
        }
    }

    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        Spell::Pointer spell = skill->getSpell();
        AffectHandler::Pointer ah = skill->getAffect();
        SkillCommand::Pointer cmd = skill->getCommand();
        
        if (spell && spell->wrapper) {
            LogStream::sendNotice() << "Fenia spell: setting target for " << skill->getName() << endl;
            wrapper_cast<SpellWrapper>(spell->wrapper)->setTarget(*spell);
        }

        if (ah && ah->wrapper) {
            LogStream::sendNotice() << "Fenia affect handler: setting target for " << skill->getName() << endl;
            wrapper_cast<AffectHandlerWrapper>(ah->wrapper)->setTarget(*ah);
        }

        if (cmd && cmd->wrapper) {
            LogStream::sendNotice() << "Fenia skill command: setting target for " << skill->getName() << endl;
            wrapper_cast<SkillCommandWrapper>(cmd->wrapper)->setTarget(*cmd);
        }
    }

    // Most commands take their wrapper id from their own help id, which the help
    // manager already keeps unique. Commands that share another command's help
    // (pourout/fill under pour) fall back to a name-hash id in getID(); guard
    // against the astronomically unlikely case that two command ids clash by
    // logging + skipping the loser (it keeps its C++ run(), just no Fenia
    // override), mirroring the word-effect collision handling below.
    std::map<long long, DLString> commandIds;
    for (auto &cmd: commandManager->getCommands()) {
        WrappedCommand *wcmd = cmd.getDynamicPointer<WrappedCommand>();
        if (wcmd && wcmd->wrapper) {
            long long cid = wcmd->getID();
            std::pair<std::map<long long, DLString>::iterator, bool> ins
                = commandIds.insert(std::make_pair(cid, wcmd->getName()));
            if (!ins.second) {
                LogStream::sendError() << "Fenia command id collision: " << wcmd->getName()
                                       << " clashes with " << ins.first->second
                                       << " -- Fenia override disabled for " << wcmd->getName() << endl;
                continue;
            }
            LogStream::sendNotice() << "Fenia command: setting target for " << wcmd->getName() << endl;
            wrapper_cast<FeniaCommandWrapper>(wcmd->wrapper)->setTarget(wcmd);
        }
    }

    for (int i = 0; i < behaviorManager->size(); i++) {
        Behavior *bhv = behaviorManager->find(i);
        if (bhv->wrapper) {
            LogStream::sendNotice() << "Behavior: setting target for " << bhv->getName() << endl;
            wrapper_cast<BehaviorWrapper>(bhv->wrapper)->setTarget(bhv);
        }
    }

    // Word-effects have no persistent id of their own, so stamp each with its
    // language + name (getID() hashes that pair) and bind any persisted Fenia
    // wrapper. Runs on every linkTargets pass -- boot, `plug reload feniaroot`,
    // and the findref sweep -- so it must stay idempotent. A hash collision is
    // logged and the loser skipped (it keeps its C++ effect, just can't be
    // Fenia-overridden) rather than crashing a running server.
    if (languageManager) {
        std::map<long long, DLString> effectIds;
        for (auto &lpair: languageManager->getLanguages()) {
            Language::Pointer lang = lpair.second;
            if (!lang)
                continue;
            for (auto &epair: lang->getEffects()) {
                WordEffect *effect = epair.second.getPointer();
                if (!effect)
                    continue;

                effect->setEffectIdentity(lang->getName(), epair.first);
                DLString tag = lang->getName() + ":" + epair.first;
                long long eid = effect->getID();

                std::pair<std::map<long long, DLString>::iterator, bool> ins
                    = effectIds.insert(std::make_pair(eid, tag));
                if (!ins.second) {
                    LogStream::sendError() << "Word-effect id collision: " << tag
                                           << " clashes with " << ins.first->second
                                           << " -- Fenia override disabled for " << tag << endl;
                    continue;
                }

                WrapperManager::getThis()->linkWrapper(effect);
                if (effect->wrapper)
                    LogStream::sendNotice() << "Word-effect: linked wrapper for " << tag << endl;
            }
        }
    }
}

static void dumpTables(Json::Value &apiDump)
{
    Json::Value tables;

    for (auto &pair: FlagTableRegistry::getNamesMap()) {
        const DLString &name = pair.first;
        const FlagTable *table = pair.second;
        const FlagTable::Field *fields = table->fields;

        Json::Value tableEntries;

        for(int i = 0; i < table->size; i++) {
            Json::Value entry;
            entry["name"] = fields[i].name;
            entry["msg"] = fields[i].message ? fields[i].message : "";

            if (table->enumerated)
                entry["value"] = fields[i].value;
            else
                entry["value"] = static_cast<Json::Value::UInt64>(table->bitstring(fields[i].name));

            tableEntries.append(entry);
        }

        tables[name] = tableEntries;
    }

    apiDump["tables"] = tables;
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
    Class::regMoc<MaterialWrapper>( );
    Class::regMoc<AffectHandlerWrapper>( );
    Class::regMoc<AffectWrapper>( );
    Class::regMoc<SkillCommandWrapper>( );
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
    Class::regMoc<WearlocWrapper>( );
    Class::regMoc<SkillWrapper>( );
    Class::regMoc<SkillGroupWrapper>( );
    Class::regMoc<FeniaCommandWrapper>( );
    Class::regMoc<AreaQuestWrapper>( );
    Class::regMoc<AutoQuestWrapper>( );
    Class::regMoc<BehaviorWrapper>();
    Class::regMoc<WordEffectWrapper>();
    Class::regMoc<PlayerWrapper>();
    
    FeniaManager::getThis( )->recover( );
        
    DLScheduler::getThis()->putTaskNOW( ValidateTask::Pointer(NEW) );

    linkTargets();

    Scripting::gc = true;

    if (dreamland->hasOption(DL_BUILDPLOT))
        return;

    LogStream::sendNotice() << "Dumping Fenia API to disk." << endl;

    Json::Value apiDump;
    traitsAPIJson<CharacterWrapper>("char", apiDump, true);     
    traitsAPIJson<PlayerWrapper>("player", apiDump, true);     
    traitsAPIJson<ObjectWrapper>("obj", apiDump, true);     
    traitsAPIJson<RoomWrapper>("room", apiDump, true);     
    traitsAPIJson<MobIndexWrapper>("mob_index", apiDump, false);     
    traitsAPIJson<ObjIndexWrapper>("obj_index", apiDump, false);     
    traitsAPIJson<AreaIndexWrapper>("area_index", apiDump, false);    
    traitsAPIJson<Root>("root", apiDump, true);     
    traitsAPIJson<AffectWrapper>("affect", apiDump, false);     
    traitsAPIJson<AreaWrapper>("area", apiDump, false);     
    traitsAPIJson<HometownWrapper>("hometown", apiDump, false);     
    traitsAPIJson<ProfessionWrapper>("profession", apiDump, false);     
    traitsAPIJson<RaceWrapper>("race", apiDump, false);     
    traitsAPIJson<ClanWrapper>("clan", apiDump, false);     
    traitsAPIJson<CraftProfessionWrapper>("craftprofession", apiDump, false);     
    traitsAPIJson<BonusWrapper>("bonus", apiDump, false);     
    traitsAPIJson<ReligionWrapper>("religion", apiDump, false);     
    traitsAPIJson<LiquidWrapper>("liquid", apiDump, false);     
    traitsAPIJson<WearlocWrapper>("wearloc", apiDump, false);     
    traitsAPIJson<MaterialWrapper>("material", apiDump, false);
    traitsAPIJson<SkillWrapper>("skill", apiDump, false);     
    traitsAPIJson<SkillGroupWrapper>("skillgroup", apiDump, false);     
    traitsAPIJson<SpellWrapper>("spell", apiDump, false);
    traitsAPIJson<AffectHandlerWrapper>("affecthandler", apiDump, false);
    traitsAPIJson<SkillCommandWrapper>("skillcommand", apiDump, false);
    traitsAPIJson<FeniaSpellContext>("spellcontext", apiDump, false);
    traitsAPIJson<FeniaCommandContext>("commandcontext", apiDump, false);
    traitsAPIJson<FeniaString>("string", apiDump, false);
    traitsAPIJson<FeniaCommandWrapper>("command", apiDump, false);     
    traitsAPIJson<AreaQuestWrapper>("areaquest", apiDump, false);     
    traitsAPIJson<AutoQuestWrapper>("autoquest", apiDump, false);
    // Both are built per call and thrown away, like spellcontext above, so they
    // are documented here but never registered as wrappers -- quest_core owns
    // their moc registration.
    traitsAPIJson<QuestWrapper>("quest", apiDump, false);
    traitsAPIJson<QuestRewardWrapper>("questreward", apiDump, false);
    traitsAPIJson<QuestSelectWrapper>("questselection", apiDump, false);
    traitsAPIJson<BehaviorWrapper>("behavior", apiDump, false);
    traitsAPIJson<WordEffectWrapper>("wordeffect", apiDump, false);
    dumpTables(apiDump);

    Json::FastWriter writer;
    DLFileStream("/tmp", "feniaapi", ".json").fromString(
        koi2utf(
            writer.write(apiDump))
    );
}

void WrappersPlugin::unlinkTargets()
{
}

void WrappersPlugin::destruction( ) {
    DLScheduler::getThis()->slay( ValidateTask::Pointer(NEW) );

    unlinkTargets();

    Scripting::gc = false;
    FeniaManager::getThis( )->backup( );

    Class::unregMoc<PlayerWrapper>();
    Class::unregMoc<BehaviorWrapper>();
    Class::unregMoc<WordEffectWrapper>();
    Class::unregMoc<AutoQuestWrapper>( );
    Class::unregMoc<AreaQuestWrapper>( );
    Class::unregMoc<WearlocWrapper>( );
    Class::unregMoc<LiquidWrapper>( );
    Class::unregMoc<SkillGroupWrapper>( );
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
    Class::unregMoc<SkillCommandWrapper>( );
    Class::unregMoc<FeniaCommandWrapper>( );
    Class::unregMoc<AffectWrapper>( );
    Class::unregMoc<AreaIndexWrapper>( );
    Class::unregMoc<SpellWrapper>( );
    Class::unregMoc<MaterialWrapper>( );
    Class::unregMoc<AffectHandlerWrapper>( );
    Class::unregMoc<ObjIndexWrapper>( );
    Class::unregMoc<MobIndexWrapper>( );
    Class::unregMoc<CharacterWrapper>( );
    Class::unregMoc<ObjectWrapper>( );
    Class::unregMoc<RoomWrapper>( );
    Class::unregMoc<Root>( );
}

