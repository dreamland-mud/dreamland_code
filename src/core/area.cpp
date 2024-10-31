#include "area.h"
#include "grammar_entities_impl.h"
#include "json_utils.h"
#include "dreamland.h"
#include "autoflags.h"
#include "def.h"

int top_area;

struct area_file * area_file_list;

AreaVector areaInstances;

AreaIndexVector areaIndexes;

struct area_file *
new_area_file(const char *name)
{
    struct area_file *rc = new area_file;

    rc->file_name = name;
        
    rc->next = area_file_list;
    area_file_list = rc;
    return rc;
}

AreaIndexData::AreaIndexData()
    : 
      low_range(0), high_range(0),
      min_vnum(0), max_vnum(0),
      count(0),
      area_flag(0),
      behavior(AreaBehavior::NODE_NAME),
      security(9), vnum(0), changed(false),
      wrapper(0),
      area(0)
{
}

Area * AreaIndexData::create()
{
    if (area) // FIXME allow multiple instances
        throw Exception("Attempt to create second instance of an area.");

    area = new Area;
    area->pIndexData = this;
    area->area_flag = area_flag;

    areaInstances.push_back(area);
    
    return area;
}

DLString AreaIndexData::getName(char gcase) const
{
    return name.get(LANG_DEFAULT).ruscase(gcase);
}

Area::Area()
    : empty(true), age(15), nplayer(0),
      area_flag(0), pIndexData(0)
{
}


AreaIndexData * get_area_index(const DLString &filename)
{
    for(auto &area: areaIndexes)
        if (filename == area->area_file->file_name)
            return area;
            
    return 0;
}

