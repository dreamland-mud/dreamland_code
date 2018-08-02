/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __KHUZDUL_H__
#define __KHUZDUL_H__

#include "language.h"

class KhuzdulLanguage : public Language {
XML_OBJECT
public:
    typedef ::Pointer<KhuzdulLanguage> Pointer;
    
    KhuzdulLanguage( );

    virtual void initialization( );
    virtual void destruction( );

protected:
    virtual DLString createDictum( ) const;
    virtual void dream( const Word &, PCharacter * ) const;

    XML_VARIABLE WordList radicals;
    XML_VARIABLE WordList patterns;

private:
    DLString radicalToDictum( ) const;

    static const DLString LANG_NAME;
};


#endif
