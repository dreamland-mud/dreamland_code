#include <string>
#include <sstream>

using namespace std;

float linear_interpolation(float x, float x1, float x2, float y1, float y2 )
{
    if (x2 == x1)
        return y1;

    return y1 + ((x - x1) / (x2 - x1)) * (y2 - y1);
}


/** Generate random alnum string of given length. */
string create_nonce(int len)
{
    ostringstream buf;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        buf << alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return buf.str();
}

int dice_ave(int d1, int d2)
{
    return (d2 + 1) * d1 / 2;
}

