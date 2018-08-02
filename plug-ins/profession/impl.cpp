/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "class.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "defaultprofession.h"
#include "mobileprofession.h"

TABLE_LOADER(ProfessionLoader, "professions", "Profession");

class ProfessionRegistrator : public Plugin {
public:
    virtual void initialization( )
    {
	Class::regXMLVar<ProfessionHelp>( );
	Class::regXMLVar<ProfessionTitlesByLevel>( );
	Class::regMoc<ProfessionTitlesByConstant>( );
	Class::regMoc<DefaultProfession>( );
	Class::regMoc<MobileProfession>( );
    }

    virtual void destruction( )
    {
	Class::unregXMLVar<ProfessionHelp>( );
	Class::unregXMLVar<ProfessionTitlesByLevel>( );
	Class::unregMoc<ProfessionTitlesByConstant>( );
	Class::unregMoc<DefaultProfession>( );
	Class::unregMoc<MobileProfession>( );
    }
};


extern "C"
{
    SO::PluginList initialize_profession( ) {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<ProfessionRegistrator>( ppl );
	Plugin::registerPlugin<ProfessionLoader>( ppl );
	
	return ppl;
    }
}

