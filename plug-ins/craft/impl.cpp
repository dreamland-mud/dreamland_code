#include "so.h"
#include "logstream.h"
#include "class.h"
#include "dlxmlloader.h"
#include "affecthandlertemplate.h"
#include "xmltableloaderplugin.h"
#include "xmlattributeplugin.h"
#include "mocregistrator.h"
#include "character.h"
#include "object.h"
#include "subprofession.h"
#include "craftattribute.h"
#include "craftskill.h"
#include "craftwearloc.h"
#include "dl_math.h"

TABLE_LOADER(CraftProfessionLoader, "craft-professions", "CraftProfession");
TABLE_LOADER(CraftWearlocLoader, "craft-wearlocs", "Wearlocation");

AFFECT_DECL(RemoveTattoo);
VOID_AFFECT(RemoveTattoo)::update( Object *obj, Affect *paf ) 
{ 
    DefaultAffectHandler::update( obj, paf );

    if (obj->carried_by) {
        if (chance(50))
            obj->carried_by->pecho("%^O1 бледнеет.", obj);
        else 
            obj->carried_by->pecho("Краски на %O6 постепенно тускнеют.", obj);
    }
}

class CraftProfessionRegistrator : public Plugin {
public:
    virtual void initialization( )
    {
        Class::regXMLVar<CraftProfessionHelp>( );
        Class::regMoc<CraftProfession>( );
    }

    virtual void destruction( )
    {
        Class::unregXMLVar<CraftProfessionHelp>( );
        Class::unregMoc<CraftProfession>( );
    }
};


extern "C"
{
    SO::PluginList initialize_craft( ) {
        SO::PluginList ppl;
        
        Plugin::registerPlugin<CraftProfessionManager>( ppl );
        Plugin::registerPlugin<CraftProfessionRegistrator>( ppl );
        Plugin::registerPlugin<CraftProfessionLoader>( ppl );
        Plugin::registerPlugin<MocRegistrator<CraftSkill> >( ppl );
        Plugin::registerPlugin<MocRegistrator<CraftTattooWearloc> >( ppl );
        Plugin::registerPlugin<CraftWearlocLoader>( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeCraft> >( ppl );
        
        return ppl;
    }
}


