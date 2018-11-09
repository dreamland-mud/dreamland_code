/* $Id: xmlattributelanguage.h,v 1.1.2.1 2007/05/02 02:52:33 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef XMLATTRIBUTELANGUAGE_H
#define XMLATTRIBUTELANGUAGE_H

#include "xmllong.h"
#include "xmlmap.h"
#include "xmlboolean.h"
#include "xmlattribute.h"
#include "word.h"

/*
 * Contains all dreamed words for this character. Content of this
 * attribute is displayed by '<language> list'.
 */
class XMLAttributeLanguage : public XMLAttribute, public WordContainer 
{
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeLanguage> Pointer;

        XMLAttributeLanguage( );
    
        virtual void wordUsed( const Word &, PCharacter * );
        void removeWord( const Word &, PCharacter * );

        XML_VARIABLE XMLLong lastDreamTime;
        XML_VARIABLE XMLLong sleepTime;
};

/*
 * Contains extra info (such as word dictums and whether their sense is visible)
 * for dreamed words and questor rewards. Content of this attribute is displayed
 * by '<language> list'.
 */
class XMLAttributeLanguageHints : public XMLAttribute, public XMLVariableContainer 
{
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeLanguageHints> Pointer;
        typedef XMLMapBase<XMLBoolean> Hints;

        virtual ~XMLAttributeLanguageHints( );
        
        void addWord( const Word &, bool );
        bool hasHint( const Word & ) const;
        bool hasWord( const DLString & ) const;
        void removeWord(const DLString &);
        void removeWord(const Word &);

        XML_VARIABLE Hints hints;
};

#endif

