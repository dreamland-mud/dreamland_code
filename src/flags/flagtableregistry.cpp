/* $Id: flagtableregistry.cpp,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include <set>
#include "flagtableregistry.h"
#include "flagtable.h"
#include "logstream.h"

/*-------------------------------------------------------------------------*
 * FlagTableRegistry::Entry
 *-------------------------------------------------------------------------*/
FlagTableRegistry::Entry::Entry( const char *name, const FlagTable *table )
{
    FlagTableRegistry::addTable( name, table );
    this->table = table;
}

FlagTableRegistry::Entry::~Entry( )
{
    FlagTableRegistry::removeTable( table );
}

/*-------------------------------------------------------------------------*
 * FlagTableRegistry
 *-------------------------------------------------------------------------*/
FlagTableRegistry::NamesMap FlagTableRegistry::names2tables;
FlagTableRegistry::TablesMap FlagTableRegistry::tables2names;

void FlagTableRegistry::addTable( DLString name, const FlagTable *table )
{
    names2tables[name] = table;
    tables2names[table] = name;
}

void FlagTableRegistry::removeTable( const FlagTable *table )
{
    TablesMap::iterator t = tables2names.find( table );
    
    if (t != tables2names.end( )) {
        NamesMap::iterator n = names2tables.find( t->second );

        if (n != names2tables.end( )) 
            names2tables.erase( n );

        tables2names.erase( t );
    }
}

void FlagTableRegistry::removeTable( const DLString &name )
{
    NamesMap::iterator n = names2tables.find( name );
    
    if (n != names2tables.end( )) {
        TablesMap::iterator t = tables2names.find( n->second );

        if (t != tables2names.end( ))
            tables2names.erase( t );

        names2tables.erase( n );
    }
}

const DLString & FlagTableRegistry::getName( const FlagTable * table ) 
{
    if (table == 0)
        return DLString::emptyString;

    TablesMap::iterator t = tables2names.find( table );

    if (t == tables2names.end( ))
        return DLString::emptyString;

    return t->second;
}

const FlagTable * FlagTableRegistry::getTable( const DLString & arg )
{
    if (arg.empty( ))
        return NULL;

    NamesMap::iterator n = names2tables.find( arg );

    if (n == names2tables.end( ))
        return NULL;

    return n->second;
}

// A FlagTableWrapper (affect location/bitvector) whose table pointer is non-null
// but unknown to the registry means memory corruption: something stored a
// non-table pointer (seen on live: apply_flags.fields, one indirection off
// &apply_flags). The wrapper's guard then returns empty instead of dereferencing
// it and crashing. Log once per distinct bad pointer so a recurrence stays
// visible without spamming a hot display path. Root cause of the write is still
// open, so this breadcrumb is the only signal it happened again.
void reportUnregisteredFlagTable( const FlagTable *table )
{
    static std::set<const FlagTable *> reported;

    if (reported.insert( table ).second)
        LogStream::sendError( )
            << "reportUnregisteredFlagTable: non-table pointer "
            << (const void *)table
            << " reached a name/message lookup -- corrupt affect/flags entry, returning empty."
            << endl;
}


