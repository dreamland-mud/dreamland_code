/* $Id: manager-impl.h,v 1.1.4.5.6.4 2010-01-01 15:14:15 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: manager-impl.h,v 1.1.4.5.6.4 2010-01-01 15:14:15 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */


#ifndef __MANAGER_IMPL_H__
#define __MANAGER_IMPL_H__

#include <istream>
#include <ostream>

using namespace std;

#include "manager-decl.h"

namespace Scripting {

template <typename T>
BaseManager<T>::BaseManager() : lastId(1), reuseMode(false), reuseCursor(1)
{
}

template <typename T>
BaseManager<T>::~BaseManager() 
{
}

template <typename T>
void 
BaseManager<T>::erase(id_t id) 
{
    iterator i = T::Map::find(id);

    if(i != T::Map::end())
        T::Map::erase(i);
}

template <typename T>
T &
BaseManager<T>::at(id_t id) 
{
    iterator i = lower_bound(id);

    if(i == T::Map::end() || id < i->getId())
        i = _M_insert_unique_(i, T(id));


    return *i;
}

template <typename T>
T &
BaseManager<T>::allocate()
{
    // Recompiling a re-posted CodeSource: hand back existing slots in the same
    // sequential order a fresh compile would (2, 3, 4, ...), reusing the live
    // Function object at each id instead of minting a new one past lastId.
    if (reuseMode)
        return at(++reuseCursor);

    while(T::Map::find(++lastId) != T::Map::end())
        ;
    return at(lastId);
}

}

#endif
