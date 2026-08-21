/* $Id$
 *
 * ruffina, 2009
 */
#include "wordeffect.h"
#include "word.h"
#include "language.h"
#include "languagemanager.h"

#include "fenia/exceptions.h"
#include "merc.h"

#include "def.h"

// Low-nibble wrapper-id tags: room=1 obj=2 mob=3 area=4 spell=5 ... behavior=10
// autoquest=11. Word-effects take 12. See wrappermanager.h and getID() below.
#define WORDEFFECT_ID_TAG 12

// Stable, stdlib-independent 64-bit FNV-1a. Used so a word-effect's wrapper id
// stays constant across reboots and rebuilds (unlike std::hash, which may
// change with the compiler), keeping persisted Fenia handlers bound.
static unsigned long long word_effect_hash( const DLString &s )
{
    unsigned long long h = 1469598103934665603ULL;
    for (size_t i = 0; i < s.size( ); i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

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

void WordEffect::setEffectIdentity( const DLString &language, const DLString &name )
{
    languageName = language;
    effectName = name;
}

const DLString & WordEffect::getEffectName( ) const
{
    return effectName;
}

const DLString & WordEffect::getLanguageName( ) const
{
    return languageName;
}

long long WordEffect::getID( ) const
{
    if (effectName.empty( ))
        throw Scripting::Exception( "WordEffect ID requested before identity was stamped" );

    // 56-bit hash keeps the shifted value positive and leaves the low nibble
    // for the type tag. Collisions across all effects are asserted against at
    // boot in WrappersPlugin, so a clash fails loudly instead of misbinding.
    unsigned long long h = word_effect_hash( languageName + ":" + effectName ) & 0x00FFFFFFFFFFFFFFULL;
    return (long long)((h << 4) | WORDEFFECT_ID_TAG);
}

