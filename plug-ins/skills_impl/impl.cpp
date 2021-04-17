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

#include "feniaskillaction.h"
#include "basicskill.h"
#include "summoncreaturespell.h"
#include "transportspell.h"
#include "sleepaffecthandler.h"
#include "skillhelp.h"
#include "skillgrouphelp.h"
#include "xmlattributerestring.h"
#include "def.h"

TABLE_LOADER(SkillGroupLoader, "skill-groups", "SkillGroup");

extern "C"
{
        SO::PluginList initialize_skills_impl( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<MocRegistrator<BasicSkill> >( ppl );
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
                Plugin::registerPlugin<MocRegistrator<FeniaSpellContext> > (ppl);
                return ppl;
        }
        
}
