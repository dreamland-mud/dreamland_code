/* $Id$
 *
 * ruffina, 2004
 */
#include "druidic.h"
#include "druidic_effects.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "class.h"
#include "pcharacter.h"

#include "mercdb.h"
#include "def.h"

const DLString DruidicLanguage::LANG_NAME = "druidic";

DruidicLanguage::DruidicLanguage( ) : Language( LANG_NAME )
{
}

void DruidicLanguage::initialization( )
{
    Class::regMoc<DruidicLanguage>( );
#if 0    
    Class::regMoc<AnimalSpiritWE>( );
#endif
    Language::initialization( );
}

void DruidicLanguage::destruction( )
{
    Language::destruction( );
#if 0    
    Class::unregMoc<AnimalSpiritWE>( );
#endif    
    Class::unregMoc<DruidicLanguage>( );
}

/*
 * TODO: create list of typical endings (-each, -uig,-og,-ain,-aig,-in)
 * and prefixes. Implement lenition and ellipses 
 */
DLString DruidicLanguage::createDictum( ) const
{
    int n;
    DLString dictum;
    DLString pre, end, root;
    
    if (endings.empty( ) && prefixes.empty( ) && roots.empty( ))
	throw LanguageException( *this, "totally empty" );
    
    if (!roots.empty( )) {
	n = number_range( 0, roots.size( ) - 1 );
	root = roots[n].getValue( );
    } 
    
    if (!prefixes.empty( ) && chance( 70 )) {
	n = number_range( 0, prefixes.size( ) - 1 );
	pre = prefixes[n].getValue( );
    }

    if (!endings.empty( )) {
	n = number_range( 0, endings.size( ) - 1 );
	end = endings[n].getValue( );
    }
    
    dictum = pre + root + end;
    return dictum;
}

void DruidicLanguage::dream( const Word &word, PCharacter *ch ) const
{
    ch->printf( "В шорохе листьев древних дубов тебе слышится слово {c%s.{x\r\n",
                word.toStr( ) ); 
}

