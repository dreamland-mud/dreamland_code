/* $Id: word.h,v 1.1.2.1 2007/05/02 02:52:33 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __LANGUAGE_WORD_H__
#define __LANGUAGE_WORD_H__

#include "xmlmap.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"

class PCharacter;
class Character;
class Language;
class WordEffect;

typedef ::Pointer<Language> LanguagePointer;

class Word : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<Word> Pointer;
    
    Word( );

    ::Pointer<WordEffect> getEffect( ) const;
    int getPower( ) const;
    bool empty( ) const;
    const char * toStr( ) const;

    XML_VARIABLE XMLString lang;
    XML_VARIABLE XMLString dictum;
    XML_VARIABLE XMLString effect;
    XML_VARIABLE XMLInteger count;
};

class WordContainer : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<WordContainer> Pointer;
    typedef XMLMapBase<Word> Words;

    bool findWord( Word &, LanguagePointer, const DLString & ) const;
    bool findWord( Word &, const DLString & ) const;
    bool addWord( const Word & );
    void eraseWords( LanguagePointer );
    int getPower( LanguagePointer ) const;

    virtual void wordUsed( const Word &, PCharacter * ) = 0;

    inline Words & getWords( )
    {
        return words;
    }

protected:
    XML_VARIABLE Words words;
};

#endif
