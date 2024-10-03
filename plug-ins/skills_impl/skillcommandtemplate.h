/* $Id: skillcommandtemplate.h,v 1.1.2.7 2010-09-01 21:20:47 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef        __SKILLCOMMANDTEMPLATE_H__
#define __SKILLCOMMANDTEMPLATE_H__

// MOC_SKIP_BEGIN
#include "defaultskillcommand.h"
#include "plugininitializer.h"
#include "classselfregistratorplugin.h"

template <const char *&tn>
struct SkillCommandTemplate : public DefaultSkillCommand, public ClassSelfRegistratorPlugin<tn> {
    typedef ::Pointer<SkillCommandTemplate> Pointer;

    SkillCommandTemplate( ) {
        name[EN] = cmdName;
    }

    virtual void run( Character * ch, char *argument ) { 
        DefaultSkillCommand::run( ch, argument );
    }
    virtual bool applyLegacy(Character * ch, Character *victim, int level) {
        return DefaultSkillCommand::applyLegacy(ch, victim, level);
    }
    virtual bool visible(Character *ch) const {
        return DefaultSkillCommand::visible(ch);
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
private:
    static const char *cmdName;
};

#define SKILL_DUMMY(x)         dummySkill_ ##x## _TypeName
#define SKILL(x) SkillCommandTemplate<SKILL_DUMMY(x)>

#define SKILL_DECL(x) \
const char *SKILL_DUMMY(x) = "SKILL(" #x ")"; \
template<> const char *SKILL(x)::cmdName = #x; \
PluginInitializer<SKILL(x)> dummySkill_ ##x## _init;

#define SKILL_RUNP(x) \
SKILL_DECL(x); \
template <> \
void SKILL(x)::run( Character* ch, char *argument ) 

#define SKILL_APPLY(x) \
template <> bool SKILL(x)::applyLegacy(Character * ch, Character *victim, int level)

// MOC_SKIP_END
#endif
