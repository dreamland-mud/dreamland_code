/* $Id: wordeffect.h,v 1.1.2.1 2007/05/02 02:52:33 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __WORDEFFECT_H__
#define __WORDEFFECT_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"

class Character;
class PCharacter;
class Object;

class WordEffect : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<WordEffect> Pointer;
    
    WordEffect( );

    virtual bool run( PCharacter *, Character * ) const;
    virtual bool run( PCharacter *, Object * ) const;

    int getFrequency( ) const;
    DLString getMeaning( ) const;
    bool isGlobal( ) const;
    bool isObject( ) const;
    bool isOffensive( ) const;

protected:
    XML_VARIABLE XMLInteger frequency;
    XML_VARIABLE XMLString  meaning;
    XML_VARIABLE XMLBoolean global;
    XML_VARIABLE XMLBoolean object;
    XML_VARIABLE XMLBoolean offensive;
};

#endif
