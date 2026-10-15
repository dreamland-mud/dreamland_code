/* Dream Land, 2026 */
#ifndef QUESTTYPESPLUGIN_H
#define QUESTTYPESPLUGIN_H

#include <vector>
#include "plugin.h"
#include "questregistrator.h"

/** Registers one autoquest type per quests/<Name>.xml config file.
 *
 *  Every *.xml in that dir becomes a type, so a copy like KillQuest-old.xml
 *  would register a second type with the same feniaId. Keep backups out of it.
 *
 *  🛑 Register it from a plugin listed in plugin.xml AFTER feniaroot
 *  (quest_command), never from quest_core: quest_core is loaded as a
 *  dependency of feniaroot, before FeniaManager::wrapperManager exists, so its
 *  types would link no Fenia wrapper and every quest would fail at onCreate.
 */
class QuestTypesPlugin : public Plugin {
public:
    typedef ::Pointer<QuestTypesPlugin> Pointer;

protected:
    virtual void initialization( );
    virtual void destruction( );

private:
    std::vector<QuestRegistratorBase::Pointer> types;
};

#endif
