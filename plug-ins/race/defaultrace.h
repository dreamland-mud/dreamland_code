/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTRACE_H
#define DEFAULTRACE_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "xmlshort.h"
#include "xmlinteger.h"
#include "xmltableelement.h"
#include "xmlenumeration.h"
#include "xmlflags.h"
#include "xmlglobalbitvector.h"
#include "xmltableloaderplugin.h"

#include "helpmanager.h"
#include "markuphelparticle.h"
#include "race.h"

class DefaultRace;

TABLE_LOADER_DECL(RaceLoader);

/*
 * RaceHelp
 */
class RaceHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<RaceHelp> Pointer;

    virtual void setRace( ::Pointer<DefaultRace> race );
    virtual void unsetRace( );
    virtual void save() const;

    virtual DLString getTitle(const DLString &label) const;
    virtual void getRawText( Character *, ostringstream & ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    ::Pointer<DefaultRace> race;
};

inline const DLString & RaceHelp::getType( ) const
{
    return TYPE;
}

/*
 * DefaultRace
 */
class DefaultRace : public virtual Race, 
                    public XMLTableElement,
                    public XMLVariableContainer 
{
XML_OBJECT
public:        
    typedef ::Pointer<DefaultRace> Pointer;

    DefaultRace( );
    
    virtual const DLString & getName( ) const;
    virtual bool isValid( ) const;
    virtual bool matchesStrict( const DLString & ) const;
    virtual bool matchesUnstrict( const DLString & ) const;
    virtual void setName( const DLString & );
    virtual void loaded( );
    virtual void unloaded( );

    virtual const Flags & getDet( ) const;
    virtual const Flags & getAct( ) const;
    virtual const Flags & getAff( ) const;
    virtual const Flags & getOff( ) const;
    virtual const Flags & getImm( ) const;
    virtual const Flags & getRes( ) const;
    virtual const Flags & getVuln( ) const;
    virtual const Flags & getForm( ) const;
    virtual const Flags & getParts( ) const;
    virtual const EnumerationArray & getStats( ) const;
    virtual const GlobalBitvector & getWearloc( ) const;
    virtual const Enumeration & getSize( ) const;
    virtual Flags getAttitude( const Race & ) const;
    virtual const GlobalBitvector &getAffects() const;

    virtual const DLString & getMaleName( ) const;
    virtual const DLString & getNeuterName( ) const;
    virtual const DLString & getFemaleName( ) const;
    virtual const DLString & getMltName( ) const;
    virtual DLString getNameFor( Character *looker, Character *me ) const;

    XML_VARIABLE XMLFlagsNoEmpty         det;
    XML_VARIABLE XMLFlagsNoEmpty         act;
    XML_VARIABLE XMLFlagsNoEmpty         aff;
    XML_VARIABLE XMLFlagsNoEmpty         off;
    XML_VARIABLE XMLFlagsNoEmpty         imm, res, vuln;
    XML_VARIABLE XMLFlagsNoEmpty         form;
    XML_VARIABLE XMLFlagsNoEmpty         parts;
    XML_VARIABLE XMLEnumerationArray     stats; 
    XML_VARIABLE XMLEnumerationNoEmpty   size;
    XML_VARIABLE XMLGlobalBitvector      wearloc;
    XML_VARIABLE XMLGlobalBitvector      affects;

    XML_VARIABLE XMLGlobalBitvector      hunts, donates;

    XML_VARIABLE XMLStringNoEmpty  nameMale; 
    XML_VARIABLE XMLStringNoEmpty  nameFemale; 
    XML_VARIABLE XMLStringNoEmpty  nameNeuter; 
    XML_VARIABLE XMLStringNoEmpty  nameMlt; 

    XML_VARIABLE XMLPointerNoEmpty<RaceHelp> help;
};

#endif
