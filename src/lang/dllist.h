/* $Id: dllist.h,v 1.14.2.3.24.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
// dllist.h: interface for the DLList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef        DLLIST_H
#define DLLIST_H

#include <list>

/**
 * @author Igor S. Petrenko
 * @short Расширенный класс list
 * Добавлены методы для удаления элементов, для которых нужно
 * вызвать delete
 */

template<typename T>
class DLList : public std::list<T*>
{
    typedef std::list<T*> List;
public:
    typedef typename List::iterator iterator;
    using List::front;
    using List::back;
    using List::push_front;
    using List::push_back;
    using List::pop_front;
    using List::pop_back;
    using List::begin;
    using List::end;
    using List::erase;

    inline iterator erase_delete( iterator, iterator );
    inline void pop_back_delete( );
    inline void clear_delete( );
};



template<typename T>
typename DLList<T>::iterator DLList<T>::erase_delete( iterator begin, iterator end )
{
        for( iterator pos = begin; pos != end; ++pos )
        {
                delete( *pos );
        }
        return erase( begin, end );
}

template<typename T>
void DLList<T>::pop_back_delete( )
{
        delete( *back( ) );
        pop_back( );
}

template<typename T>
void DLList<T>::clear_delete( )
{
        erase_delete( begin( ), end( ) );
}

#endif
