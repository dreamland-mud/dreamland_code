/* $Id: nativeext.h,v 1.1.2.4 2005/04/27 18:46:13 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef NATIVEEXT_H
#define NATIVEEXT_H

#include <sstream>
#include <iomanip>
#include <jsoncpp/json/json.h>

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
typedef map<string, Json::Value> JsonAPI;

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

template <typename TT>
void traitsAPIJsonAux( JsonAPI &api, bool readOnly ) 
{
    typename TT::List *list = TT::List::begin();
    
    for ( ; list; list = list->getNext()) {
        Json::Value details;
        details["help"] = list->getVal().help;
        details["ro"] = readOnly;
        details["name"] = list->getKey().name;
        api[list->getKey().name] = details;
    }
}

template <typename T>
void traitsAPIJson(const DLString &prefix, Json::Value &apiDump, bool splitMethodsAndFields)
{
    typedef NativeTraits<T> Traits;
    JsonAPI api;
    Json::Value fields;
    Json::Value methods;

    traitsAPIJsonAux<typename Traits::Get>( api, true );
    traitsAPIJsonAux<typename Traits::Set>( api, false );

    for (auto &apiEntry: api)
        if (splitMethodsAndFields)
            fields.append(apiEntry.second);
        else
            methods.append(apiEntry.second);

    api.clear();

    traitsAPIJsonAux<typename Traits::Invoke>( api, false );
    for (auto &apiEntry: api)
        methods.append(apiEntry.second);

    if (splitMethodsAndFields) {
        apiDump[prefix + "_fields"] = fields;
        apiDump[prefix + "_methods"] = methods;

    } else {
        apiDump[prefix + "_methods"] = methods;
    }
}


}

// MOC_SKIP_END
#endif 
