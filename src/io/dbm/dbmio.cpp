/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <logstream.h>
#include <sstream>

#include "dbmio.h"

DBMIO::DBMIO() : db(0)
{
}

DBMIO::~DBMIO()
{
    if(db) {
	try {
	    close();
	} catch(const DBMIO::Exception &ex) {
	    LogStream::sendError() 
		<< "Exception " << ex.what()
		<< " caused by DBMIO::close() called from destructor" << endl;
	}
    }
}

u_int32_t 
DBMIO::hash(const void *k, size_t ksize)
{
    if(ksize != sizeof(Key)) {
	ostringstream os;
	DLString key;
	key.assign((const char *)k, ksize);
	    
	os << "hash: wrong key size ksize=" << ksize 
	   << " sizeof(Key)=" << sizeof(Key) 
	   << " key='" << key << "'" << endl;

	LogStream::sendError() << os.str( );
//	throw DBMIO::Exception(os.str( ));
    }

    return *(Key*)k;
}

void
DBMIO::open(const char *fname, int flags, int mode)
{
    HASHINFO hi;
    
    bzero(&hi, sizeof(hi));

    hi.cachesize = 1024*1024;

    db = dbopen(fname, flags, mode, DB_HASH, &hi);

    if(!db)
	throw DBMIO::Exception(strerror(errno));
}

void 
DBMIO::sync()
{
    if(!db)
	throw DBMIO::Exception("no database opened");

    if(db->sync(db, 0))
	throw DBMIO::Exception(strerror(errno));
}

void 
DBMIO::close()
{
    if(!db)
	throw DBMIO::Exception("no database opened");

    if(db->close(db))
	throw DBMIO::Exception(strerror(errno));

    db = 0;
}


void 
DBMIO::del(const Key &k)
{
    Key kCopy = k;
    DBT key;
    int rc;
    
    if(!db)
	throw DBMIO::Exception("no database opened");

    key.data = &kCopy;
    key.size = sizeof(kCopy);
    rc = db->del(db, &key, 0);
    
    if(rc < 0)
	throw DBMIO::Exception(strerror(errno));

    if(rc > 0)
	throw DBMIO::Exception("key not in database");
}

void 
DBMIO::get(const Key &k, DLString &v)
{
    Key kCopy = k;
    DBT key, val;
    int rc;
    const char *d;
    
    if(!db)
	throw DBMIO::Exception("no database opened");

    key.data = &kCopy;
    key.size = sizeof(kCopy);
    val.data = 0;
    val.size = 0;
    rc = db->get(db, &key, &val, 0);
    
    if(rc < 0)
	throw DBMIO::Exception(strerror(errno));

    if(rc > 0)
	throw DBMIO::Exception("key not in database");

    d = (const char *)val.data;
    v.assign(d, val.size);
}

void 
DBMIO::put(const Key &k, const DLString &v)
{
    Key kCopy = k;
    DBT key, val;
    int rc;
    char data[v.size()];
    
    if(!db)
	throw DBMIO::Exception("no database opened");

    copy(v.begin(), v.end(), data);

    key.data = &kCopy;
    key.size = sizeof(kCopy);
    val.data = data;
    val.size = v.size();
    if(val.size > 4096)
	LogStream::sendError() << k << ": extreme record size: " << val.size << endl;
    
    rc = db->put(db, &key, &val, 0);
    
    if(rc < 0)
	throw DBMIO::Exception(strerror(errno));

    if(rc > 0)
	throw DBMIO::Exception("key already in database");
}

void
DBMIO::seq(Key &k, DLString &v, unsigned int flags)
{
    DBT key, val;
    int rc;
    const char *d;
    
    if(!db)
	throw DBMIO::Exception("no database opened");

    key.data = 0;
    key.size = 0;
    val.data = 0;
    val.size = 0;
    rc = db->seq(db, &key, &val, flags);
    
    if(rc < 0)
	throw DBMIO::Exception(strerror(errno));

    if(rc > 0)
	throw DBMIO::EOFException();

    if(key.size != sizeof(Key))
	throw DBMIO::Exception("seq: wrong key size");
    
    k = *(Key*)key.data;
    
    d = (const char *)val.data;
    v.assign(d, val.size);
}

