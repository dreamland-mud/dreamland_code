#ifndef AFFECTLIST_H
#define AFFECTLIST_H

#include <list>

class Affect;

struct AffectList: public std::list<Affect *> {
    Affect * find(int type) const;
    Affect * find(int type, int location) const;
    list<Affect *> findAll(int type) const;
    list<Affect *> findAllWithBits(int where, int bits) const;
    list<Affect *> findAllWithHandler() const;
    
    void deallocate();
    AffectList clone() const;

    Affect* get(int position) const;
    bool hasNext(const_iterator &pos) const;
};

#endif