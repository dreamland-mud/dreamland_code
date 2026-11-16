#include <sstream>
#include <set>

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

/* The trigger must start a line. The server ends lines with "\n\r", and
 * the web client takes a line start after either character. */
static void line_start( Descriptor *d, std::ostringstream &buf )
{
    if (d->outtop > 0 && d->outbuf[d->outtop - 1] != '\n' && d->outbuf[d->outtop - 1] != '\r')
        buf << "\n\r";
}

/* MSP's T= field: the name's first path segment. */
static DLString sound_type( const DLString &name )
{
    DLString::size_type slash = name.find( '/' );
    return slash == DLString::npos ? DLString( "alert" ) : name.substr( 0, slash );
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

    std::ostringstream buf;
    line_start( d, buf );
    buf << "!!SOUND(" << name << ".mp3"
        << " V=" << volume << " L=" << loops << " P=" << priority
        << " T=" << sound_type( name ) << " U=" << MSP_URL << ")\n\r";

    d->send( buf.str( ).c_str( ) );
    return true;
}

bool msp_sound( Character *ch, const DLString &name, int volume, int loops, int priority )
{
    if (!ch || ch->is_npc( ) || !ch->desc)
        return false;

    return msp_sound( ch->desc, name, volume, loops, priority );
}

/* Innermost open watch; each links to the one it shadows. */
static MspAlertWatch *alertWatches = 0;

MspAlertWatch::MspAlertWatch( )
    : prev( alertWatches )
{
    alertWatches = this;
}

MspAlertWatch::~MspAlertWatch( )
{
    alertWatches = prev;
}

bool msp_affect_alert( Character *ch, const DLString &name, int priority )
{
    if (!msp_sound( ch, name, 100, 1, priority ))
        return false;

    for (MspAlertWatch *w = alertWatches; w; w = w->prev)
        if (w->watched.count( ch->desc ))
            w->alerted.insert( ch->desc );

    return true;
}

bool msp_music( Descriptor *d, const DLString &name, int volume )
{
    if (!msp_enabled( d ) || (!name.empty( ) && !valid_name( name )))
        return false;

    std::ostringstream buf;
    line_start( d, buf );

    if (name.empty( ))
        buf << "!!MUSIC(Off)\n\r";
    else
        /* C=1: a client already playing this track keeps playing it instead of
         * restarting, so resending the same loop costs nothing audible. */
        buf << "!!MUSIC(" << name << ".mp3"
            << " V=" << clamp( volume, 0, 100 ) << " L=-1 C=1"
            << " T=" << sound_type( name ) << " U=" << MSP_URL << ")\n\r";

    d->send( buf.str( ).c_str( ) );
    return true;
}

/* Connections whose client (re)announced MSP since the last ambience pass.
 * Lives here and not in the updates plugin because the handshake code
 * (iomanager) cannot reach that plugin, while both link this one. Pointers
 * are only compared, never followed, and ~Descriptor drops its own. */
static std::set<Descriptor *> ambienceReset;

void msp_ambience_reset( Descriptor *d )
{
    ambienceReset.insert( d );
}

bool msp_ambience_take_reset( Descriptor *d )
{
    return ambienceReset.erase( d ) > 0;
}
