/* $Id: txncontext.h,v 1.1.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __TXNCONTEXT_H__
#define __TXNCONTEXT_H__

#include <stdint.h>

#include "dlstring.h"


class Db;
class DbTxn;
class DbEnv;

class DbEnvContext {
    friend class DbContext;
public:
    DbEnvContext( );
    ~DbEnvContext( );
    
    void open(const DLString &path);
    void close( );

    DbEnv *dbEnv;
};

class TxnContext {
public:
    TxnContext( );
    virtual ~TxnContext( );

    virtual DbEnvContext *getDbEnv() const = 0;

    bool txnRunning( );

    void commit( );
    void abort( );
    
protected:
    DbTxn *getCurrentTxn( );

private:
    DbTxn *currentTxn;
};

class DbContext : public virtual TxnContext {
public:
    typedef uint32_t key_t;
    typedef uint32_t size_t;

    struct Data {
        Data( ) : ptr(0), size(0) { }
        Data( void *p, size_t s ) : ptr(p), size(s) { }
        
        void set(void *p, size_t s) { ptr = p; size = s; }
        
        void *get_data( ) { return ptr; }
        size_t get_size( ) { return size; }

    private:
        void *ptr;
        size_t size;
    };
    
    DbContext( );
    virtual ~DbContext( );

    void open( const char *file, const char *db );
    void close( );

    void load( );
    virtual void seq( key_t, Data & ) = 0;

    void get( key_t, Data & );
    void put( key_t, Data & );
    void del( key_t );
private:
    Db *db;
};

#endif
