/* $Id: fpstream.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __FPSTREAM_H__
#define __FPSTREAM_H__

#include "pstream.h"

#include <fstream>

using namespace std;

class fpstream : public opstream, public ipstream {
public:
    void open(const char *fname, ios::openmode m) {
        fb.open(fname, m);
    }

    void close( ) {
        fb.close( );
    }

    void seek(int p) {
        fb.pubseekpos(p);
    }

private:
    filebuf fb;
protected:
    virtual streambuf *buf() {
        return &fb;
    }
};

#endif
