#include "affectlist.h"
#include "affect.h"
#include "affecthandler.h"

/* Find an effect in an affect list */
Affect* AffectList::find( int sn ) const
{
    for (auto &paf: *this)
        if (paf->type == sn)
            return paf;

    return 0;
}

/** Find effect with given type and location. */
Affect * AffectList::find(int type, int location) const
{
    for (auto &paf: *this)
        if (paf->type == type && paf->location == location)
            return paf;

    return 0;
}

/** Find all affects with a given type. */
list<Affect *> AffectList::findAll(int type) const
{
    list<Affect *> result;

    for (auto &paf: *this)
        if (paf->type == type)
            result.push_back(paf);

    return result;
}

/** Find all affects where bitvector matches this one. */
list<Affect *> AffectList::findAllWithBits(const FlagTable *table, int bits) const
{
    list<Affect *> result;

    for (auto &paf: *this)
        if (paf->bitvector.getTable() == table && (paf->bitvector & bits))
            result.push_back(paf);

    return result;
}

/** Find all affects where AffectHandler exists and can be called. */
list<Affect *> AffectList::findAllWithHandler() const
{
    list<Affect *> result;

    for (auto paf_iter = cbegin(); paf_iter != cend(); paf_iter++) {
        Affect *paf = *paf_iter;
        if (!hasNext(paf_iter) && paf->type->getAffect( ))
            result.push_back(paf);
    }
    
    return result;
}


/* Destroy all elements and clear the list. */
void AffectList::deallocate()
{
    while (!empty()) {
        ddeallocate(back());
        pop_back();
    }
}

AffectList AffectList::clone() const
{
    AffectList other;
    other.assign(begin(), end());
    return other;
}

/** Return false if this is the last affect of the same type in a row. */
bool AffectList::hasNext(const_iterator &pos) const
{
    const_iterator next = pos;
    next++;

    return (next != end()
            && (*next)->type == (*pos)->type
            && (*next)->duration == (*pos)->duration);
}

Affect * AffectList::get(int position) const
{
    int cnt = 0;

    for (auto i = cbegin(); i != cend(); i++, cnt++)
        if (cnt == position)
            return *i;
    
    return 0;
}