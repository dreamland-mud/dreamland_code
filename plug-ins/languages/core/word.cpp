/* $Id$
 *
 * ruffina, 2009
 */
#include "logstream.h"
#include "word.h"
#include "wordeffect.h"
#include "language.h"
#include "languagemanager.h"
#include "xmlattributelanguage.h"

#include "pcharacter.h"
#include "def.h"

Word::Word( )
{
}

int Word::getPower( ) const
{
    WordEffect::Pointer ef = getEffect( );

    if (!ef)
        return 0;

    return count.getValue( ) * (5 - ef->getFrequency( ));
}

bool Word::empty( ) const
{
    return dictum.getValue( ).empty( ) || effect.getValue( ).empty( );
}

const char * Word::toStr( ) const
{
    return dictum.getValue( ).c_str( );
}

WordEffect::Pointer Word::getEffect( ) const
{
    WordEffect::Pointer ef;
    Language::Pointer language = languageManager->findLanguage( lang.getValue( ) );
    
    if (!language) {
        LogStream::sendError( ) << "Unknown language " << lang << " for word " << dictum << endl;
        return ef;
    }
    
    ef = language->findEffect( effect.getValue( ) );

    if (!ef) {
        LogStream::sendError( ) << "Unknown effect " << effect << " for word " << dictum << endl;
        return ef;
    }

    return ef;
}

bool WordContainer::addWord( const Word &word )
{
    if (word.empty( ))
        return false;

    words[word.dictum] = word;
    return true;
}

void WordContainer::eraseWords( const Language &lang )
{
    Words::iterator w;
    Words newWords;
    
    for (w = words.begin( ); w != words.end( ); w++)
        if (w->second.lang.getValue( ) != lang.getName( ))
            newWords[w->first] = w->second;
    
    words = newWords;
}

bool WordContainer::findWord( Word &word, const Language &lang, const DLString &arg ) const
{
    Words::const_iterator w = words.find( arg );

    if (w == words.end( ))
        return false;
    
    if (w->second.lang.getValue( ) != lang.getName( ))
        return false;

    word = w->second;
    return true;
}

bool WordContainer::findWord( Word &word, const DLString &arg ) const
{
    Words::const_iterator w = words.find( arg );

    if (w == words.end( ))
        return false;
    
    word = w->second;
    return true;
}

int WordContainer::getPower( const Language &lang ) const
{
    Words::const_iterator w;
    int power = 0;

    for (w = words.begin( ); w != words.end( ); w++)
        if (w->second.lang.getValue( ) == lang.getName( ))
            power += w->second.getPower( );
        
    return power;
}                
