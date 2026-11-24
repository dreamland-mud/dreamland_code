#include <map>

#include "roomtempdescr.h"
#include "room.h"

/*
 * Kept here rather than on Room so that changing it needs no core rebuild:
 * comm (look, write) and feniaroot (room methods) both link this plugin.
 * Reloading this plugin empties the map; the owners refill it.
 */
struct RoomTempEntry {
    RoomTempEntry( ) { }
    ~RoomTempEntry( ) { eds.deallocate( ); }
    RoomTempEntry( const RoomTempEntry & ) = delete;
    RoomTempEntry & operator = ( const RoomTempEntry & ) = delete;

    ExtraDescrList eds;
    // Keyed by the exact keyword string of an entry in eds.
    std::map<DLString, XMLMultiString> marks;
};

static std::map<Room *, RoomTempEntry> roomTempEntries;

const ExtraDescrList & RoomTempDescr::list( Room *room )
{
    static const ExtraDescrList empty;

    auto e = roomTempEntries.find( room );
    if (e == roomTempEntries.end( ))
        return empty;

    return e->second.eds;
}

ExtraDescription * RoomTempDescr::find( Room *room, const DLString &keyword )
{
    return list( room ).find( keyword );
}

void RoomTempDescr::set( Room *room, const DLString &keyword,
                         const XMLMultiString &text, const XMLMultiString &mark )
{
    RoomTempEntry &entry = roomTempEntries[room];
    ExtraDescription *ed = entry.eds.find( keyword );

    if (!ed) {
        ed = new ExtraDescription( );
        ed->keyword = keyword;
        entry.eds.push_back( ed );
    }

    ed->description = text;

    if (mark.emptyValues( ))
        entry.marks.erase( keyword );
    else
        entry.marks[keyword] = mark;
}

bool RoomTempDescr::clear( Room *room, const DLString &keyword )
{
    auto e = roomTempEntries.find( room );
    if (e == roomTempEntries.end( ))
        return false;

    bool found = e->second.eds.findAndDestroy( keyword );
    e->second.marks.erase( keyword );

    if (e->second.eds.empty( ))
        roomTempEntries.erase( e );

    return found;
}

DLString RoomTempDescr::marks( Room *room, lang_t lang )
{
    DLString result;

    auto e = roomTempEntries.find( room );
    if (e == roomTempEntries.end( ))
        return result;

    // In the order the descriptions were added, not by keyword.
    for (auto &ed: e->second.eds) {
        auto m = e->second.marks.find( ed->keyword );
        if (m == e->second.marks.end( ))
            continue;

        const DLString &line = m->second.getForLang( lang );
        if (!line.empty( ))
            result += line + "\n";
    }

    return result;
}
