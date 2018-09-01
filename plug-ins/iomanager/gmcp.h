#ifndef GMCP_H
#define GMCP_H

#include <string>

using namespace std;
class Descriptor;

class GMCPHandler {
public:
        static void sendVersion(Descriptor *d);

private:
        static void send(Descriptor *d, const string &package, const string &message, const string &data);
};

#endif
