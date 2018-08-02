/* $Id: xmlattributelanguage.cpp,v 1.1.2.2 2007/06/26 07:16:23 rufina Exp $
 *
 * ruffina, 2005
 */

#include "xmlattributelanguage.h"

#include "pcharacter.h"

XMLAttributeLanguage::XMLAttributeLanguage( ) 
{
}

void XMLAttributeLanguage::wordUsed( const Word &word, PCharacter *ch )
{
    Words::iterator w = words.find( word.dictum );
    
    if (--w->second.count > 0)
	return;

    ch->pecho( "{wСлово {w%s{w ускользает от тебя.{x", 
               w->second.dictum.getValue( ).c_str( ) );
    
    words.erase( w );
    
    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );
    if (attrHints)
        attrHints->hints.erase( word.dictum );
}   

XMLAttributeLanguageHints::~XMLAttributeLanguageHints( )
{
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

