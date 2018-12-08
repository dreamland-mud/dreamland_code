#ifndef DEFAULT_BONUS_H
#define DEFAULT_BONUS_H

#include "bonus.h"
#include "xmltableelement.h"
#include "xmllist.h"
#include "xmlstring.h"
#include "xmlboolean.h"

class DefaultBonus : public XMLVariableContainer, public Bonus, public XMLTableElement {
XML_OBJECT
public:
    typedef ::Pointer<DefaultBonus> Pointer;
    
    DefaultBonus( );

    virtual void loaded( );
    virtual void unloaded( );
    virtual const DLString & getName( ) const;
    virtual void setName( const DLString & );
    virtual const DLString &getRussianName( ) const;
    virtual const DLString &getShortDescr( ) const;
    virtual char getColor() const;
    virtual bool isValid( ) const;
    virtual bool isReligious() const;
    virtual bool isActive(PCharacter *, const struct time_info_data &) const;
    virtual void reportTime(PCharacter *, ostringstream &) const;

protected:
    bool activeForAll(const struct time_info_data &) const;
    bool activeForPlayer(PCharacter *, const struct time_info_data &) const;
 
    XML_VARIABLE XMLString nameRus;
    XML_VARIABLE XMLListBase<XMLString> aliases;
    XML_VARIABLE XMLString color;
    XML_VARIABLE XMLString legend;
    XML_VARIABLE XMLString shortDescr;
    XML_VARIABLE XMLString msgTodayReligion;
    XML_VARIABLE XMLString msgTodayGlobal;
    XML_VARIABLE XMLString globalDay;
    XML_VARIABLE XMLString globalMonth;
    XML_VARIABLE XMLBoolean religious;
};

class ExperienceBonus : public DefaultBonus {
XML_OBJECT
public:
    typedef ::Pointer<ExperienceBonus> Pointer;

    virtual void reportAction(PCharacter *, ostringstream &) const;
};

#endif
