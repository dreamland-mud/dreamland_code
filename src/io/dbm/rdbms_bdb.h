/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#ifndef __RDBMS_BDB_H__
#define __RDBMS_BDB_H__

#include <db_cxx.h>

#include "dlstring.h"

class Db;
class DbTxn;
class DbEnv;

class DbEnvContext {
public:
    DbEnvContext( );
    virtual ~DbEnvContext( );

    void open( const DLString &path );
    void close( );

    DbEnv *dbEnv;
};

class DbTxnContext {
public:
    DbTxnContext( );
    virtual ~DbTxnContext( );

    virtual DbEnvContext *getDbEnv( ) const = 0;

    bool txnRunning( );

    void commit( );
    void abort( );

protected:
    DbTxn * getCurrentTxn( );

    DbTxn *currentTxn;
};

template <typename K>
class DbContext : public virtual DbTxnContext {
public:
    typedef unsigned int size_t;
    typedef K key_t;

    DbContext( );
    virtual ~DbContext( );

    void open( const char *file, const char *db, bool fDup = false );
    void close( );
    void load( );
    
    void get( key_t, void *&, size_t & );
    void put( key_t, void *, size_t );
    void del( key_t );
    void delBoth( key_t, key_t );
    virtual void seq( key_t, void *, size_t ) = 0;

    template <typename H>
    void forEach( key_t k, H h ) {
        Dbt key(key2dbt(k)), val;
        Dbc *cur = 0;
        int ret;

        db->cursor( getCurrentTxn( ), &cur, 0 );
        
        ret = cur->get(&key, &val, DB_SET);
        while(ret != DB_NOTFOUND) {
            h(val.get_data( ), val.get_size( ));
            ret = cur->get(&key, &val, DB_NEXT_DUP);
        }

        cur->close( );
    }

private:

    Dbt key2dbt( key_t );
    void dbt2key( Dbt &, key_t & );

    Db *db;
};


#endif
