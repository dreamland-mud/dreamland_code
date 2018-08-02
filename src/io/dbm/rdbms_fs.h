/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#ifndef __RDBMS_FS_H__
#define __RDBMS_FS_H__

#include "dlstring.h"

class DbEnvContext {
public:
    DbEnvContext( );
    virtual ~DbEnvContext( );

    void open( const DLString &path );
    void close( );

    DLString path;
};

class DbTxnContext {
public:
    DbTxnContext( );
    virtual ~DbTxnContext( );

    virtual DbEnvContext *getDbEnv( ) const = 0;

    bool txnRunning( );

    void commit( );
    void abort( );
};

template <typename key_t>
class DbContext : public virtual DbTxnContext {
public:
    typedef unsigned int size_t;

    DbContext( );
    virtual ~DbContext( );

    void open( const char *file, const char *db );
    void close( );
    void load( );
    
    void get( key_t, void *&, size_t & );
    void put( key_t, void *, size_t );
    void del( key_t );
    void delBoth( key_t, key_t ) { }
    virtual void seq( key_t, void *, size_t ) = 0;

private:
    DLString getFileName( key_t k );

    const char *file;
    const char *dbname;
};

#endif
