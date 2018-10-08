/* $Id: txncontext.cpp,v 1.1.6.3 2010-01-01 15:14:15 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */

#include "txncontext.h"
#include "exception.h"
#include "logstream.h"

#include "config.h"

#ifdef HAS_BDB
#include <db_cxx.h>
#else
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#include <sstream>

using namespace std;

struct DbEnv {
    DbEnv( int ) { }
    DLString path;
};

struct Db {
    DLString file, name;
};

#endif

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
#ifdef HAS_BDB
    dbEnv->open(path.c_str( ), DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN | DB_CREATE | DB_RECOVER, 0640);
#else
    dbEnv->path = path;
#endif
}

void 
DbEnvContext::close( )
{
#ifdef HAS_BDB
    dbEnv->close( 0 );
#else
    dbEnv->path = "";
#endif
}

/*-------------------------------------------------------------------
 * TxnContext
 *-------------------------------------------------------------------*/
TxnContext::TxnContext( ) : currentTxn( 0 )
{
}

TxnContext::~TxnContext( )
{
#ifdef HAS_BDB
    if(currentTxn) {
        LogStream::sendError( )
            << "Scripting::TxnContext::~TxnContext: "
            << "aborting pending transaction." << endl;

        currentTxn->abort( );
    }
#endif
}

bool
TxnContext::txnRunning( )
{
    return currentTxn != NULL;
}

DbTxn *
TxnContext::getCurrentTxn( )
{
#ifdef HAS_BDB
    if(!currentTxn)
        getDbEnv( )->dbEnv->txn_begin(NULL, &currentTxn, DB_TXN_SYNC);
#endif

    return currentTxn;
}

void
TxnContext::commit( )
{
#ifdef HAS_BDB
    if(currentTxn)
        currentTxn->commit( 0 );
#endif

    currentTxn = NULL;
}

void
TxnContext::abort( )
{
#ifdef HAS_BDB
    if(currentTxn)
        currentTxn->abort( );
#else
        LogStream::sendError( )
            << "Scripting::TxnContext::~TxnContext: "
            << "rollback is not supported by database vendor"
            << endl;
#endif

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
#ifdef HAS_BDB
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
#else
    db = new Db( );
    db->file = file;
    db->name = dbname;
#endif
}

void 
DbContext::close( )
{
    if(!db) {
        LogStream::sendError() << "DbContext already closed" << endl;
        return;
    }
    
#if BDB
    try {
        db->close( 0 );
        delete db;
        db = NULL;
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "Failed to close DbContext: " << ex.what( ) << endl;
    }
#else
    delete db;
    db = NULL;
#endif
}


void
DbContext::load( )
{
    const char *file = 0, *dbname = 0;
    
#ifdef HAS_BDB
    db->get_dbname(&file, &dbname);
#else
    file = db->file.c_str( );
    dbname = db->name.c_str( );
#endif

    LogStream::sendNotice( ) 
        << "Loading DbContext: "
        << (file ? file : "<unknown>") << ":" 
        << (dbname ? dbname : "<unknown>") << "..." << endl;

    int cnt = 0;
    DbTxn *txn;
    
    txn = getCurrentTxn( );

#ifdef HAS_BDB
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
#else

    int i;
    DIR *dirp;
    dirent *dp;
    
    for(i=0;i<16;i++) {
        ostringstream fname;
        fname << getDbEnv( )->dbEnv->path << "/" 
              << db->file << "-" << db->name << "/"
              << i;
            
        dirp = opendir(fname.str( ).c_str( ));
        if(dirp == NULL)
            continue;
        
        while( (dp = readdir(dirp)) != NULL ) {
            DLString fileName(dp->d_name);

            DLString::size_type pos = fileName.rfind( '.' );
            
            if(pos == DLString::npos)
                continue;

            DLString ext = fileName.substr(pos);
            
            if(ext != ".xml" && ext != ".XML")
                continue;

            fileName = fileName.substr(0, pos);
            
            if(fileName.empty( ) || !fileName.isNumber( ))
                continue;

            key_t id = fileName.toLong( );

            fileName = DLString(fname.str( )) + "/" + dp->d_name;
            
            struct stat sb;
            
            if(stat(fileName.c_str( ), &sb) < 0)
                continue;
            
            if(!S_ISREG(sb.st_mode))
                continue;

            FILE *f = fopen(fileName.c_str( ), "rb");

            if(f == NULL)
                continue;

            fseek(f, 0, SEEK_END);
            unsigned int size = ftell(f);
            fseek(f, 0, SEEK_SET);
            char buf[size];

            int c;
            if((c = fread(buf, size, 1, f)) != 1)
                LogStream::sendError() 
                    << "DbContext::seq(" << id << "): "
                    << "truncated record: " << size 
                    << ", " << sb.st_size
                    << ", " << c 
                    << ": " << strerror(errno) << endl;

            fclose(f);
            
            Data dat(buf, size);
            seq(id, dat);
            cnt++;
        }
        
        closedir(dirp);
    }
    
#endif

    LogStream::sendNotice() << "Total " << cnt << " records loaded" << endl;
    
}

void 
DbContext::get( key_t k, Data &dat )
{
#ifdef HAS_BDB
    try {
        Dbt key(&k, sizeof(k)), val;

        db->get(getCurrentTxn( ), &key, &val, 0);

        dat.set(val.get_data( ), val.get_size( ));
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "DbContext::get(" << k << "): " 
            << ex.what( ) << endl;
    }
#else
    ostringstream fname;
    fname << getDbEnv( )->dbEnv->path << "/" 
          << db->file << "-" << db->name << "/"
          << (k % 16) << "/"
          << k << ".xml";
    
    FILE *f = fopen(fname.str( ).c_str( ), "rb");
    if(f == NULL) {
        LogStream::sendError() 
            << "DbContext::get(" << k << "): "
            << strerror(errno) << endl;
        return;
    }

    fseek(f, 0, SEEK_END);
    unsigned int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    /*!!!XXX - not reenterable!!!*/
    static Data dta(0, 0);

    if(dta.get_data( ))
        delete [] (char *)dta.get_data( );
    
    dta.set(new char[size], size);

    if(fread(dta.get_data( ), dta.get_size( ), 1, f) != 1)
        LogStream::sendError() 
            << "DbContext::get(" << k << "): "
            << "promised and actual record size missmatch"
            << endl;

    fclose(f);

    dat = dta;
#endif
}

void
DbContext::put( key_t k, Data &dat )
{
#ifdef HAS_BDB
    try {
        Dbt key(&k, sizeof(k)), val(dat.get_data( ), dat.get_size( ));

        db->put( getCurrentTxn( ), &key, &val, 0 );
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "DbContext::put(" << k << "): " << ex.what( ) << endl;
    }
#else
    ostringstream fname;
    fname << getDbEnv( )->dbEnv->path << "/" 
          << db->file << "-" << db->name << "/"
          << (k % 16) << "/"
          << k << ".xml";
    
    FILE *f = fopen(fname.str( ).c_str( ), "wb");
    if(f == NULL) {
        LogStream::sendError() 
            << "DbContext::put(" << k << "): " 
            << fname.str( ) << ": " << strerror(errno) << endl;
        return;
    }

    if(fwrite(dat.get_data( ), dat.get_size( ), 1, f) != 1)
        LogStream::sendError() 
            << "DbContext::put(" << k << "): "
            << "record truncated" << endl;
        ;

    fclose(f);
#endif
}

void
DbContext::del( key_t k )
{
#ifdef HAS_BDB
    try {
        Dbt key(&k, sizeof(k));

        db->del( getCurrentTxn( ), &key, 0 );
    } catch(const DbException &ex) {
        LogStream::sendError() 
            << "DbContext::del(" << k << "): " << ex.what( ) << endl;
    }
#else
    ostringstream fname;
    fname << getDbEnv( )->dbEnv->path << "/" 
          << db->file << "-" << db->name << "/"
          << (k % 16) << "/"
          << k << ".xml";
    
    if(unlink(fname.str( ).c_str( )) < 0)
        LogStream::sendError() 
            << "DbContext::del(" << k << "): " 
            << fname.str( ) << ": " << strerror(errno) << endl;
#endif
}

