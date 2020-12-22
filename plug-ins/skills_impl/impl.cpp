/* $Id: impl.cpp,v 1.1.2.4.18.9 2009/09/01 22:29:52 rufina Exp $
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "so.h"
#include "mocregistrator.h"
#include "xmlvariableregistrator.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "xmlattributeplugin.h"
#include "mobilebehaviorplugin.h"
#include "defaultaffecthandler.h"
#include "defaultskillgroup.h"
#include "spellmanager.h"
#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "skillmanager.h"
#include "dreamland.h"

#include "summoncreaturespell.h"
#include "transportspell.h"
#include "sleepaffecthandler.h"
#include "skillhelp.h"
#include "skillgrouphelp.h"
#include "xmlattributerestring.h"
#include "mercdb.h"
#include "def.h"

TABLE_LOADER(SkillGroupLoader, "skill-groups", "SkillGroup");

/** 
 * Destroy summoned mobs once a day if the owner no longer has access to the corresponding skill.
 * Move golems to a storage location if their owner is offline for more than a day.
 * Mobs in mansions are not affected.
 */
class CreaturePurgeTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<CreaturePurgeTask> Pointer;

    virtual void run()
    {
        map<int, SummonCreatureSpell::Pointer> vnum2skill;

        for (int sn = 0; sn < skillManager->size(); sn++) {
            Skill *skill = skillManager->find(sn);
            if (!skill->getSpell())
                continue;

            SummonCreatureSpell::Pointer spell = skill->getSpell().getDynamicPointer<SummonCreatureSpell>();
            if (spell)                            
                vnum2skill[spell->getMobVnum()] = spell;
        }

        Character *wch_next;
        for (Character *wch = char_list; wch; wch = wch_next) {
            SummonedCreature::Pointer creature;
            wch_next = wch->next;

            if (!wch->is_npc() || wch->master)
                continue;

            if (IS_SET(wch->in_room->room_flags, ROOM_MANSION))
                continue;

            if (!wch->getNPC()->behavior)
                continue;
            if (!(creature = wch->getNPC()->behavior.getDynamicPointer<SummonedCreature>()))
                continue;

            int vnum = wch->getNPC()->pIndexData->vnum;
            PCMemoryInterface *owner = PCharacterManager::find(creature->creatorName);
            if (!owner) {
                notice("Mob %d in room [%d] [%s] extracted, no owner.", vnum, wch->in_room->vnum, wch->in_room->getName());
                continue;
            }

            auto v2s = vnum2skill.find(vnum);
            if (v2s == vnum2skill.end()) {
                LogStream::sendWarning() << "Odd summoned creature " << vnum << " without a skill." << endl;
                continue;
            }

            SummonCreatureSpell::Pointer spell = v2s->second;
            Skill::Pointer skill = spell->getSkill();
            if (!skill->visible(owner)) {
                notice("Mob %d in room [%d] [%s] extracted, owner %s (%s, clan %s) doesn't know skill %s.",
                        vnum, wch->in_room->vnum, wch->in_room->getName(), 
                        owner->getName().c_str(), owner->getProfession()->getName().c_str(),
                        owner->getClan()->getName().c_str(), skill->getName().c_str());
                continue;
            }

            if (spell->getStorageVnums().empty())
                continue;

            if (owner->isOnline())
                continue;

            if (dreamland->getCurrentTime() - owner->getLastAccessTime().getTime() <= Date::SECOND_IN_DAY)
                continue;

            int rvnum = spell->getStorageVnums().randomNumber();
            Room *room = get_room_instance(rvnum);
            if (!room) {
                bug("Spell %s has invalid storage vnum %d.", skill->getName().c_str(), rvnum);
                continue;
            }

            notice("Mob %d moved from room [%d] [%s] to storage room [%d], owner %s offline for more than a day", 
                    vnum, wch->in_room->vnum, wch->in_room->getName(),
                    rvnum, owner->getName().c_str());
        }
    }

    virtual void after()
    {
        DLScheduler::getThis()->putTaskInSecond(Date::SECOND_IN_DAY, Pointer(this));
    }

    virtual int getPriority() const
    {
        return SCDP_ROUND + 90;
    }
};

extern "C"
{
        SO::PluginList initialize_skills_impl( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<SpellManager>( ppl );
                Plugin::registerPlugin<MocRegistrator<DefaultAffectHandler> >( ppl );                
                Plugin::registerPlugin<MocRegistrator<DefaultSkillGroup> >( ppl );                
                Plugin::registerPlugin<XMLVariableRegistrator<SkillGroupHelp> >( ppl );
                Plugin::registerPlugin<SkillGroupLoader>( ppl );
                Plugin::registerPlugin<MobileBehaviorRegistrator<SummonedCreature> >( ppl );
                Plugin::registerPlugin<MocRegistrator<DefaultSpell> >( ppl );                
                Plugin::registerPlugin<MocRegistrator<GateSpell> >( ppl );                
                Plugin::registerPlugin<MocRegistrator<SummonSpell> >( ppl );                
                Plugin::registerPlugin<MocRegistrator<AnatoliaCombatSpell> >( ppl );
                Plugin::registerPlugin<MocRegistrator<SleepAffectHandler> >( ppl );                
                Plugin::registerPlugin<XMLVariableRegistrator<SkillHelp> >( ppl );
                Plugin::registerPlugin<XMLAttributeVarRegistrator<XMLAttributeRestring> >( ppl );
                Plugin::registerPlugin<CreaturePurgeTask>( ppl );
                
                return ppl;
        }
        
}
