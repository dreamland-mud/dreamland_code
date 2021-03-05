/* $Id: genericskill.h,v 1.1.2.10.6.9 2010-09-05 13:57:10 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __GENERICSKILL_H__
#define __GENERICSKILL_H__

#include "basicskill.h"
#include "mobskilldata.h"

#include "xmlvector.h"
#include "xmlboolean.h"
#include "xmlmap.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlglobalbitvector.h"

#include "skillgroup.h"
#include "pcskilldata.h"

using namespace std;

class PCharacter;
class NPCharacter;
class SkillClassInfo;
class SkillRaceBonus;
class SkillClanAntiBonus;
class XMLSkillParents;

class GenericSkill : public BasicSkill 
{
XML_OBJECT
friend class XMLSkillParents;
public:
    typedef ::Pointer<GenericSkill> Pointer;
    typedef XMLMapBase<SkillClassInfo> Classes;
    typedef XMLMapBase<SkillRaceBonus> RaceBonuses;
    
    GenericSkill( );
    virtual ~GenericSkill( );

    virtual void loaded( );
    virtual GlobalBitvector & getGroups();
    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    virtual int getRating( PCharacter * ) const;
    virtual int getMaximum( Character * ) const;

    virtual bool canPractice( PCharacter *, ostream & buf ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );

    virtual void show( PCharacter *, ostream & buf ) const;

    virtual const DLString & getCategory( ) const
    {
        return CATEGORY;
    }

    bool isProfessional() const;
    bool checkAlignEthos(Character *) const;
    inline const Classes & getClasses() const { return classes; }

    virtual bool accessFromString(const DLString &newValue, ostringstream &errBuf);
    virtual DLString accessToString() const;

protected:
    static const DLString CATEGORY;                                             
    
    bool availableForAll( ) const;
    const SkillClassInfo * getClassInfo( CharacterMemoryInterface * ) const;
    const SkillRaceBonus *getRaceBonus( CharacterMemoryInterface * ) const;
    bool isRaceAffect( CharacterMemoryInterface * ) const;

    XML_VARIABLE XMLGlobalBitvector group;
    XML_VARIABLE XMLFlagsNoEmpty   raceAffect;
    XML_VARIABLE MobProfSkillData mob;

    XML_VARIABLE RaceBonuses raceBonuses;
    XML_VARIABLE Classes classes;

    XML_VARIABLE XMLBooleanNoFalse hidden;
};


class SkillClassInfo : public XMLVariableContainer {
XML_OBJECT
friend class GenericSkill;
public:
    typedef ::Pointer<SkillClassInfo> Pointer;
    typedef XMLMapBase<SkillClanAntiBonus> ClanAntiBonuses;
    
    SkillClassInfo( );

    const SkillClanAntiBonus *getClanAntiBonus( CharacterMemoryInterface * ) const;

    inline bool isAlwaysAvailable( ) const {
        return always;
    }
    inline int getLevel( ) const {
        return level.getValue( );
    }
    inline int getRating( ) const {
        return rating.getValue( );
    }
    inline int getMaximum( ) const {
        return maximum.getValue( );
    }
    bool visible( ) const;
     
protected:
    XML_VARIABLE XMLInteger level;
    XML_VARIABLE XMLInteger rating;
    XML_VARIABLE XMLInteger maximum;
    XML_VARIABLE XMLIntegerNoEmpty weight;
    XML_VARIABLE XMLBooleanNoFalse always;
    XML_VARIABLE ClanAntiBonuses clanAntiBonuses;
};

class SkillRaceBonus : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SkillRaceBonus> Pointer;
    
    inline int getLevel( ) const {
        return level.getValue( );
    }
    inline int getBonus( ) const {
        return bonus.getValue( );
    }
    inline bool isProfessional( ) const {
        return professional.getValue( );
    }
    bool visible( ) const;
protected:
    XML_VARIABLE XMLInteger level;
    XML_VARIABLE XMLIntegerNoEmpty bonus;
    XML_VARIABLE XMLBoolean professional;
};

class SkillClanAntiBonus : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SkillClanAntiBonus> Pointer;
    
    inline const DLString & getClanName( ) const {
        return clanName.getValue( );
    }
protected:
    XML_VARIABLE XMLString clanName;
};

#endif
