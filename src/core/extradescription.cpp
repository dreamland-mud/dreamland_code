#include "extradescription.h"
#include "dl_strings.h"

ExtraDescription * ExtraDescrList::find(const DLString &kw) const
{
    for (const auto &ed: *this) {
        if (kw == ed->keyword)
            return ed;
    }

    return 0;
}

ExtraDescription * ExtraDescrList::findUnstrict(const DLString &kw) const
{
    for (const auto &ed: *this) {
        if (is_name(kw.c_str(), ed->keyword.c_str()))
            return ed;
    }

    return 0;
}


bool ExtraDescrList::findAndDestroy(const DLString &kw)
{
    ExtraDescription *ed = find(kw);
    if (!ed)
        return false;

    remove(ed);
    delete ed;
    return true;
}

void ExtraDescrList::deallocate()
{
    for(iterator pos = begin(); pos != end(); ++pos)
        delete( *pos );

    clear();
}






