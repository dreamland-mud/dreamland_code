/* $Id: globalregistry.cpp,v 1.1.2.5 2014-09-19 11:45:54 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include <stdio.h>
#include "globalregistry.h"

GlobalRegistryBase::~GlobalRegistryBase( )
{
}

void GlobalRegistryBase::registrate( GlobalRegistryElement::Pointer elem )
{
    Indexes::iterator iter = indexes.find( elem->getName( ) );

    if (iter == indexes.end( )) 
        add( elem );
    else 
        replace( iter->second, elem );
}

void GlobalRegistryBase::unregistrate( GlobalRegistryElement::Pointer elem )
{
    replace( elem->getIndex( ), getDumbElement( elem->getName( ) ) );
}

int GlobalRegistryBase::lookup( const DLString &name )
{
    Indexes::iterator i;
    
    if (name.empty( ))
        return -1;

    i = indexes.find( name );
    
    if (i == indexes.end( )) 
        return add( getDumbElement( name ) );
    else 
        return i->second;
}

const DLString & GlobalRegistryBase::getName( int ndx ) const
{
    if (goodIndex( ndx ))
        return table[ndx]->getName( );
    else
        return DLString::emptyString;
}


int GlobalRegistryBase::add( GlobalRegistryElement::Pointer elem )
{
    int ndx = table.size( );

    indexes[ elem->getName( ) ] = ndx;
    table.push_back( elem );
    elem->setIndex( ndx );

    return ndx;
}

void GlobalRegistryBase::replace( int ndx, GlobalRegistryElement::Pointer elem )
{
    table[ndx] = elem;
    elem->setIndex( ndx );
}

void GlobalRegistryBase::outputAll( ostringstream &out, int width, int columns ) const
{
    char buf[256];
    int col = 0;
    DLString pattern;

    pattern << "%-" << width << "s";

    for (unsigned int i = 0; i < table.size( ); i++) {
        sprintf( buf, pattern.c_str( ), table[i]->getName( ).c_str( ) );
        out << buf;

        if (++col % columns == 0)
            out << endl;
    }

    if (col % columns != 0)
        out << endl;
}

