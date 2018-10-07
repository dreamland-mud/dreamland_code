/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DRUIDIC_H__
#define __DRUIDIC_H__

#include "language.h"

class DruidicLanguage : public Language {
XML_OBJECT
public:
    typedef ::Pointer<DruidicLanguage> Pointer;
    
    DruidicLanguage( );

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
