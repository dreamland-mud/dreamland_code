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

const DLString ArcadianLanguage::LANG_NAME = "arcadian";

ArcadianLanguage::ArcadianLanguage( ) : Language( LANG_NAME )
{
}

void ArcadianLanguage::initialization( )
{
    Class::regMoc<WaterToBeerWE>( );
    Class::regMoc<WaterToWineWE>( );
    Class::regMoc<WineRefreshWE>( );
    Class::regMoc<WineAwakeWE>( );
    Class::regMoc<WineSleepWE>( );
    Class::regMoc<WineCalmWE>( );
    Class::regMoc<BeerArmorWE>( );
    Class::regMoc<BeerElementalWE>( );
    Class::regMoc<ArcadianLanguage>( );
    Language::initialization( );
}

void ArcadianLanguage::destruction( )
{
    Language::destruction( );
    Class::unregMoc<ArcadianLanguage>( );
    Class::unregMoc<WaterToBeerWE>( );
    Class::unregMoc<WaterToWineWE>( );
    Class::unregMoc<WineRefreshWE>( );
    Class::unregMoc<WineAwakeWE>( );
    Class::unregMoc<WineSleepWE>( );
    Class::unregMoc<WineCalmWE>( );
    Class::unregMoc<BeerArmorWE>( );
    Class::unregMoc<BeerElementalWE>( );
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
    ch->printf( "В веселом гаме и цокоте копыт ты различаешь слово {c%s{x.\r\n",
                word.toStr( ) ); 
}

