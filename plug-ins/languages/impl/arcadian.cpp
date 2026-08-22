/* $Id$
 *
 * ruffina, 2009
 */
#include "arcadian.h"
#include "arcadian_effects.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "class.h"
#include "pcharacter.h"

#include "merc.h"
#include "def.h"
#include "l10n.h"

const DLString ArcadianLanguage::LANG_NAME = "arcadian";

ArcadianLanguage::ArcadianLanguage( ) : Language( LANG_NAME )
{
}

void ArcadianLanguage::initialization( )
{
    // The concrete arcadian effects were ported to Fenia; every arcadian.xml
    // effect node now instantiates the single FeniaWordEffectWE stub.
    Class::regMoc<FeniaWordEffectWE>( );
    Class::regMoc<ArcadianLanguage>( );
    Language::initialization( );
}

void ArcadianLanguage::destruction( )
{
    Language::destruction( );
    Class::unregMoc<ArcadianLanguage>( );
    Class::unregMoc<FeniaWordEffectWE>( );
}

DLString ArcadianLanguage::createDictum( ) const
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

void ArcadianLanguage::dream( const Word &word, PCharacter *ch ) const
{
    ch->pecho( _("В веселом гаме и цокоте копыт ты различаешь слово {c%s{x."),
                word.toStr( ) ); 
}

