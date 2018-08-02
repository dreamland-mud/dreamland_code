/* $Id$
 *
 * ruffina, 2009
 */
#include "khuzdul.h"
#include "khuzdul_effects.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "class.h"
#include "pcharacter.h"

#include "mercdb.h"
#include "def.h"

const DLString KhuzdulLanguage::LANG_NAME = "khuzdul";

KhuzdulLanguage::KhuzdulLanguage( ) : Language( LANG_NAME )
{
}

void KhuzdulLanguage::initialization( )
{
    Class::regMoc<FireproofWE>( );
    Class::regMoc<EnchantWeaponWE>( );
    Class::regMoc<BerserkWE>( );
    Class::regMoc<MendingWE>( );
    Class::regMoc<KhuzdulLanguage>( );
    Language::initialization( );
}

void KhuzdulLanguage::destruction( )
{
    Language::destruction( );
    Class::unregMoc<KhuzdulLanguage>( );
    Class::unregMoc<FireproofWE>( );
    Class::unregMoc<EnchantWeaponWE>( );
    Class::unregMoc<BerserkWE>( );
    Class::unregMoc<MendingWE>( );
}


DLString KhuzdulLanguage::createDictum( ) const
{
    DLString dictum;
    
    if (radicals.empty( ))
	throw LanguageException( *this, "empty radicals" );

    if (patterns.empty( ))
	throw LanguageException( *this, "empty patterns" );

    dictum = radicalToDictum( );

    if (dictum.size( ) < 6 || chance( 50 ))
	dictum += "-" + radicalToDictum( );

    return dictum;
}

DLString KhuzdulLanguage::radicalToDictum( ) const
{    
    DLString dictum, radical, pattern;
    DLString::size_type i;
    
    radical = radicals[ number_range( 0, radicals.size( ) - 1 ) ].getValue( );
    pattern = patterns[ number_range( 0, patterns.size( ) - 1 ) ].getValue( );

    for (i = 0; i < pattern.size( ); i++) {
	char c = pattern.at( i );
	
	if (c >= '1' && c <= '9')
	    try {
		c = radical.at( c - '1' );
	    } catch (const std::exception &) {
		continue;
	    }
	
	if (isupper( c )) {
	    dictum.append( (char)tolower( c ) );
	    dictum.append( 'h' );
	}
	else
	    dictum.append( c );
    }
    
    return dictum;
}


void KhuzdulLanguage::dream( const Word &word, PCharacter *ch ) const
{
    ch->printf( "Отзвуком древних битв приходит в твои сны слово {c%s{x.\r\n", word.toStr( ) ); 
}

