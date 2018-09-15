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
    
    Proficiency::const_iterator p;
    bool found = false;

    for (p = proficiency.begin(); p != proficiency.end(); p++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(p->first);
        if (prof) {
            if (found)
                buf << ", ";
            buf << prof->getNameFor(args.looker);
            found = true;
        }
    }
    
    if (found)
        args.lines.push_back( buf.str() );

    return found;
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
            buf << "Дополнительная профессия: " << prof->getNameFor(args.pch) << " уровня " << p->second.level;
            args.lines.push_back( buf.str() );
        }
    }

    return !args.lines.empty();
}

int XMLAttributeCraft::proficiencyLevel(const CraftProfession &prof) const
{
    return proficiencyLevel(prof.getName());
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

