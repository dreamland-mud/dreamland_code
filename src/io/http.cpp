
#include <ostream>

#include "http.h"

using namespace std;

ostream &
operator << (ostream &os, const HttpResponse &response)
{
    os << response.proto << " " << response.status << " " << response.message << "\r\n";

    for(map<string, string>::const_iterator it=response.headers.begin();it!=response.headers.end();it++)
        os << it->first << ": " << it->second << "\r\n";
    
    os << "\r\n" << response.body;
    return os;
}

