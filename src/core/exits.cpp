#include "exits.h"
#include "room.h"
#include "string_utils.h"
#include "autoflags.h"
#include "def.h"


void exit_data::resolve()
{
    u1.to_room = get_room_instance( u1.vnum );
}

void exit_data::reset()
{
    exit_info = exit_info_default;
}

extra_exit_data::extra_exit_data()
    : exit_info(0), exit_info_default(0),
      key(0),
      max_size_pass(0), level(0)
{
    u1.to_room = 0;
}

extra_exit_data::~extra_exit_data()
{
}

void extra_exit_data::resolve()
{
    u1.to_room = get_room_instance( u1.vnum );
}

void extra_exit_data::reset()
{
    exit_info = exit_info_default;
}

extra_exit_data *ExtraExitList::find(const DLString &keyword) const
{
    for (auto &eexit: *this) {
        if (String::toString(eexit->keyword) == keyword)
            return eexit;
            
        if (eexit->keyword.matchesUnstrict(keyword))
            return eexit;

        if (eexit->short_desc_from.matchesUnstrict(keyword))
            return eexit;

        if (eexit->short_desc_to.matchesUnstrict(keyword))
            return eexit;
    }

    return 0;
}

bool ExtraExitList::findAndDestroy(const DLString &keyword)
{
    extra_exit_data *eexit = find(keyword);
    if (!eexit)
        return false;

    remove(eexit);
    delete eexit;
    return true;
}
