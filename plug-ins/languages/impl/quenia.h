/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __QUENIA_H__
#define __QUENIA_H__

#include "language.h"

class QueniaLanguage : public Language {
XML_OBJECT
public:
    typedef ::Pointer<QueniaLanguage> Pointer;
    
    QueniaLanguage( );

    virtual void initialization( );
    virtual void destruction( );

protected:
    virtual DLString createDictum( ) const;
    virtual void dream( const Word &, PCharacter * ) const;

    XML_VARIABLE WordList prefixes;
    XML_VARIABLE WordList suffixes;
    XML_VARIABLE WordList roots;

private:
    DLString concatenate( DLString &, DLString & ) const;
    void correct( DLString & ) const;

    static const DLString LANG_NAME;
};


#endif
