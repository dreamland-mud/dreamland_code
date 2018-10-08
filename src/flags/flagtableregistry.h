/* $Id: flagtableregistry.h,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __FLAGTABLEREGISTRY_H__
#define __FLAGTABLEREGISTRY_H__

#include <map>
#include "dlstring.h"

struct FlagTable;

struct FlagTableRegistry {
    struct Entry {
        Entry( const char *, const FlagTable * );
        ~Entry( );
        
        const FlagTable *table;
    };

    typedef map<const FlagTable *, DLString> TablesMap;
    typedef map<DLString, const FlagTable *> NamesMap;

    static void addTable( DLString, const FlagTable * );
    static void removeTable( const FlagTable * );
    static void removeTable( const DLString & );

    static const DLString &getName( const FlagTable * );
    static const FlagTable * getTable( const DLString & );

    inline static const NamesMap & getNamesMap( );
    inline static const TablesMap & getTablesMap( );

private:
    static NamesMap names2tables;
    static TablesMap tables2names;
};

const FlagTableRegistry::NamesMap & FlagTableRegistry::getNamesMap( ) 
{
    return names2tables;
}

const FlagTableRegistry::TablesMap & FlagTableRegistry::getTablesMap( ) 
{
    return tables2names;
}

#endif
