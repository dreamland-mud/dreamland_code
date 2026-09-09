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

    // Recompile-in-place support for CodeSource::eval on a re-posted scenario.
    // While reuseMode is set, allocate() hands back existing slots by sequential
    // id (reuseCursor) instead of minting fresh ones, so a hot reload overwrites
    // its Function objects in place -- live closures over (csId, fnId) keep a
    // valid pointer and pick up the new body -- instead of leaving a duplicate
    // CodeSource behind. Only ever set on a CodeSource's own `functions` manager
    // during eval; false everywhere else. See Trello #2857 (P2b).
    bool reuseMode;
    typename T::id_t reuseCursor;
};

}

#endif
