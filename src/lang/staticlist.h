/* $Id: staticlist.h,v 1.1.2.3.18.1 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __STATICLIST_H__
#define __STATICLIST_H__

struct StaticListDefComp {
    template <typename K1, typename K2>
    bool operator() (const K1 &k1, const K2 &k2) const {
        return k1 == k2;
    }
};

template <typename Key, typename Val, typename Cmp = StaticListDefComp>
struct StaticList {
    template <typename K, typename V>
    StaticList(K k, V v) : key(k), val(v) {
        next = first;
        if(next)
            next->pnext = &next;
        first = this;
        pnext = &first;
    }
    ~StaticList() {
        if(next)
            next->pnext = pnext;
        *pnext = next;
    }

    template <typename K>
    static Val *lookup(const K &key, Cmp cmp = Cmp( )) {
        StaticList *l;
        
        for(l = first; l; l = l->next)
            if( cmp(l->key, key) )
                return &l->val;

        return 0;
    }
    
    static StaticList * begin( ) {
        return first;
    }

    const Key & getKey( ) const {
        return key;
    }

    const Val & getVal( ) const {
        return val;
    }
    
    StaticList * getNext( ) {
        return next;
    }
    
private:
    Key key;
    Val val;
    
    StaticList *next, **pnext;
    static StaticList *first;
};

#endif /* __STATICLIST_H__ */
