/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultliquid.h"
#include "desire.h"
#include "liquidflags.h"

#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * DefaultLiquid
 *------------------------------------------------------------------*/
DefaultLiquid::DefaultLiquid( )
                : desires( desireManager ),
                  flags( 0, &liquid_flags )
{
}


void DefaultLiquid::loaded( )
{
    liquidManager->registrate( Pointer( this ) );
}

void DefaultLiquid::unloaded( )
{
    liquidManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultLiquid::getShortDescr( lang_t lang ) const
{
    return shortDescr.getForLang( lang );
}
const DLString & DefaultLiquid::getColor( lang_t lang ) const
{
    return color.getForLang( lang );
}
int DefaultLiquid::getSipSize( ) const
{
    return sipSize.getValue( );
}

GlobalArray & DefaultLiquid::getDesires( )
{
    return desires;
}

const Flags & DefaultLiquid::getFlags( ) const
{
    return flags;
}

