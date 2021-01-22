#include "logstream.h"
#include "feniaspellhelper.h"
#include "feniamanager.h"
#include "skillmanager.h"
#include "skill.h"
#include "spell.h"

void FeniaSpellHelper::linkWrappers()
{
    for (int sn = 0; sn < skillManager->size(); sn++) {
        Spell::Pointer spell = skillManager->find(sn)->getSpell();
        if (spell)
            linkWrapper(*spell);
    }
}

void FeniaSpellHelper::extractWrappers() 
{
    for (int sn = 0; sn < skillManager->size(); sn++) {
        Spell::Pointer spell = skillManager->find(sn)->getSpell();
        if (spell)
            extractWrapper(*spell);
    }    
}

void FeniaSpellHelper::linkWrapper(Spell *spell) 
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when linking spell wrapper for " << spell->getSkill()->getName() << endl;
        return;
    }

    FeniaManager::wrapperManager->linkWrapper(spell);
    if (spell->wrapper)
        LogStream::sendNotice() << "Fenia spell: linked wrapper for " << spell->getSkill()->getName() << endl;
}

void FeniaSpellHelper::extractWrapper(Spell *spell) 
{
    if (!spell->wrapper)
        return;
        
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when extracting spell wrapper for " << spell->getSkill()->getName() << endl;
        return;
    }

    spell->extractWrapper(false);
}

