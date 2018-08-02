/* $Id: flagtablewrapper.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __FLAGTABLEWRAPPER_H__
#define __FLAGTABLEWRAPPER_H__

#include "flagtable.h"
#include "flagtableregistry.h"

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

#endif

