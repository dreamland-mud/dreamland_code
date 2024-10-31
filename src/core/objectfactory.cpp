#include "objectfactory.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "json_utils.h"
#include "race.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "religion.h"
#include "behavior.h"
#include "dlscheduler.h"
#include "dreamland.h"
#include "flagtable.h"
#include "autoflags.h"
#include "dl_strings.h"
#include "def.h"

OBJ_INDEX_DATA * obj_index_hash[1024];

obj_index_data::obj_index_data()
                : behaviors(behaviorManager)
{
    next = NULL;
    area = NULL;
    vnum = 0;
    reset_num = 0;
    material = "none";
    item_type = ITEM_TRASH;
    extra_flags = 0;
    wear_flags = 0;
    level = 0;
    condition = 100;
    count = 0;
    weight = 0;
    cost = 0;

    for (int i = 0; i < 5; i++)
        value[i] = 0;
        
    limit = -1;
    wrapper = 0;
    area = 0;
}

obj_index_data::~obj_index_data()
{
    
}

DLString obj_index_data::getProperty(const DLString &key) const
{
    // Look in props on index data: props[key] or props["blablah"][key]
    DLString jsonValue;
    JsonUtils::findValue(props, key, jsonValue);
    return jsonValue;
}

const char * obj_index_data::getDescription( lang_t lang ) const
{
    return description.get(lang).c_str();
}

const char * obj_index_data::getShortDescr( lang_t lang ) const
{
    return short_descr.get(lang).c_str();
}


OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % 1024];
          pObjIndex != 0;
          pObjIndex  = pObjIndex->next )
    {
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;
    }

    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
	    LogStream::sendError() << "get_obj_index: vnum " << vnum << " not found on world startup" << endl;

    return 0;
}



