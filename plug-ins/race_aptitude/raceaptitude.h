/* $Id: raceaptitude.h,v 1.1.2.2 2008/05/27 21:30:05 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __RACEAPTITUDE_H__
#define __RACEAPTITUDE_H__

#include "basicskill.h"
#include "mobskilldata.h"

#include "xmlmap.h"
#include "xmlenumeration.h"
#include "xmlglobalbitvector.h"
#include "skillgroup.h"

class SkillRaceInfo;

class RaceAptitude : public BasicSkill {
XML_OBJECT
public:
    typedef ::Pointer<RaceAptitude> Pointer;
    typedef XMLMapBase<SkillRaceInfo> Races;
    
    RaceAptitude( );

    inline virtual GlobalBitvector & getGroups();
    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;    
    virtual MobSkillData *getMobSkillData();    
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );
    virtual void show( PCharacter *, std::ostream & ) const; 

    // Online editing helpers.
    virtual bool accessFromString(const DLString &newValue, ostringstream &errBuf);
    virtual DLString accessToString() const;

    virtual int getCategory( ) const;
    
protected:
    const SkillRaceInfo *getRaceInfo( CharacterMemoryInterface * ) const;
    
    XML_VARIABLE XMLGlobalBitvector group;
    XML_VARIABLE MobSkillData mob;

    XML_VARIABLE Races races;
};

class SkillRaceInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SkillRaceInfo> Pointer;

    XML_VARIABLE XMLInteger level;
};

inline GlobalBitvector & RaceAptitude::getGroups() 
{
    return group;
}

#endif
