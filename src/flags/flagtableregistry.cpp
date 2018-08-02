/* $Id: flagtableregistry.cpp,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include "flagtableregistry.h"
#include "flagtable.h"

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


