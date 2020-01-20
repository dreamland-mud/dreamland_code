/* $Id: genericskillloader.cpp,v 1.1.2.4.10.1 2007/06/26 07:15:12 rufina Exp $
 *
 * ruffina, 2004
 */

#include "so.h"
#include "logstream.h"
#include "genericskillloader.h"
#include "genericskill.h"
#include "profession.h"
#include "dlfilestream.h"
#include "stringlist.h"

const DLString GenericSkillLoader::TABLE_NAME = "generic-skills";
const DLString GenericSkillLoader::NODE_NAME = "skill";

void GenericSkillLoader::initialization( )
{
    XMLTableLoaderPlugin::initialization( );
    resolveAll( );
    dumpCSV();
}

void GenericSkillLoader::destruction( )
{
    unresolveAll( );
    XMLTableLoaderPlugin::destruction( );
}

void GenericSkillLoader::resolveAll( )
{
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
        GenericSkill *skill = e->getStaticPointer<GenericSkill>( );

        skill->resolve( );
    }
}

void GenericSkillLoader::unresolveAll( )
{
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
        GenericSkill *skill = e->getStaticPointer<GenericSkill>( );

        skill->unresolve( );
    }
}

/** Create a CSV file with all professional skills and their levels. */
void GenericSkillLoader::dumpCSV()
{
    ostringstream buf;
    StringList profs;
    
    // Header: name,warrior,cleric,witch,...
    for (int i = 0; i < professionManager->size( ); i++) {
        Profession *prof = professionManager->find( i );
        if (prof->isValid( ) && prof->isPlayed( ) 
            && prof->getName() != "druid" && prof->getName() != "universal")
        {
            profs.push_back(prof->getName());
        }
    }
    buf << "name," << profs.join(",") << endl;

    // Body: armor,,1,1,...
    for (LoadedList::iterator e = elements.begin( ); e != elements.end( ); e++) {
        GenericSkill *skill = e->getStaticPointer<GenericSkill>( );

        if (skill->classes.empty())
            continue;

        ostringstream levels;
        bool found = false;

        for (auto &profName : profs) {
            Profession *prof = professionManager->find( profName );
            const auto &c = skill->classes.find(prof->getName());
            if (c != skill->classes.end()) {
                levels << "," << c->second.getLevel();
                found = true;
            }
            else
                levels << ",";
        }

        if (found)
            buf << skill->getName() << levels.str() << endl;
    }

    try {
        DLFileStream( "/tmp/class_skills.csv" ).fromString( buf.str( ) );
    } catch (const ExceptionDBIO &ex) {
        LogStream::sendError() << ex.what() << endl;
    }
}

extern "C"
{
        SO::PluginList initialize_genericskill_loader( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<GenericSkillLoader>( ppl );
                
                return ppl;
        }
        
}
