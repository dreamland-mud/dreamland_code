/* $Id$
 *
 * ruffina, 2009
 */
#include "ahenn.h"
#include "ahenn_effects.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "iconvmap.h"
#include "class.h"
#include "pcharacter.h"
#include "dlfilestream.h"

#include "merc.h"
#include "def.h"

static IconvMap utf2koi("utf-8", "koi8-r");

/*---------------------------------------------------------------------------
 * AhennLanguage
 *--------------------------------------------------------------------------*/
const DLString AhennLanguage::LANG_NAME = "ahenn";

AhennLanguage::AhennLanguage( ) : Language( LANG_NAME )
{
}

void AhennLanguage::initialization( )
{
    Class::regMoc<BadSpellWE>( );
    Class::regMoc<InspirationWE>( );
    Class::regMoc<AhennLanguage>( );
    Language::initialization( );
}

void AhennLanguage::destruction( )
{
    Language::destruction( );
    Class::unregMoc<AhennLanguage>( );
    Class::unregMoc<BadSpellWE>( );
    Class::unregMoc<InspirationWE>( );
}


DLString AhennLanguage::createDictum( ) const
{
    vector<DLString> lines;
    
    try {
        DLFileStream vocab(languageManager->getWordsDir(), "ahenn", ".txt");
        lines = vocab.toVector();

        if (lines.empty())
            throw LanguageException( *this, "empty " + vocab.getAbsolutePath());

    } catch (const ExceptionDBIO &ex) {
        throw LanguageException(*this, ex.what());
    }


    DLString dictum = lines[number_range(0, lines.size() - 1)];
    return utf2koi(dictum);
}


void AhennLanguage::dream( const Word &word, PCharacter *ch ) const
{
    ch->printf( "Из звуков мелодии, древней, как сам мир, рождается слово {c%s{x.\r\n",
                word.toStr( ) );
}

