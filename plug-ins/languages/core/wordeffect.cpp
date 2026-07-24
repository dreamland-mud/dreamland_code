/* $Id$
 *
 * ruffina, 2009
 */
#include "wordeffect.h"
#include "word.h"
#include "language.h"
#include "languagemanager.h"

#include "merc.h"

#include "def.h"

WordEffect::WordEffect( )
               : object( false ), offensive(false)
{
}

int WordEffect::getFrequency( ) const
{
    return frequency.getValue( );
}

DLString WordEffect::getMeaning( lang_t lang ) const
{
    return meaning.getForLang( lang );
}

bool WordEffect::isGlobal( ) const
{
    return global.getValue( );
}

bool WordEffect::isObject( ) const
{
    return object.getValue( );
}

bool WordEffect::isOffensive( ) const
{
    return offensive.getValue( );
}

bool WordEffect::run( PCharacter *, Character * ) const
{
    return false;
}

bool WordEffect::run( PCharacter *, Object * ) const
{
    return false;
}

