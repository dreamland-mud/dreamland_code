/* $Id: safelist.h,v 1.1.4.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __SAFELIST_H__
#define __SAFELIST_H__

struct SafeListElement {
    SafeListElement( )
    {
	next = prev = this;
    }

    ~SafeListElement( )
    {
	fromList( );
    }

    void linkup( )
    {
	next->prev = this;
	prev->next = this;
    }
    
    void insertBefore(SafeListElement *e) 
    {
	next = e;
	prev = e->prev;
	linkup( );
    }
    void insertAfter(SafeListElement *e) 
    {
	next = e->next;
	prev = e;
	linkup( );
    }
    
    void fromList( ) 
    {
	next->prev = prev;
	prev->next = next;
	next = prev = this;
    }

    SafeListElement *next;
    SafeListElement *prev;

    enum {
	SLE_ENTRY,
	SLE_ITERATOR,
	SLE_HEAD
    } type;
};

template <typename>
struct SafeListEntry : public SafeListElement {
    SafeListEntry( ) 
    {
	type = SLE_ENTRY;
    }
};

template <typename ElementType, typename ListType>
class SafeList {
public:
    typedef SafeListEntry<ListType> EntryType;
    
    template <typename Type>
    struct iterator_base : public SafeListElement {
    friend class SafeList;
	iterator_base( ) 
	{
	    type = SLE_ITERATOR;
	}
	
	iterator_base(const iterator_base &it) 
	{
	    type = SLE_ITERATOR;
	    insertAfter(it.getPrev( ));
	}
	
	iterator_base( Type *e );

	const iterator_base &operator =( const iterator_base &r ) 
	{
	    fromList( );
	    insertAfter( r.getPrev( ) );
	    return *this;
	}
	    
	iterator_base & operator ++( ) 
	{
	    EntryType *e = getNext( );
	    fromList( );
	    insertAfter(e);
	    return *this;
	}
	
	iterator_base & operator --( );
	Type & operator *( );
	Type * operator -> ( );
    
	bool operator == ( const iterator_base &it ) const
	{
	    return getPrev( ) == it.getPrev( );
	}
    
	bool operator != ( const iterator_base &it ) const
	{
	    return getPrev( ) != it.getPrev( );
	}

    protected:
	iterator_base( SafeListElement *e ) 
	{
	    type = SLE_ITERATOR;
	    insertAfter( e );
	}

	EntryType *getNext( ) const;
	EntryType *getPrev( ) const;
    };
    
    typedef iterator_base<ElementType> iterator;
    typedef iterator_base<const ElementType> const_iterator;

    SafeList( )
    {
	head.type = SafeListElement::SLE_HEAD;
    }
    
    SafeList( const SafeList &list )
    {
	head.type = SafeListElement::SLE_HEAD;
    }
    
    const SafeList & operator = (SafeList &list)
    {
	clear( );
	return *this;
    }
    
    void clear( )
    {
	while (!empty( ))
	    erase( &front( ) );
    }

    iterator begin( )
    {
	return ++end( );
    }
    
    const_iterator begin( ) const
    {
	return ++end( );
    }
    
    iterator end( )
    {
	return iterator( &head );
    }
    
    const_iterator end( ) const
    {
	return const_iterator( const_cast<SafeListElement *>( &head ) );
    }
    
    ElementType & front( ) 
    {
	return *begin( );
    }

    const ElementType & front( ) const
    {
	return *begin( );
    }

    bool empty( ) const
    {
	return begin( ) == end( );
    }
    
    int size( ) const
    {
	int cnt;
	const_iterator i;
	
	for (i = begin( ), cnt = 0; i != end( ); ++i, ++cnt)
	    ;

	return cnt;
    }
    
    void push_back( ElementType *elem );
    void push_front( ElementType *elem );
    void insert( const iterator &it, ElementType *elem );

    void erase( const iterator &it )
    {
	it.getPrev( )->fromList( );
    }

    SafeListElement head;
};

#endif
