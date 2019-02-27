/* $Id: nativeext.h,v 1.1.2.4 2005/04/27 18:46:13 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef NATIVEEXT_H
#define NATIVEEXT_H

#include <sstream>
#include <iomanip>

#include "logstream.h"
#include "native.h"
#include "dlfilestream.h"
#include "dreamland.h"

using namespace std;

// MOC_SKIP_BEGIN
namespace Scripting {

struct api_details {
    string help;
    bool readOnly;
};

typedef map<string, api_details> API;

template <typename TT>
void traitsAPIAux( ostringstream &buf ) 
{
    typename TT::List *list = TT::List::begin();
    
    for ( ; list; list = list->getNext()) {
        const string &name = list->getKey().name;
        const string &help = list->getVal().help;
        buf << "{g" << setiosflags(ios::left) << setw(20) << name << "{w" << resetiosflags(ios::left) << help << endl;
    }
}

template <typename TT>
void traitsAPIAux( API &api, bool readOnly ) 
{
    typename TT::List *list = TT::List::begin();
    
    for ( ; list; list = list->getNext()) {
        struct api_details d;
        d.help = list->getVal().help;
        d.readOnly = readOnly;
        api[list->getKey().name] = d;
    }
}

template <typename T>
void traitsAPI( ostringstream &buf ) 
{
    typedef NativeTraits<T> Traits;
    
    buf << "{Y" << Traits::NAME << "{x" << endl
        << Traits::HELP << endl;

    API api;
    buf << endl << "{WПоля, доступные только для чтения [ro] и для записи [rw]: {x" << endl;

    traitsAPIAux<typename Traits::Get>( api, true );
    traitsAPIAux<typename Traits::Set>( api, false );

    for (API::const_iterator a = api.begin(); a != api.end(); a++)
        buf << "[" << (a->second.readOnly ? "ro" : "rw") << "] "
            << "{g" << setiosflags(ios::left) << setw(18) << a->first << "{x: " << resetiosflags(ios::left)
            << a->second.help << endl;

    buf << endl << "{WМетоды: {x" << endl;
    traitsAPIAux<typename Traits::Invoke>( buf );
}

/** 
 * Save API output for this wrapper to one or two files, in the form of HTML table rows.
 * TODO HTML-escaping
 */
template <typename T>
void traitsAPIDump(const DLString &prefix, bool showFieldStatus, bool methodsSeparately)
{
    typedef NativeTraits<T> Traits;
    ostringstream fbuf, mbuf;
    DLDirectory dir(dreamland->getMiscDir(), "api");

    {
        API api;

        traitsAPIAux<typename Traits::Get>( api, true );
        traitsAPIAux<typename Traits::Set>( api, false );
        for (API::const_iterator a = api.begin(); a != api.end(); a++) {
            fbuf << "<tr><td>"; 
            if (showFieldStatus)
                fbuf << (a->second.readOnly ? "ro":"rw") << "</td><td>";
            fbuf << a->first << "</td><td>" << a->second.help << "</td></tr>";
        }
    }
    {
        API api;

        traitsAPIAux<typename Traits::Invoke>( api, false );
        for (API::const_iterator a = api.begin(); a != api.end(); a++)
            mbuf << "<tr><td>" << a->first << "</td><td>" << a->second.help << "</td></tr>";
    }
    
    try {
        if (methodsSeparately) {
            DLFileStream(dir, prefix + "_methods").fromString(mbuf.str());
            DLFileStream(dir, prefix + "_fields").fromString(fbuf.str());
        } else {
            fbuf << mbuf.str();
            DLFileStream(dir, prefix + "_fields").fromString(fbuf.str());
        }

    } catch (const ExceptionDBIO &ex) {
        LogStream::sendError() << "Error writing API: " << ex.what() << endl;
        return;
    }
}

}

// MOC_SKIP_END
#endif 
