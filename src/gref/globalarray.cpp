/* $Id: globalarray.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include "globalarray.h"
#include "globalbitvector.h"
#include "logstream.h"
#include "stringlist.h"

static int zeroValue;

const GlobalArray GlobalArray::emptyArray;

GlobalArray::GlobalArray( )
                         : registry( NULL )
{
}

GlobalArray::GlobalArray( GlobalRegistryBase *reg )
                         : registry( reg )
{
    clear( );
}

GlobalArray::~GlobalArray( )
{
}

void GlobalArray::clear( )
{
    vector<int>::clear( );

    if (registry)
        resize( registry->size( ) );
}

bool GlobalArray::isEmpty() const
{
    for (unsigned int sn = 0; sn < size(); sn++)
        if (at(sn) != 0)
            return false;
    return true;
}

int & GlobalArray::operator [] (size_type ndx)
{
    if (!registry)
        return zeroValue;
    if (!registry->goodIndex( ndx ))
        return zeroValue;
    if (ndx >= size( ))
        resize( ndx + 1 );
    
    return vector<int>::operator [](ndx);
}

const int & GlobalArray::operator [] (size_type ndx) const
{
    return vector<int>::operator [](ndx);
}

void GlobalArray::applyBitvector(const GlobalBitvector &bv, int modifier)
{
    vector<int> array = bv.toArray();

    for (unsigned int sn = 0; sn < array.size(); sn++) {
        (*this)[array[sn]] += modifier;
    }
}


StringList GlobalArray::toStringList(bool fRussian) const
{
    StringList lines;

    if (!registry)
        return lines;

    for (unsigned int sn = 0; sn < size(); sn++) {
        int mod = at(sn);
        if (mod == 0)
            continue;

        GlobalRegistryElement *e = registry->find(sn);
        DLString line = fRussian ? e->getRussianName() : e->getName();
        line << " на " << mod;
        lines.push_back(line);
    }

    return lines;
}
