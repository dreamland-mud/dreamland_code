/* $Id$
 *
 * ruffina, Dream Land, 2018
 */


#ifndef ICONVMAP_H
#define ICONVMAP_H

#include <iconv.h>

class IconvMap {
public:
    IconvMap(const char *from, const char *to);
    ~IconvMap();

    std::string operator() (const std::string &src);

private:
    iconv_t icnv_desc;
};

#endif
