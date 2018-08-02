/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __ARCADIAN_H__
#define __ARCADIAN_H__

#include "language.h"

class ArcadianLanguage : public Language {
XML_OBJECT
public:
    typedef ::Pointer<ArcadianLanguage> Pointer;
    
    ArcadianLanguage( );

    virtual void initialization( );
    virtual void destruction( );

protected:
    virtual DLString createDictum( ) const;
    virtual void dream( const Word &, PCharacter * ) const;

    XML_VARIABLE WordList prefixes;
    XML_VARIABLE WordList endings;
    XML_VARIABLE WordList roots;

private:
    static const DLString LANG_NAME;
};


#endif
