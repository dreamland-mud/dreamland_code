#ifndef AREA_H
#define AREA_H

#include "fenia/register-decl.h"
#include "grammar_entities.h"
#include "xmlmultistring.h"
#include "xmlstreamable.h"
#include "helpmanager.h"
#include "areabehavior.h"
#include "areaquest.h"

class RoomIndexData;
class Room;
struct Area;
class AreaIndexData;

// Keeps track of all areas loaded; used to assign area's vnum field.
extern int        top_area; 

typedef vector<Area *> AreaVector;
// All area instances are kept here
extern AreaVector areaInstances;

typedef vector<AreaIndexData *> AreaIndexVector;
// All area prototypes are kept here
extern AreaIndexVector areaIndexes;

// MOC_SKIP_BEGIN
struct area_file {
    struct area_file *next;
    struct AreaIndexData *area;
    DLString file_name;
};

extern struct area_file * area_file_list;
struct area_file * new_area_file(const char *name);
// MOC_SKIP_END

/*
 * Area definition.
 */
struct AreaIndexData {
    AreaIndexData();

    Area *create();

    DLString getName(char gcase = '1') const;

    XMLMultiString name; // main area name in all languages
    XMLMultiString altname; // alternative names for this area
    DLString authors;
    DLString translator;
    XMLMultiString speedwalk;
    int low_range;
    int high_range;
    int min_vnum;
    int max_vnum;
    unsigned long count;
    XMLMultiString resetMessage;
    int area_flag;
    struct area_file *area_file;
    XMLPersistentStreamable<AreaBehavior> behavior;
    HelpArticles helps;

    /*OLC*/
    int security;
    int vnum;
    bool changed;
    map<int, RoomIndexData *> roomIndexes;

    Scripting::Object *wrapper;

    list<XMLPointer<AreaQuest>> quests;
    map<int, AreaQuest *> questMap;

    // FIXME: support multiple named instances.
    Area *area;
};

struct Area {
    Area();

    bool empty;
    int age;
    int nplayer;
    int area_flag;
    map<int, Room *> rooms;

    AreaIndexData *pIndexData;
};

// Look up area prototype by its unique filename.
AreaIndexData * get_area_index(const DLString &filename);

#endif
