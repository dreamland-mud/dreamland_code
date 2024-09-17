#include "skill_alloc.h"
#include "skillmanager.h"
#include "skillreference.h"
#include "xmltableloader.h"
#include "logstream.h"
#include "helpmanager.h"

GSN(garble);
GSN(kassandra);
GSN(rear_kick);
GSN(sanctuary);

BasicSkill::Pointer SkillAlloc::newSkill(const DLString &skillName, Skill *refSkill, const DLString &className)
{
    BasicSkill::Pointer newSkill;

    if (skillManager->findExisting(skillName))
        return newSkill;

    XMLTableLoader *loader = dynamic_cast<BasicSkill *>(refSkill)->getLoader();
    
    if (!loader)
        return newSkill;

    try {
        AllocateClass::Pointer alloc = Class::allocateClass(className);
        newSkill = alloc.getDynamicPointer<BasicSkill>();
    }
    catch (const ExceptionClassNotFound &e) {
        LogStream::sendError() << "SkillAlloc: " << e.what() << endl;
        return newSkill;
    }

    newSkill->setName(skillName);
    newSkill->help.construct();
    newSkill->help->setID(
        helpManager->getLastID() + 1
    );

    loader->loadElement(newSkill);
    loader->saveElement(newSkill);

    return newSkill;
}

// Figure out class name via 'reflection', without depending on corresponding plugin directly.
// Figure out who loads that type of skills, by looking at a typical example.

BasicSkill::Pointer SkillAlloc::newClassSkill(const DLString &name)
{
    return newSkill(name, gsn_sanctuary.getElement(), "GenericSkill");
}

BasicSkill::Pointer SkillAlloc::newClanSkill(const DLString &name)
{
    return newSkill(name, gsn_garble.getElement(), "ClanSkill");
}

BasicSkill::Pointer SkillAlloc::newOrdenSkill(const DLString &name)
{
    return newSkill(name, gsn_garble.getElement(), "ClanOrgSkill");
}

BasicSkill::Pointer SkillAlloc::newRaceSkill(const DLString &name)
{
    return newSkill(name, gsn_rear_kick.getElement(), "RaceAptitude");
}

BasicSkill::Pointer SkillAlloc::newOtherSkill(const DLString &name)
{
    return newSkill(name, gsn_kassandra.getElement(), "BasicSkill");
}
