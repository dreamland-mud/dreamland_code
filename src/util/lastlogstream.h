/* $Id: lastlogstream.h,v 1.1.2.1.6.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef __LASTLOGSTREAM_H__
#define __LASTLOGSTREAM_H__

#include <sstream>

using namespace std;

class LastLogStream {
public:
    virtual ~LastLogStream( );
    static ostream & send( );
    static void clear( );

private:
    static ostringstream data;
};

#endif
