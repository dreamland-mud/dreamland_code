#include <string>
#include <sstream>
#include <random>

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

/**
 * Generate a random string from an unambiguous alphabet (no 0 O 1 I L), seeded from
 * the system CSPRNG. For codes that gate identity -- account linking codes and the like.
 * create_nonce() above is rand()-based and predictable; fine for cosmetic tokens, not these.
 */
string create_secure_nonce(int len)
{
    static const char alphabet[] = "ABCDEFGHJKMNPQRSTUVWXYZ23456789";
    std::random_device rd;
    std::uniform_int_distribution<size_t> dist(0, sizeof(alphabet) - 2);

    ostringstream buf;
    for (int i = 0; i < len; ++i)
        buf << alphabet[dist(rd)];
    return buf.str();
}

/** CSPRNG-seeded decimal digits, for emailed one-time codes. */
string create_secure_digits(int len)
{
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 9);

    ostringstream buf;
    for (int i = 0; i < len; ++i)
        buf << dist(rd);
    return buf.str();
}

int dice_ave(int d1, int d2)
{
    return (d2 + 1) * d1 / 2;
}

int signum(float x)
{
    if (x < 0.0)
        return -1;
    if (x > 0.0)
        return 1;
    return 0;
}

int percentage(int original, int percent) 
{
    if (percent == 0)
        return original;
    else 
        return original * (100 + percent) / 100;
}