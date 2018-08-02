/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <list>
#include <istream>
#include <ostream>

using namespace std;

#include "dlstring.h"

class Formatter {
public:
    Formatter(ostream &o, istream &i);
    virtual ~Formatter();

    void format(unsigned int tab = 8, unsigned int width = 70);

private:
    typedef list<DLString> words_t;

    ostream &os;
    istream &is;
    words_t words;

    virtual bool is_separator(char c);
    bool getword(DLString &word);
    bool skipspaces();
    void printline(unsigned int tab, unsigned int width);
};

#endif

