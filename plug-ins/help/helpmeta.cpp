#include <sstream>

#include "helpmeta.h"

using namespace std;

const char *HELP_META_PAD = "  {W*{x ";

DLString help_meta_line(const DLString &label, const DLString &value)
{
    ostringstream buf;

    buf << HELP_META_PAD << "{c" << label << "{x: " << value;
    return buf.str();
}
