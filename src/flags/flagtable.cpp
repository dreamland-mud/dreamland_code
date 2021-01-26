/* $Id: flagtable.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include "flagtable.h"

/*----------------------------------------------------------------------
 * FlagTable
 *---------------------------------------------------------------------*/
int FlagTable::index( const DLString &arg, bool strict ) const
{
    if (arg.empty( ))
        return NO_FLAG;

    for (int i = 0; i < size; i++) 
        if (strict) {
            if (arg ^ fields[i].name)
                return i;
        } 
        else {
            if (arg.strPrefix( fields[i].name ))
                return i;
        }

    return NO_FLAG;
}

bitnumber_t FlagTable::value( const DLString &arg, bool strict ) const
{
    int i = index( arg, strict );

    if (i == NO_FLAG)
        return NO_FLAG;
    else
        return fields[i].value;
}

bitstring_t FlagTable::bitstring( const DLString &arg, bool strict ) const
{
    Bitstring marked;
    bool found = false;
    DLString args( arg );

    while (!args.empty( )) {
        DLString word = args.getOneArgument( );
        int i = index( word, strict );

        if (i != NO_FLAG) {
            marked.setBitNumber( fields[i].value );
            found = true;
        }
    }
    
    if (found)
        return marked;
    else
        return NO_FLAG;
}

DLString FlagTable::names( bitstring_t bits ) const
{
    if (bits == NO_FLAG)
        return DLString::emptyString;

    DLString buf;
    Bitstring b( bits );
    
    for (int i = 0; i <= max; i++)
        if (b.isSetBitNumber( i ) && reverse[i] != NO_FLAG) {
            if (!buf.empty( ))
                buf << " ";

            buf << fields[reverse[i]].name;
        }

    return buf;
}

DLString FlagTable::messages( bitstring_t bits, bool comma, char gcase ) const
{
    if (bits == NO_FLAG)
        return DLString::emptyString;

    DLString buf;
    Bitstring b( bits );
    
    for (int i = 0; i <= max; i++)
        if (b.isSetBitNumber( i ) && reverse[i] != NO_FLAG) {
            if (!buf.empty( ))
                buf << (comma ? ", " : " ");

            buf << DLString( fields[reverse[i]].message ).ruscase( gcase );
        }

    return buf;
}

DLString FlagTable::name( bitnumber_t value ) const
{
    if (value < 0 || value > max || reverse[value] == NO_FLAG)
        return DLString::emptyString;
    else
        return fields[reverse[value]].name;
}

DLString FlagTable::message( bitnumber_t value, char gcase ) const
{
    if (value < 0 || value > max || reverse[value] == NO_FLAG)
        return DLString::emptyString;
    
    auto &field = fields[reverse[value]];
    
    if (field.message)
        return DLString(field.message).ruscase(gcase);

    return field.name;
}


