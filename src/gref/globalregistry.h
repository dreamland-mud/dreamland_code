/* $Id: globalregistry.h,v 1.1.2.5 2010-09-05 13:57:11 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#ifndef __GLOBALREGISTRY_H__
#define __GLOBALREGISTRY_H__

#include <map>
#include <vector>
#include <list>
#include <sstream>

#include "globalregistryelement.h"

class GlobalRegistryBase {
public:
    typedef std::vector<GlobalRegistryElement::Pointer> Table;
    typedef std::map<DLString, int> Indexes;
    typedef std::list<int> SortedIndexes;
    
    virtual ~GlobalRegistryBase( );
    void registrate( GlobalRegistryElement::Pointer );
    void unregistrate( GlobalRegistryElement::Pointer );
    
    int lookup( const DLString & );
    const DLString & getName( int ) const;
    inline bool goodIndex( int ) const;
    inline int size( ) const;
    void outputAll( ostringstream &, int, int ) const;
    
    template <typename Comparator>
    inline void sortIndexes( SortedIndexes &, Comparator );
    
protected:
    
    Table table;
    Indexes indexes;
    
private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const = 0;
    
    int add( GlobalRegistryElement::Pointer );
    void replace( int, GlobalRegistryElement::Pointer );
};


template <typename Elem>
class GlobalRegistry : public GlobalRegistryBase {
public:
    
    Elem * find( int ndx )
    {
        if (!goodIndex( ndx ))
            return NULL;

        return (Elem *)*table[ndx];
    }

    Elem * find( const DLString &name )
    {
        return find( lookup( name ) );
    }

    Elem * findExisting( const DLString &name )
    {
        unsigned int i;
        
        if (name.empty( ))
            return NULL;
            
        for (i = 0; i < table.size( ); i++) 
            if (table[i]->matchesStrict( name ))
                return (Elem *)*table[i];

        return NULL;
    }

    Elem * findUnstrict( const DLString &name )
    {
        unsigned int i;
        
        if (name.empty( ))
            return NULL;
            
        for (i = 0; i < table.size( ); i++) {
            if (table[i]->isValid( )) {
                if (table[i]->matchesUnstrict( name ))
                    return (Elem *)*table[i];
            }
        }

        return NULL;
    }
};

template <typename Comparator>
inline void GlobalRegistryBase::sortIndexes( SortedIndexes &order, Comparator comparator )
{
    for (unsigned int t = 0; t < table.size( ); t++)
        order.push_back( t );

    order.sort( comparator );
}

inline bool GlobalRegistryBase::goodIndex( int n ) const
{
    return (n >= 0 && n < (int) table.size( ));
}

inline int GlobalRegistryBase::size( ) const
{
    return table.size( );
}

#endif
