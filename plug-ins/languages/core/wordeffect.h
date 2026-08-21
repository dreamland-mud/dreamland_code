/* $Id: wordeffect.h,v 1.1.2.1 2007/05/02 02:52:33 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __WORDEFFECT_H__
#define __WORDEFFECT_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "fenia/wrappertarget.h"
#include "lang.h"

class Character;
class PCharacter;
class Object;

class WordEffect : public XMLVariableContainer, public WrapperTarget {
XML_OBJECT
public:
    typedef ::Pointer<WordEffect> Pointer;

    WordEffect( );

    virtual bool run( PCharacter *, Character * ) const;
    virtual bool run( PCharacter *, Object * ) const;

    int getFrequency( ) const;
    DLString getMeaning( lang_t lang = LANG_DEFAULT ) const;
    bool isGlobal( ) const;
    bool isObject( ) const;
    bool isOffensive( ) const;

    // Fenia wrapper identity. Word-effects have no persistent numeric id of
    // their own (they are per-language XML nodes keyed only by name), so the
    // language stamps each effect with its name and its language name at boot,
    // and getID() derives a stable, collision-checked id from that pair. Needed
    // for the Fenia wrapper to rebind persisted handlers across reboots.
    void setEffectIdentity( const DLString &language, const DLString &name );
    const DLString &getEffectName( ) const;
    const DLString &getLanguageName( ) const;
    long long getID( ) const;

protected:
    XML_VARIABLE XMLInteger frequency;
    XML_VARIABLE XMLMultiString meaning;
    XML_VARIABLE XMLBoolean global;
    XML_VARIABLE XMLBoolean object;
    XML_VARIABLE XMLBoolean offensive;

    // Runtime identity, not serialized -- stamped from the owning language.
    DLString languageName;
    DLString effectName;
};

#endif
