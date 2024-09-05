/* $Id: globalbitvector.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#ifndef GLOBALBITVECTOR_H
#define GLOBALBITVECTOR_H

#include <vector>
#include <set>
#include "dlstring.h"
#include "globalregistry.h"

class GlobalBitvector {
public:
    static const GlobalBitvector emptyBitvector;
    
    GlobalBitvector( );
    GlobalBitvector( GlobalRegistryBase *registry );
    GlobalBitvector( GlobalRegistryBase *registry, unsigned int ndx );

    virtual ~GlobalBitvector( );

    inline bool isSet( unsigned int ndx ) const
    {
        return ndx < bits.size( ) && bits[ndx];
    }
    inline bool isSet( const GlobalRegistryElement &e ) const
    {
        return isSet( e.getIndex( ) );
    }
    inline bool isSet( const GlobalRegistryElement *e ) const
    {
        return isSet( e->getIndex( ) );
    }
    inline bool isSetAll( const GlobalBitvector &bv ) const
    {
        for (unsigned int b = 0; b < bv.bits.size( ); b++)
            if (bv.bits[b])
                if (!isSet( b ))
                    return false;

        return true;
    }
    inline bool isSetAny( const GlobalBitvector &bv ) const
    {
        for (unsigned int b = 0; b < bv.bits.size( ); b++)
            if (bv.bits[b])
                if (isSet( b ))
                    return true;

        return false;
    }

    inline void set( unsigned int ndx )
    {
        if (ndx >= bits.size( ))
            bits.resize( ndx + 1, false );

        bits[ndx] = true;
    }
    inline void set( const GlobalRegistryElement &e )
    {
        set( e.getIndex( ) );
    }
    inline void set( const GlobalBitvector &bv )
    {
        for (unsigned int b = 0; b < bv.bits.size( ); b++)
            if (bv.bits[b])
                set( b );
    }

    inline void remove( unsigned int ndx )
    {
        if (ndx < bits.size( ))
            bits[ndx] = false;
    }
    inline void remove( const GlobalRegistryElement &e )
    {
        return remove( e.getIndex( ) );
    }
    inline void remove( const GlobalBitvector &bv )
    {
        for (unsigned int b = 0; b < bv.bits.size( ); b++)
            if (bv.bits[b])
                remove( b );
    }
    inline void change( const GlobalBitvector &bv, bool fSet )
    {
        for (unsigned int b = 0; b < bv.bits.size( ); b++)
            if (bv.bits[b])
                fSet ? set(b) : remove(b);
    }
    
    inline void clear( )
    {
        bits.clear( );
    }

    inline bool empty( ) const
    {
        for (unsigned int b = 0; b < bits.size( ); b++)
            if (bits[b]) 
                return false;

        return true;
    }

    inline void setRegistry( GlobalRegistryBase *reg )
    {
        clear( );
        registry = reg;
    }

    inline GlobalRegistryBase * getRegistry( ) const
    {
        return registry;
    }

    void fromString( const DLString &source );
    DLString toString( char joiner = ' ' ) const;
    DLString toRussianString( char gcase = '1', const char *joiner = 0 ) const;
    vector<int> toArray( ) const;
    std::set<int> toSet() const;

protected:
    vector<bool> bits;
    GlobalRegistryBase *registry;
};


#endif
