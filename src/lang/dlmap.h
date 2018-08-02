/* $Id: dlmap.h,v 1.15.2.2.24.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
// dlmap.h: interface for the DLMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	DLMAP_H
#define DLMAP_H

#include <map>

/**
 * @author Igor S. Petrenko
 * @short Расширенный класс map
 * Добавлены методы для удаления элементов, для которых нужно
 * вызвать delete
 */
template<typename Key, class Value, class Compare=std::less<Key> >
class DLMap : public std::map<Key, Value*, Compare>
{
    typedef std::map<Key, Value*, Compare> Map;
public:
    typedef typename Map::iterator iterator;
    using Map::begin;
    using Map::end;
    using Map::erase;

    inline void erase_delete( iterator, iterator );
    inline void erase_delete( iterator );
    inline void clear_delete( );
};



template<typename Key, typename Value, typename Compare>
void DLMap<Key, Value, Compare>::erase_delete( iterator begin, iterator end )
{
	for( iterator pos = begin; pos != end; pos++ )
	{
		delete( pos->second );
	}
	erase( begin, end );
}

template<typename Key, typename Value, typename Compare>
void DLMap<Key, Value, Compare>::erase_delete( iterator position )
{
	delete( position->second );
	erase( position );
}

template<typename Key, typename Value, typename Compare>
void DLMap<Key, Value, Compare>::clear_delete( )
{
	erase_delete( begin( ), end( ) );
}

#endif
