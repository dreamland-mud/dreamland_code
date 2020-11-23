/* $Id: bitstring.h,v 1.1.2.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __BITSTRING_H__
#define __BITSTRING_H__

typedef long long bitstring_t;
typedef int bitnumber_t;

class Bitstring  {
public:
    inline Bitstring( );
    inline Bitstring( bitstring_t );

    inline bitstring_t getValue( ) const;
    inline void setValue( bitstring_t );
    inline void clear( );

    inline bool isSet( bitstring_t ) const;
    inline void setBit( bitstring_t );
    inline void removeBit( bitstring_t );
    inline void toggleBit( bitstring_t );
    inline void changeBit(bitstring_t value, bool fAdd);
    
    inline bool isSetBitNumber( bitnumber_t ) const;
    inline void setBitNumber( bitnumber_t );
    inline void removeBitNumber( bitnumber_t );
    inline void toggleBitNumber( bitnumber_t );
    inline bool equalsToBitNumber( bitnumber_t ) const;
    
    inline operator const bitstring_t & ( ) const;

protected:
    inline static bitstring_t numberToBit( bitnumber_t );

    bitstring_t value;
};

inline Bitstring::Bitstring( ) 
                     : value( 0 )
{
}
inline Bitstring::Bitstring( bitstring_t v ) 
                     : value( v )
{
}
inline bitstring_t Bitstring::getValue( ) const
{
    return value;
}
inline void Bitstring::setValue( bitstring_t value )
{
    this->value = value;
}
inline void Bitstring::clear( )
{
    value = 0;
}
inline bool Bitstring::isSet( bitstring_t bit ) const 
{
    return value & bit;
}
inline void Bitstring::removeBit( bitstring_t bit ) 
{
    value &= ~bit;
}
inline void Bitstring::setBit( bitstring_t bit ) 
{
    value |= bit;
}
inline void Bitstring::toggleBit( bitstring_t bit )
{
    value ^= bit;
}
inline void Bitstring::changeBit(bitstring_t value, bool fAdd)
{
    if (fAdd)
        setBit(value);
    else
        removeBit(value);
}
inline bool Bitstring::isSetBitNumber( bitnumber_t number ) const
{
    return isSet( numberToBit( number ) );
}
inline void Bitstring::setBitNumber( bitnumber_t number )
{
    setBit( numberToBit( number ) );
}
inline void Bitstring::removeBitNumber( bitnumber_t number )
{
    removeBit( numberToBit( number ) );
}
inline void Bitstring::toggleBitNumber( bitnumber_t number )
{
    toggleBit( numberToBit( number ) );
}
inline bool Bitstring::equalsToBitNumber( bitnumber_t number ) const
{
    return value == numberToBit( number );
}
inline Bitstring::operator const bitstring_t & ( ) const
{
    return value;
}
inline bitstring_t Bitstring::numberToBit( bitnumber_t number ) 
{
    return (1 << number);
}



#endif
