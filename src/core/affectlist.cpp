#include "affectlist.h"
#include "affect.h"
#include "affectmanager.h"
#include "affecthandler.h"

Affect* AffectList::find( int sn ) const
{
    for (auto &paf: *this)
        if (paf->type == sn)
            return paf;

    return 0;
}

Affect * AffectList::find(int type, int location) const
{
    for (auto &paf: *this)
        if (paf->type == type && paf->location == location)
            return paf;

    return 0;
}

list<Affect *> AffectList::findAll(int type) const
{
    list<Affect *> result;

    for (auto &paf: *this)
        if (paf->type == type)
            result.push_back(paf);

    return result;
}

list<Affect *> AffectList::findAllWithBits(const FlagTable *table, int bits) const
{
    list<Affect *> result;

    for (auto &paf: *this)
        if (paf->bitvector.getTable() == table && (paf->bitvector & bits))
            result.push_back(paf);

    return result;
}

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


void AffectList::deallocate()
{
    while (!empty()) {
        AffectManager::getThis()->extract(back());
        pop_back();
    }
}

AffectList AffectList::clone() const
{
    AffectList other;
    other.assign(begin(), end());
    return other;
}

bool AffectList::hasNext(const_iterator &pos) const
{
    const_iterator next = pos;
    next++;

    return (next != end()
            && !(*next)->isExtracted()
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