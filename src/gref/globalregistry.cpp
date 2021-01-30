/* $Id: globalregistry.cpp,v 1.1.2.5 2014-09-19 11:45:54 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include <stdio.h>
#include "globalregistry.h"
#include "stringlist.h"

RegistryMap registryMap;

GlobalRegistryBase::~GlobalRegistryBase( )
{
}

/** Remember this instance in a global map. */
void GlobalRegistryBase::saveRegistryName() const
{
    registryMap[getRegistryName()] = this;
}

/** Erase from global registry map on destruction. */
void GlobalRegistryBase::eraseRegistryName() const
{
    registryMap.erase(getRegistryName());
}

void GlobalRegistryBase::setRegistryName(const DLString &registryName)
{
    this->registryName = registryName;
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

const DLString & GlobalRegistryBase::getRegistryName() const
{
    return registryName;
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

GlobalRegistryElement * GlobalRegistryBase::find( int ndx )
{
    if (goodIndex( ndx ))
        return *table[ndx];
        
    return NULL;
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

list<GlobalRegistryElement *> GlobalRegistryBase::findAll(const DLString &arguments)
{
    list<GlobalRegistryElement *> result;
    StringList args(arguments);

    for (unsigned int i = 0; i < table.size( ); i++) {
        for (auto &s: args)
            if (table[i]->matchesUnstrict(s))
                result.push_back(table[i].getPointer());
    }

    return result;
}

bool GlobalRegistryBase::hasElement(const DLString &name) const
{
    for (unsigned int i = 0; i < table.size( ); i++)
        if (table[i]->matchesStrict(name))
            return true;

    return false;
}