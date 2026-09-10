/* $Id: flagtablewrapper.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __FLAGTABLEWRAPPER_H__
#define __FLAGTABLEWRAPPER_H__

#include "flagtable.h"
#include "flagtableregistry.h"

// Logs, rate-limited, when a wrapper holds a non-null pointer the registry does
// not know -- that only happens through memory corruption. Defined in
// flagtableregistry.cpp so this header need not pull in logstream.
void reportUnregisteredFlagTable( const FlagTable * );

/*
 * FlagTableWrapper
 */
struct FlagTableWrapper {
    inline FlagTableWrapper( );
    inline FlagTableWrapper( const FlagTable * );

    inline DLString getTableName( ) const;
    inline const FlagTable * getTable( ) const;
    inline void setTable( const FlagTable * );
    inline void setTable( const DLString & );

    // A corrupt affect can hold a table pointer that is not a FlagTable at all.
    // Dereferencing it (name/message/names) reads garbage and segfaults the whole
    // server. Trust the pointer only when the registry knows it.
    inline bool tableIsReal( ) const;

protected:
    const FlagTable * table;
};

inline FlagTableWrapper::FlagTableWrapper( )
                               : table( NULL )
{
}
inline FlagTableWrapper::FlagTableWrapper( const FlagTable *t )
                               : table( t )
{
}
inline DLString FlagTableWrapper::getTableName( ) const
{
    return FlagTableRegistry::getName( table );
}
inline const FlagTable * FlagTableWrapper::getTable( ) const
{
    return table;
}
inline void FlagTableWrapper::setTable( const FlagTable *table )
{
    this->table = table;
}
inline void FlagTableWrapper::setTable( const DLString &str )
{
    table = FlagTableRegistry::getTable( str );
}
inline bool FlagTableWrapper::tableIsReal( ) const
{
    if (table == 0)
        return false;
    if (FlagTableRegistry::getTablesMap( ).count( table ) != 0)
        return true;
    reportUnregisteredFlagTable( table );
    return false;
}

#endif

