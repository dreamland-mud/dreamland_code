#ifndef CRAFTSKILL_H
#define CRAFTSKILL_H

#include "basicskill.h"

#include "xmlmap.h"
#include "xmlflags.h"

class SubProfessionInfo;

class CraftSkill : public BasicSkill {
XML_OBJECT
friend class CraftSkillLoader;
public:
    typedef ::Pointer<CraftSkill> Pointer;
    typedef XMLMapBase<SubProfessionInfo> SubProfessions;
    
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
    static const DLString CATEGORY;

    XML_VARIABLE XMLInteger weight;    
    XML_VARIABLE XMLSkillGroupReference group;
    XML_VARIABLE SubProfessions subprofessions;
};


class SubProfessionInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SubProfessionInfo> Pointer;

    XML_VARIABLE XMLInteger level;
};

#endif
