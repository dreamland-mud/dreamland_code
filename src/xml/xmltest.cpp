/* $Id: xmltest.cpp,v 1.1.2.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "xmldocument.h"
#include "fpstream.h"
#include "profiler.h"

#include <fstream>

using namespace std;

int
main(int argc, char **argv)
{
    XMLDocument::Pointer doc(NEW);
    doc->load(cin);

    {
        ProfilerBlock pb("save xml");
        fstream ofs("1", ios::out);
        doc->save(ofs);
    }

    {
        ProfilerBlock pb("save bin");
        fpstream ofp;
        ofp.open("2", ios::out);
        ofp << **doc;
    }

    {
        ProfilerBlock pb("load xml");
        XMLDocument::Pointer d(NEW);
        fstream ifs("1", ios::in);
        d->load(ifs);
    }

    {
        ProfilerBlock pb("load bin");
        XMLDocument::Pointer d(NEW);
        fpstream ifp;
        ifp.open("2", ios::in);
        ifp >> **d;
    }
}
