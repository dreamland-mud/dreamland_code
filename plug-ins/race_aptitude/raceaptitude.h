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
#include "skillgroup.h"

class SkillRaceInfo;

class RaceAptitude : public BasicSkill {
XML_OBJECT
friend class RaceAptitudeLoader;
public:
    typedef ::Pointer<RaceAptitude> Pointer;
    typedef XMLMapBase<SkillRaceInfo> Races;
    
    RaceAptitude( );

    inline virtual SkillGroupReference & getGroup( );

    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );

    virtual void show( PCharacter *, std::ostream & ); 

    virtual const DLString & getCategory( ) const
    {
        return CATEGORY;
    }
    
protected:
    static const DLString CATEGORY;                                             

    const SkillRaceInfo *getRaceInfo( Character * ) const;
    
    XML_VARIABLE XMLSkillGroupReference group;
    XML_VARIABLE MobSkillData mob;

    XML_VARIABLE Races races;
};

class SkillRaceInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SkillRaceInfo> Pointer;

    XML_VARIABLE XMLInteger level;
};

inline SkillGroupReference & RaceAptitude::getGroup( ) 
{
    return group;
}

#endif
