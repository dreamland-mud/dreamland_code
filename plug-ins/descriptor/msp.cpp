#include <sstream>

#include "msp.h"
#include "descriptor.h"
#include "character.h"
#include "def.h"

/* The web client serves the library here, and Mudlet downloads from the same
 * place by U=, so one sound has one address everywhere. */
static const char *MSP_URL = "https://dreamland.rocks/play/sounds/";

bool msp_enabled( Descriptor *d )
{
    return d && IS_SET(d->oob_proto, OOB_MSP);
}

/* Library names only: lower-case path segments, no "..", no absolute path.
 * Anything else would let a script point Mudlet's downloader elsewhere. */
static bool valid_name( const DLString &name )
{
    if (name.empty( ) || name.size( ) > 64 || name[0] == '/' || name[name.size( ) - 1] == '/')
        return false;

    if (name.find( ".." ) != DLString::npos || name.find( "//" ) != DLString::npos)
        return false;

    for (DLString::size_type i = 0; i < name.size( ); i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '/'))
            return false;
    }

    return true;
}

static int clamp( int v, int lo, int hi )
{
    return v < lo ? lo : (v > hi ? hi : v);
}

bool msp_sound( Descriptor *d, const DLString &name, int volume, int loops, int priority )
{
    if (!msp_enabled( d ) || !valid_name( name ))
        return false;

    volume = clamp( volume, 0, 100 );
    loops = clamp( loops, -1, 100 );
    if (loops == 0)
        loops = 1;
    priority = clamp( priority, 0, 100 );

    DLString::size_type slash = name.find( '/' );
    DLString type = slash == DLString::npos ? DLString( "alert" ) : name.substr( 0, slash );

    std::ostringstream buf;

    /* The trigger must start a line. The server ends lines with "\n\r", and
     * the web client takes a line start after either character. */
    if (d->outtop > 0 && d->outbuf[d->outtop - 1] != '\n' && d->outbuf[d->outtop - 1] != '\r')
        buf << "\n\r";

    buf << "!!SOUND(" << name << ".mp3"
        << " V=" << volume << " L=" << loops << " P=" << priority
        << " T=" << type << " U=" << MSP_URL << ")\n\r";

    d->send( buf.str( ).c_str( ) );
    return true;
}

bool msp_sound( Character *ch, const DLString &name, int volume, int loops, int priority )
{
    if (!ch || ch->is_npc( ) || !ch->desc)
        return false;

    return msp_sound( ch->desc, name, volume, loops, priority );
}
