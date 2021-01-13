#include "stringlist.h"
#include "grammar_entities_impl.h"
#include "craftattribute.h"
#include "subprofession.h"
#include "pcharacter.h"

XMLAttributeCraft::XMLAttributeCraft( )
{
}

XMLAttributeCraft::~XMLAttributeCraft( )
{
}


bool XMLAttributeCraft::handle( const WhoisArguments &args )
{
    ostringstream buf;

    if (proficiency.empty())
        return false;

    buf << "владеет профессиями: ";
    
    StringList pnames;
    Proficiency::const_iterator p;
    for (p = proficiency.begin(); p != proficiency.end(); p++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(p->first);
        if (prof)
            pnames.push_back(prof->getNameFor(args.looker));
    }
    buf << pnames.join(", ");
    
    if (!pnames.empty())
        args.lines.push_back( buf.str() );

    return !pnames.empty();
}

bool XMLAttributeCraft::handle( const ScoreArguments &args )
{
    if (proficiency.empty())
        return false;

    Proficiency::const_iterator p;

    for (p = proficiency.begin(); p != proficiency.end(); p++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(p->first);
        if (prof) {
            ostringstream buf;
            ExperienceCalculator::Pointer calc = prof->getCalculator(args.pch);

            buf << "Профессия: " << prof->getNameFor(args.pch) << " уровня " << p->second.level
                << ", опыта до уровня " << calc->expToLevel() << "/" << calc->expThisLevel();
            args.lines.push_back( buf.str() );
        }
    }

    return !args.lines.empty();
}

int XMLAttributeCraft::proficiencyLevel(const DLString &profName) const
{
    Proficiency::const_iterator p = proficiency.find(profName);
    if (p == proficiency.end())
        return 0;
        
    return p->second.level;
}

bool XMLAttributeCraft::learned(const DLString &profName) const
{
    Proficiency::const_iterator p = proficiency.find(profName);
    return p != proficiency.end();
}

void XMLAttributeCraft::setProficiencyLevel(const DLString &profName, int level)
{
    proficiency[profName].level = level;
}

int XMLAttributeCraft::exp(const DLString &profName) const
{
    Proficiency::const_iterator p = proficiency.find(profName);
    if (p == proficiency.end())
        return 0;
        
    return p->second.exp;
}

int XMLAttributeCraft::gainExp(const DLString &profName, int xp)
{
    Proficiency::iterator p = proficiency.find(profName);
    if (p == proficiency.end()) {
        CraftProficiency craftProf;
        craftProf.level = 1;
        craftProf.exp = xp;
        proficiency[profName] = craftProf;
        return xp;
    }

    p->second.exp += xp;
    return p->second.exp;
}

