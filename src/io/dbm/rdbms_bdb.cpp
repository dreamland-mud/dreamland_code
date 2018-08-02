/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#include "logstream.h"
#include "rdbms_bdb.h"

template <>
Dbt DbContext<unsigned long int>::key2dbt( key_t k )
{
    return Dbt(&k, sizeof(k));
}

Dbt DbContext<unsigned long long>::key2dbt( key_t k )
{
    return Dbt(&k, sizeof(k));
}

Dbt DbContext<DLString>::key2dbt( key_t k )
{
    return Dbt((void *)k.c_str( ), k.size( ));
}

void DbContext<unsigned long int>::dbt2key(Dbt &dbt, key_t &k)
{
    k = *(unsigned long int *)dbt.get_data( );
}

void DbContext<unsigned long long>::dbt2key(Dbt &dbt, key_t &k)
{
    k = *(unsigned long long *)dbt.get_data( );
}

void DbContext<DLString>::dbt2key(Dbt &dbt, key_t &k)
{
    k.assign( (char *)dbt.get_data( ), dbt.get_size( ) );
}


/*-------------------------------------------------------------------
 * DbContext
 *-------------------------------------------------------------------*/

template <typename key_t>
DbContext<key_t>::DbContext( ) : db(0)
{
}

template <typename key_t>
DbContext<key_t>::~DbContext( )
{
    if(db) {
	LogStream::sendError( )
	    << "DbContext::~DbContext: "
	    << "closing pending database" << endl;

	close( );
    }
}

template <typename key_t>
void 
DbContext<key_t>::open( const char *file, const char *dbname, bool fDup )
{
    try {
	db = new Db(getDbEnv( )->dbEnv, 0);
	db->open( NULL, 
	          file, 
		  dbname, 
		  DB_BTREE, 
		  DB_CREATE | DB_AUTO_COMMIT | (fDup ? DB_DUP : 0), 
		  0 );
    } 
    catch(const DbException &ex) {
	LogStream::sendError()
	    << "Failed to open DbContext: " 
	    << (file ? file : "<unknown>") << ": " 
	    << (dbname ? dbname : "<unknown>") << ": " 
	    << ex.what( ) << endl;
    }
}

template <typename key_t>
void 
DbContext<key_t>::close( )
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


template <typename key_t>
void
DbContext<key_t>::load( )
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
		key_t id;

		dbt2key(key, id);
		seq(id, val.get_data( ), val.get_size( ));
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

template <typename key_t>
void 
DbContext<key_t>::get( key_t k, void *& d_val, size_t &d_size )
{
    try {
	Dbt key = key2dbt( k ), val;

	db->get(getCurrentTxn( ), &key, &val, DB_DBT_MALLOC);
	
	d_val = val.get_data( );
	d_size = val.get_size( );
    } catch(const DbException &ex) {
	LogStream::sendError() 
	    << "DbContext::get(" << k << "): " 
	    << ex.what( ) << endl;
    }
}

template <typename key_t>
void
DbContext<key_t>::put( key_t k, void * d_val, size_t d_size )
{
    try {
	Dbt key = key2dbt( k ), val(d_val, d_size);

	db->put( getCurrentTxn( ), &key, &val, 0 );
    } catch(const DbException &ex) {
	LogStream::sendError() 
	    << "DbContext::put(" << k << "): " << ex.what( ) << endl;
    }
}

template <typename key_t>
void
DbContext<key_t>::del( key_t k )
{
    try {
	Dbt key = key2dbt( k );

	db->del( getCurrentTxn( ), &key, 0 );
    } catch(const DbException &ex) {
	LogStream::sendError() 
	    << "DbContext::del(" << k << "): " << ex.what( ) << endl;
    }
}

template <typename key_t>
void
DbContext::delBoth( key_t k1, key_t k2 )
{
    Dbt dbtK1(key2dbt(k1)), dbtK2(key2dbt(k2));
    Dbc *cursor;

    db->cursor( getCurrentTxn( ), &cursor, DB_WRITECURSOR );

    cursor->get( &dbtK1, &dbtK2, DB_GET_BOTH );

    cursor->del( 0 );
    cursor->close( );
}



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
    dbEnv->open(path.c_str( ), DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL |
                                DB_INIT_TXN | DB_INIT_CDB | DB_CREATE , 0640);
}

void 
DbEnvContext::close( )
{
    dbEnv->close( 0 );
}



/*-------------------------------------------------------------------
 * DbTxnContext
 *-------------------------------------------------------------------*/
DbTxnContext::DbTxnContext( ) : currentTxn( 0 )
{
}

DbTxnContext::~DbTxnContext( )
{
    if(currentTxn) {
	LogStream::sendError( )
	    << "Scripting::DbTxnContext::~DbTxnContext: "
	    << "aborting pending transaction." << endl;

	currentTxn->abort( );
    }
}

bool
DbTxnContext::txnRunning( )
{
    return currentTxn != NULL;
}

DbTxn *
DbTxnContext::getCurrentTxn( )
{
    if(!currentTxn)
	getDbEnv( )->dbEnv->txn_begin(NULL, &currentTxn, DB_TXN_SYNC);

    return currentTxn;
}

void
DbTxnContext::commit( )
{
    if(currentTxn)
	currentTxn->commit( 0 );

    currentTxn = NULL;
}

void
DbTxnContext::abort( )
{
    if(currentTxn)
	currentTxn->abort( );

    currentTxn = NULL;
}

