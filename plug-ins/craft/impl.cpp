#include "so.h"
#include "class.h"
#include "dlxmlloader.h"
#include "xmltableloaderplugin.h"
#include "xmlattributeplugin.h"
#include "subprofession.h"
#include "craftattribute.h"

TABLE_LOADER(SubProfessionLoader, "subprofessions", "SubProfession");

class SubProfessionRegistrator : public Plugin {
public:
    virtual void initialization( )
    {
	Class::regXMLVar<SubProfessionHelp>( );
	Class::regMoc<SubProfession>( );
    }

    virtual void destruction( )
    {
	Class::unregXMLVar<SubProfessionHelp>( );
	Class::unregMoc<SubProfession>( );
    }
};


extern "C"
{
    SO::PluginList initialize_craft( ) {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<SubProfessionRegistrator>( ppl );
	Plugin::registerPlugin<SubProfessionLoader>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCraft> >( ppl );
	
	return ppl;
    }
}


