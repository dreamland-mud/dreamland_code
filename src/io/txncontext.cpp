/* $Id: txncontext.cpp,v 1.1.6.3 2010-01-01 15:14:15 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */

#include "txncontext.h"
#include "exception.h"
#include "logstream.h"

#include "config.h"

#include <db_cxx.h>

/*-------------------------------------------------------------------
 * DbEnvContext
 *-------------------------------------------------------------------*/
DbEnvContext::DbEnvContext( )
{
    dbEnv = new DbEnv( 0 );
}

DbEnvContext::~DbEnvContext( )
{
    delete dbEnv;
}

void 
DbEnvContext::open( const DLString &path )
{
    dbEnv->set_lk_max_locks(1000000);        
    dbEnv->open(path.c_str( ), DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_CREATE | DB_RECOVER, 0640);
}

void 
DbEnvContext::close( )
{
    dbEnv->close( 0 );
}

/*-------------------------------------------------------------------
 * TxnContext
 *-------------------------------------------------------------------*/
TxnContext::TxnContext( ) : currentTxn( 0 )
{
}

TxnContext::~TxnContext( )
{
    if(currentTxn) {
        LogStream::sendError( )
            << "Scripting::TxnContext::~TxnContext: "
            << "aborting pending transaction." << endl;

        currentTxn->abort( );
    }
}

bool
TxnContext::txnRunning( )
{
    return currentTxn != NULL;
}

DbTxn *
TxnContext::getCurrentTxn( )
{
    if(!currentTxn)
        getDbEnv( )->dbEnv->txn_begin(NULL, &currentTxn, DB_TXN_SYNC);

    return currentTxn;
}

void
TxnContext::commit( )
{
    if(currentTxn)
        currentTxn->commit( 0 );

    currentTxn = NULL;
}

void
TxnContext::abort( )
{
    if(currentTxn)
        currentTxn->abort( );

    currentTxn = NULL;
}

/*-------------------------------------------------------------------
 * DbContext
 *-------------------------------------------------------------------*/
DbContext::DbContext( ) : db(0)
{
}

DbContext::~DbContext( )
{
    if(db) {
        LogStream::sendError( )
            << "Scripting::DbContext::~DbContext: "
            << "closing pending database" << endl;

        close( );
    }
}

void 
DbContext::open( const char *file, const char *dbname )
{
    try {
        db = new Db(getDbEnv( )->dbEnv, 0);
        db->open( NULL, file, dbname, DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0 );
    } catch(const DbException &ex) {
        LogStream::sendError()
            << "Failed to open DbContext: " 
            << (file ? file : "<unknown>") << ": " 
            << (dbname ? dbname : "<unknown>") << ": " 
            << ex.what( ) << endl;
    }
}

void 
DbContext::close( )
{
    if(!db) {
        LogStream::sendError() << "DbContext already closed" << endl;
        return;
    }
    
    try {
        db->close( 0 );
        delete db;
        db = NULL;
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "Failed to close DbContext: " << ex.what( ) << endl;
    }
}


void
DbContext::load( )
{
    const char *file = 0, *dbname = 0;
    
    db->get_dbname(&file, &dbname);

    LogStream::sendNotice( ) 
        << "Loading DbContext: "
        << (file ? file : "<unknown>") << ":" 
        << (dbname ? dbname : "<unknown>") << "..." << endl;

    int cnt = 0;
    DbTxn *txn;
    
    txn = getCurrentTxn( );

    try {
        Dbt key, val;
        Dbc *cur = 0;

        db->cursor( txn, &cur, 0 );

        while(cur->get( &key, &val, DB_NEXT ) != DB_NOTFOUND) {
            try {
                key_t id = *(key_t *)key.get_data( );
                Data dat(val.get_data( ), val.get_size( ));
                seq(id, dat);
                cnt++;
            } catch(const ::Exception &e) {
                LogStream::sendError() << e.what() << ":" << endl;
            }
        }
        cur->close( );
        commit( );

    } catch(const DbException &ex) {
        LogStream::sendError() << ex.what() << ":" << endl;
        abort( );
    }

    LogStream::sendNotice() << "Total " << cnt << " records loaded" << endl;    
}

void 
DbContext::get( key_t k, Data &dat )
{
    try {
        Dbt key(&k, sizeof(k)), val;

        db->get(getCurrentTxn( ), &key, &val, 0);

        dat.set(val.get_data( ), val.get_size( ));
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "DbContext::get(" << k << "): " 
            << ex.what( ) << endl;
    }
}

void
DbContext::put( key_t k, Data &dat )
{
    try {
        Dbt key(&k, sizeof(k)), val(dat.get_data( ), dat.get_size( ));

        db->put( getCurrentTxn( ), &key, &val, 0 );
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "DbContext::put(" << k << "): " << ex.what( ) << endl;
    }
}

void
DbContext::del( key_t k )
{
    try {
        Dbt key(&k, sizeof(k));

        db->del( getCurrentTxn( ), &key, 0 );
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "DbContext::del(" << k << "): " << ex.what( ) << endl;
    }
}

