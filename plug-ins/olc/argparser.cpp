#include "argparser.h"

namespace TAO_PEGTL_NAMESPACE::mud
{
    ParseException::ParseException( const char * fmt, ... ) throw() {
        va_list ap;
        char buf[1024];

        va_start( ap, fmt );
        vsnprintf( buf, sizeof( buf ), fmt, ap );
        va_end( ap );

        setStr( std::string( buf ) );
    }

    ParseException::~ParseException() {}
}
