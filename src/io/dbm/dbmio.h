/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */


#ifndef __DBMIO_H__
#define __DBMIO_H__

#include <sys/types.h>
#include <db.h>
#include <fcntl.h>
#include <limits.h>

#include <dlstring.h>
#include <exception.h>

class DBMIO {
public:
    typedef unsigned long int Key;
    struct Exception : public ::Exception {
	Exception(const DLString &ex) : ::Exception(ex) { }
    };
    struct EOFException : public Exception { 
	EOFException() : Exception("eof") { }
    };

    DBMIO();
    ~DBMIO();

    void open(const char *fname, int flags, int mode = 0644);
    void sync();
    void close();

    void del(const Key &k);
    void get(const Key &k, DLString &v);
    void put(const Key &k, const DLString &v);
    void seq(Key &k, DLString &v, unsigned int flags);

private:
    static u_int32_t hash(const void *, size_t);
    
    DB *db;
};

#endif
