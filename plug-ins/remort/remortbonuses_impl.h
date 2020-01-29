/* $Id$
 *
 * ruffina, 2004
 */
#ifndef REMORTBONUSES_IMPL_H
#define REMORTBONUSES_IMPL_H

#include "remortbonus.h"

class StatRemortBonus : public IntegerRemortBonus {
XML_OBJECT
public:
    typedef ::Pointer<StatRemortBonus> Pointer;
    StatRemortBonus( );
protected:
    virtual bool matches( const DLString &arg ) const;
    virtual DLString getShortDescr( ) const;
    virtual int bonusMaximum( PCharacter * ) const;
    virtual int & bonusField( PCharacter * ) const;

    XML_VARIABLE XMLEnumeration stat;
};

class LevelRemortBonus : public IntegerRemortBonus {
XML_OBJECT
public:
    typedef ::Pointer<LevelRemortBonus> Pointer;
protected:
    virtual int & bonusField( PCharacter * ) const;
    virtual int bonusMaximum( PCharacter * ) const;
};

class PretitleRemortBonus : public BooleanRemortBonus {
XML_OBJECT
public:
    typedef ::Pointer<PretitleRemortBonus> Pointer;
protected:
    virtual bool & bonusField( PCharacter * ) const;
};

class HealthRemortBonus : public AppliedRemortBonus {
XML_OBJECT
public:
    typedef ::Pointer<HealthRemortBonus> Pointer;
protected:
    virtual void bonusApply( PCharacter * ) const;
    virtual void bonusRemove( PCharacter * ) const;
    virtual int bonusMaximum( PCharacter * ) const;
    virtual int & bonusField( PCharacter * ) const;
};

class ManaRemortBonus : public AppliedRemortBonus {
XML_OBJECT
public:
    typedef ::Pointer<ManaRemortBonus> Pointer;
protected:
    virtual void bonusApply( PCharacter * ) const;
    virtual void bonusRemove( PCharacter * ) const;
    virtual int bonusMaximum( PCharacter * ) const;
    virtual int & bonusField( PCharacter * ) const;
};


#endif
