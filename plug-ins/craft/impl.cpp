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
#include "l10n.h"
#include "pcharacter.h"
#include "hidetrackshook.h"

TABLE_LOADER(CraftProfessionLoader, "craft-professions", "CraftProfession");
TABLE_LOADER(CraftWearlocLoader, "craft-wearlocs", "Wearlocation");

AFFECT_DECL(RemoveTattoo);
VOID_AFFECT(RemoveTattoo)::update( Object *obj, Affect *paf ) 
{ 
    DefaultAffectHandler::update( obj, paf );

    if (obj->carried_by) {
        if (chance(50))
            obj->carried_by->pecho(_("%^O1 бледнеет."), obj);
        else 
            obj->carried_by->pecho(_("Краски на %O6 постепенно тускнеют."), obj);
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


/*
 * A Beastmaster from craft level 5 leaves no footprint on (L - 4) * 17
 * percent of steps, on every step at 10. Runs on every player step, so it
 * only reads the attribute, never creates it. Fenia twin, used by the steam
 * compass: .tmp.pet.hidesTracks (dreamland_fenia utils/pet: ai).
 */
static bool beastmaster_hides_tracks( Character *ch )
{
    if (ch->is_npc( ))
        return false;

    XMLAttributeCraft::Pointer attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeCraft>( "craft" );
    if (!attr)
        return false;

    int lvl = attr->proficiencyLevel( "beastmaster" );
    if (lvl < 5)
        return false;

    return number_percent( ) <= min( 100, (lvl - 4) * 17 );
}

/** Plugs the Beastmaster's hidden trail into movement while this library is loaded. */
class CraftHideTracksPlugin : public Plugin {
public:
    virtual void initialization( )
    {
        hide_tracks_hook = &beastmaster_hides_tracks;
    }

    virtual void destruction( )
    {
        hide_tracks_hook = 0;
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
        Plugin::registerPlugin<CraftHideTracksPlugin>( ppl );
        
        return ppl;
    }
}


