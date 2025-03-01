/* $Id: clanskill.h,v 1.1.6.4.6.6 2008/05/27 21:30:02 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __CLANSKILL_H__
#define __CLANSKILL_H__

#include "basicskill.h"
#include "mobskilldata.h"

#include "xmlmap.h"
#include "xmlflags.h"
#include "xmlboolean.h"
#include "xmlglobalbitvector.h"

#include "clanreference.h"
#include "skillgroup.h"

class SkillClanInfo;

class ClanSkill : public BasicSkill {
XML_OBJECT
public:
    typedef ::Pointer<ClanSkill> Pointer;
    typedef XMLMapBase<SkillClanInfo> Clans;
    
    ClanSkill( );

    virtual void loaded( );
    virtual GlobalBitvector & getGroups();
    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    virtual int getMaximum( Character * ) const;
    virtual MobSkillData *getMobSkillData();
    
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );

    virtual void show( PCharacter *, std::ostream & ) const; 

    // Online editing helpers.
    virtual bool accessFromString(const DLString &newValue, ostringstream &errBuf);
    virtual DLString accessToString() const;

    virtual int getCategory( ) const;

protected:
    const SkillClanInfo *getClanInfo( CharacterMemoryInterface * ) const;

    XML_VARIABLE MobSkillData mob;

    XML_VARIABLE Clans clans;
    XML_VARIABLE XMLGlobalBitvector group;
};

class SkillClanInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SkillClanInfo> Pointer;
    
    SkillClanInfo( );

    XML_VARIABLE XMLInteger level;
    XML_VARIABLE XMLInteger maximum;
    XML_VARIABLE XMLInteger rating;
    XML_VARIABLE XMLIntegerNoEmpty clanLevel;
    XML_VARIABLE XMLBoolean needItem;
    XML_VARIABLE XMLBooleanNoFalse needPractice;
    XML_VARIABLE XMLInteger maxLevel;
};

#endif
