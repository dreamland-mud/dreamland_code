/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#include <sstream>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#include "logstream.h"

#include "rdbms_fs.h"

#define HASH_SIZE 16

void string2key( const DLString &str, unsigned long int &k )
{
    k = str.toLong( );
}

void string2key( const DLString &str, unsigned long long &k )
{
    k = str.toLongLong( );
}

void string2key( const DLString &str, DLString &k )
{
    k = str;
}

int key2hash( unsigned long int k )
{
    return k % HASH_SIZE;
}

int key2hash( unsigned long long k )
{
    return k % HASH_SIZE;
}

int key2hash( const DLString &k )
{
    const char *pk = k.c_str( );
    char h = 0;
    
    while (*pk)
        h ^= *pk++;
    
    return h % HASH_SIZE;
}


/*-------------------------------------------------------------------
 * DbContext
 *-------------------------------------------------------------------*/

template <typename key_t>
DbContext<key_t>::DbContext( ) : file( NULL ), dbname( NULL )
{
}

template <typename key_t>
DbContext<key_t>::~DbContext( )
{
    if (file) {
        LogStream::sendError( )
            << "Scripting::DbContext::~DbContext: "
            << "closing pending database" << endl;

        close( );
    }
}

template <typename key_t>
void 
DbContext<key_t>::open( const char *file, const char *dbname )
{
    this->file = file;
    this->dbname = dbname;
}

template <typename key_t>
void 
DbContext<key_t>::close( )
{
    if(!file) {
        LogStream::sendError() << "DbContext already closed" << endl;
        return;
    }
    
    file = NULL;
    dbname = NULL;
}


template <typename key_t>
void
DbContext<key_t>::load( )
{
    int cnt = 0;
    int i;
    DIR *dirp;
    dirent *dp;
    
    LogStream::sendNotice( ) 
        << "Loading DbContext: "
        << (file ? file : "<unknown>") << ":" 
        << (dbname ? dbname : "<unknown>") << "..." << endl;

    for(i=0;i<HASH_SIZE;i++) {
        ostringstream fname;
        fname << getDbEnv( )->path << "/" 
              << file << "-" << dbname << "/"
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

            key_t id;
            
            string2key(fileName, id);

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
            
            seq(id, buf, size);
            cnt++;
        }
        
        closedir(dirp);
    }
    
    LogStream::sendNotice() << "Total " << cnt << " records loaded" << endl;
}

template <typename key_t>
void 
DbContext<key_t>::get( key_t k, void *& d_val, size_t &d_size )
{
    FILE *f = fopen(getFileName( k ).c_str( ), "rb");
    if(f == NULL) {
        LogStream::sendError() 
            << "DbContext::get(" << k << "): "
            << strerror(errno) << endl;
        return;
    }

    fseek(f, 0, SEEK_END);
    d_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    d_val = malloc(d_size);
    
    if (fread(d_val, d_size, 1, f) != 1)
        LogStream::sendError() 
            << "DbContext::get(" << k << "): "
            << "promised and actual record size mismatch"
            << endl;

    fclose(f);
}

template <typename key_t>
void
DbContext<key_t>::put( key_t k, void * d_val, size_t d_size )
{
    DLString fname = getFileName( k );
    FILE *f = fopen(fname.c_str( ), "wb");
    if(f == NULL) {
        LogStream::sendError() 
            << "DbContext::put(" << k << "): " 
            << fname << ": " << strerror(errno) << endl;
        return;
    }

    if(fwrite(d_val, d_size, 1, f) != 1)
        LogStream::sendError() 
            << "DbContext::put(" << k << "): "
            << "record truncated" << endl;
        ;

    fclose(f);
}

template <typename key_t>
void
DbContext<key_t>::del( key_t k )
{
    DLString fname = getFileName( k );
    if(unlink(fname.c_str( )) < 0)
        LogStream::sendError() 
            << "DbContext::del(" << k << "): " 
            << fname << ": " << strerror(errno) << endl;
}

template <typename key_t>
DLString
DbContext<key_t>::getFileName( key_t k )
{
    ostringstream fname;
    
    fname << getDbEnv( )->path << "/" 
          << file << "-" << dbname << "/"
          << key2hash( k ) << "/"
          << k << ".xml";
    return fname.str( );
}

/*-------------------------------------------------------------------
 * DbEnvContext
 *-------------------------------------------------------------------*/
DbEnvContext::DbEnvContext( )
{
}

DbEnvContext::~DbEnvContext( )
{
}

void 
DbEnvContext::open( const DLString &path )
{
    this->path = path;
}

void 
DbEnvContext::close( )
{
    path = "";
}



/*-------------------------------------------------------------------
 * DbTxnContext
 *-------------------------------------------------------------------*/
DbTxnContext::DbTxnContext( ) 
{
}

DbTxnContext::~DbTxnContext( )
{
}

bool
DbTxnContext::txnRunning( )
{
    return false;
}

void
DbTxnContext::commit( )
{
    LogStream::sendError( )
        << "DbTxnContext: "
        << "commit is not supported by database vendor"
        << endl;
}

void
DbTxnContext::abort( )
{
    LogStream::sendError( )
        << "DbTxnContext: "
        << "rollback is not supported by database vendor"
        << endl;
}



