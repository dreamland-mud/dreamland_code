/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __AHENN_H__
#define __AHENN_H__

#include "language.h"

class AhennLanguage : public Language {
XML_OBJECT
public:
    typedef ::Pointer<AhennLanguage> Pointer;
    
    AhennLanguage( );

    virtual void initialization( );
    virtual void destruction( );

protected:
    virtual DLString createDictum( ) const;
    virtual void dream( const Word &, PCharacter * ) const;

    XML_VARIABLE WordList prefixes;
    XML_VARIABLE WordList suffixes;
    XML_VARIABLE WordList words;
    XML_VARIABLE WordList verb_ends;

private:
    void parseSyllabes( const DLString &, vector<DLString> & ) const;
    DLString mixSyllabes( vector<vector<DLString> > & ) const;

    static const DLString LANG_NAME;
};


#endif
