#ifndef AFFECTLIST_H
#define AFFECTLIST_H

#include <list>

class Affect;
class FlagTable;

struct AffectList: public std::list<Affect *> {
    /* Find an affect in an affect list */    
    Affect * find(int type) const;
    
    /** Find affect with given type and location. */    
    Affect * find(int type, int location) const;

    /** Find all affects with a given type. */
    list<Affect *> findAll(int type) const;
    
    /** Find all affects where bitvector matches this one. */
    list<Affect *> findAllWithBits(const FlagTable *table, int bits) const;

    /** Find all affects where AffectHandler exists and can be called. */
    list<Affect *> findAllWithHandler() const;
    
    /** Destroy all elements and clear the list. */
    void deallocate();

    /** 
     * Create another list with the same affect pointers. Used in update loops, 
     * where affect removal can cause other affects to be removed. 
     */
    AffectList clone() const;

    Affect* get(int position) const;

    /** Return false if this is the last affect of the same type in a row. */
    bool hasNext(const_iterator &pos) const;
};

#endif