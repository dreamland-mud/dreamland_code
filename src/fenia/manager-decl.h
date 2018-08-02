/* $Id: manager-decl.h,v 1.1.4.4.6.6 2014-09-19 11:44:13 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: manager-decl.h,v 1.1.4.4.6.6 2014-09-19 11:44:13 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */


#ifndef __MANAGER_DECL_H__
#define __MANAGER_DECL_H__

namespace Scripting {

template <typename T>
class BaseManager : public T::Map {
public:
    typedef typename T::Map::iterator iterator;
    typedef typename T::Map::const_iterator const_iterator;
    typedef typename T::id_t id_t;

    using T::Map::begin;
    using T::Map::end;
    using T::Map::lower_bound;
    using T::Map::_M_insert_unique_;

    inline BaseManager();
    inline ~BaseManager();

    inline void erase(id_t id);
    inline T &at(id_t id);
    inline T &allocate();
    
    typename T::id_t lastId;
};

}

#endif
