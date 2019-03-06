/* $Id: xmlattributelanguage.cpp,v 1.1.2.2 2007/06/26 07:16:23 rufina Exp $
 *
 * ruffina, 2005
 */

#include "xmlattributelanguage.h"

#include "pcharacter.h"

XMLAttributeLanguage::XMLAttributeLanguage( ) 
{
}

void XMLAttributeLanguage::removeWord( const Word &word, PCharacter *ch )
{
    ch->pecho( "{wСлово {w%s{w ускользает от тебя.{x", word.dictum.getValue( ).c_str( ) );
    
    Words::iterator w = words.find( word.dictum );
    words.erase(w);
    
    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );
    if (attrHints)
        attrHints->removeWord( word );
}

void XMLAttributeLanguage::wordUsed( const Word &word, PCharacter *ch )
{
    Words::iterator w = words.find( word.dictum );
    
    if (--w->second.count > 0)
        return;
    
    removeWord(w->second, ch);
}   

XMLAttributeLanguageHints::~XMLAttributeLanguageHints( )
{
}
        
void XMLAttributeLanguageHints::removeWord( const Word &word )
{
    hints.erase(word.dictum);
}

void XMLAttributeLanguageHints::removeWord( const DLString &dictum )
{
    hints.erase(dictum);
}

void XMLAttributeLanguageHints::addWord( const Word &word, bool hint )
{
    if (word.empty( ))
        return;

    hints[word.dictum] = hint;
}    

bool XMLAttributeLanguageHints::hasHint( const Word &word ) const
{
    Hints::const_iterator h = hints.find( word.dictum );
    return h != hints.end( ) && h->second;
}

bool XMLAttributeLanguageHints::hasWord(const DLString &arg) const
{
    return hints.find(arg) != hints.end();
}

