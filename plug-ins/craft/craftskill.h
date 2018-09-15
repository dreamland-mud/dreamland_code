#ifndef CRAFTSKILL_H
#define CRAFTSKILL_H

#include "basicskill.h"
#include "craftattribute.h"
#include "skillgroup.h"
#include "xmlmap.h"
#include "xmlflags.h"

class CraftProfessionInfo;

class CraftSkill : public BasicSkill {
XML_OBJECT
friend class CraftSkillLoader;
public:
    typedef ::Pointer<CraftSkill> Pointer;
    typedef XMLMapBase<CraftProfessionInfo> CraftProfessions;
    
    CraftSkill( );

    virtual SkillGroupReference & getGroup( );
    
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    virtual int getWeight( Character * ) const;
    
    virtual bool canForget( PCharacter * ) const;
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );

    virtual void show( PCharacter *, std::ostream & ); 

    virtual const DLString & getCategory( ) const
    {
	return CATEGORY;
    }

protected:
    XMLAttributeCraft::Pointer getProfAttr(Character *ch) const;

    static const DLString CATEGORY;

    XML_VARIABLE XMLInteger weight;    
    XML_VARIABLE XMLSkillGroupReference group;
    XML_VARIABLE CraftProfessions subprofessions;
};


class CraftProfessionInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<CraftProfessionInfo> Pointer;

    XML_VARIABLE XMLInteger level;
};

#endif
