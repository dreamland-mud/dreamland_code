/* $Id: pstream.h,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __PSTREAM_H__
#define __PSTREAM_H__

#include <string.h>
#include <streambuf>
#include <string>

using namespace std;

class pios {
public:
    virtual ~pios( ) { };
protected:
    virtual streambuf *buf() = 0;
public:
    typedef int size_type;
};

class opstream : virtual pios {
public:
    opstream &operator << (void (*m)(opstream &)) {
        (*m)(*this);
        return *this;
    }
    opstream &operator << (char c) {
        buf()->sputc(c);
        return *this;
    }
    opstream &operator << (short s) {
        buf()->sputn((const char *)&s, sizeof(short));
        return *this;
    }
    opstream &operator << (int i) {
        buf()->sputn((const char *)&i, sizeof(int));
        return *this;
    }
    opstream &operator << (long l) {
        buf()->sputn((const char *)&l, sizeof(long));
        return *this;
    }
    opstream &operator << (const string &s) {
        size_type l = s.length( );
        buf()->sputn((const char *)&l, sizeof(l));
        buf()->sputn(s.data(), l);
        return *this;
    }
    opstream &operator << (const char *s) {
        size_type l = strlen(s);
        buf()->sputn((const char *)&l, sizeof(l));
        buf()->sputn(s, l);
        return *this;
    }

    void flush() {
        buf()->pubsync( );
    }
};

inline void flush(opstream &p) {
    p.flush( );
}


class ipstream : virtual pios {
public:
    ipstream &operator >> (void (*m)(ipstream &)) {
        (*m)(*this);
        return *this;
    }
    ipstream &operator >> (char &c) {
        c = buf()->sbumpc();
        return *this;
    }
    ipstream &operator >> (short &s) {
        buf()->sgetn((char *)&s, sizeof(short));
        return *this;
    }
    ipstream &operator >> (int &i) {
        buf()->sgetn((char *)&i, sizeof(int));
        return *this;
    }
    ipstream &operator >> (long &l) {
        buf()->sgetn((char *)&l, sizeof(long));
        return *this;
    }
    ipstream &operator >> (string &s) {
        size_type l;
        *this >> l;
        char b[l];
        buf()->sgetn(b, l);
        s.assign(b, l);
        return *this;
    }
    template <int I>
    ipstream &operator >> (char (&s)[I]) {
        size_type l;
        *this >> l;
        if(l > I-1)
            l = I-1;
        buf()->sgetn(s, l);
        return *this;
    }
};

#endif
