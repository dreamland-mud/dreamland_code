/* $Id: xmlflags.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef XMLFLAGS_H
#define XMLFLAGS_H

#include "flags.h"
#include "xmlnode.h"

/*
 * XMLFlags
 */
class XMLFlags : public Flags {
public:
    XMLFlags( bitstring_t, const FlagTable * );
    
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;
    
    inline XMLFlags & operator = ( bitstring_t bit ) { 
        setValue( bit );
        return *this;
    }

    inline XMLFlags & operator |= ( bitstring_t bit ) { 
        value |= bit;
        return *this;
    }

    inline XMLFlags & operator &= ( bitstring_t bit ) { 
        value &= bit;
        return *this;
    }

    inline XMLFlags & operator ^= ( bitstring_t bit ) { 
        value ^= bit;
        return *this;
    }
};


/*
 * XMLFlagsWithTable
 */
class XMLFlagsWithTable : public XMLFlags {
public:
    XMLFlagsWithTable( );

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;
    void clear();
    
protected:
    static const DLString ATTRIBUTE_TABLE;
};

/*
 * XMLFlagsNoEmpty
 */
class XMLFlagsNoEmpty : public XMLFlags {
public:
    XMLFlagsNoEmpty( bitstring_t, const FlagTable * );
    
    bool toXML( XMLNode::Pointer& ) const;
};

#endif
