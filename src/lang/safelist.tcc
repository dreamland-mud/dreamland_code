

template <typename ElementType, typename ListType>
template <typename Type>
SafeList<ElementType, ListType>::iterator_base<Type>::iterator_base( Type *e ) 
{
    type = SLE_ITERATOR;
    insertAfter( static_cast<EntryType *>( const_cast<ElementType *>( e ) ) );
}

template <typename ElementType, typename ListType>
template <typename Type>
typename SafeList<ElementType, ListType>::EntryType *
SafeList<ElementType, ListType>::iterator_base<Type>::getNext( ) const
{
    SafeListElement *e;
    
    for(e=next; e->type == SLE_ITERATOR; e = e->next)
	;

    return static_cast<EntryType *>( e );
}

template <typename ElementType, typename ListType>
template <typename Type>
typename SafeList<ElementType, ListType>::EntryType *
SafeList<ElementType, ListType>::iterator_base<Type>::getPrev( ) const
{
    SafeListElement *e;
    
    for(e=prev; e->type == SLE_ITERATOR; e = e->prev)
	;

    return static_cast<EntryType *>( e );
}

template <typename ElementType, typename ListType>
template <typename Type>
SafeList<ElementType, ListType>::iterator_base<Type> &
SafeList<ElementType, ListType>::iterator_base<Type>::operator --( ) 
{
    EntryType *e = static_cast<EntryType *>( getPrev( )->prev );
    fromList( );
    insertAfter(e);
    return *this;
}

template <typename ElementType, typename ListType>
template <typename Type>
Type & SafeList<ElementType, ListType>::iterator_base<Type>::operator *( ) 
{
    return *static_cast<Type *>( getPrev( ) );
}

template <typename ElementType, typename ListType>
template <typename Type>
Type * SafeList<ElementType, ListType>::iterator_base<Type>::operator -> ( )
{
    return static_cast<Type *>( getPrev( ) );
}

/************************************************************/

template <typename ElementType, typename ListType>
void SafeList<ElementType, ListType>::push_back( ElementType *elem )
{
    static_cast<EntryType *>( elem )->fromList( );
    static_cast<EntryType *>( elem )->insertBefore( &head );
}

template <typename ElementType, typename ListType>
void SafeList<ElementType, ListType>::push_front( ElementType *elem )
{
    static_cast<EntryType *>( elem )->fromList( );
    static_cast<EntryType *>( elem )->insertAfter( &head );
}

template <typename ElementType, typename ListType>
void SafeList<ElementType, ListType>::insert( const iterator &it, ElementType *elem )
{
    static_cast<EntryType *>( elem )->fromList( );
    static_cast<EntryType *>( elem )->insertBefore( it.getNext( ) );
}

