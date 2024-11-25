#ifndef CRAFTSKILL_H
#define CRAFTSKILL_H

#include "basicskill.h"
#include "craftattribute.h"
#include "skillgroup.h"
#include "xmlmap.h"
#include "xmlflags.h"
#include "xmlglobalbitvector.h"

class CraftProfessionInfo;

class CraftSkill : public BasicSkill {
XML_OBJECT
public:
    typedef ::Pointer<CraftSkill> Pointer;
    typedef XMLMapBase<CraftProfessionInfo> CraftProfessions;
    
    CraftSkill( );

    virtual GlobalBitvector & getGroups();
    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );
    virtual void improve( Character *, bool, Character *victim = NULL, int dam_type = -1, int dam_flags = 0 ) const;

    virtual void show( PCharacter *, std::ostream & ) const; 

    virtual int getCategory( ) const;

protected:

    XML_VARIABLE XMLGlobalBitvector group;
    XML_VARIABLE CraftProfessions subprofessions;
};


class CraftProfessionInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<CraftProfessionInfo> Pointer;

    XML_VARIABLE XMLInteger level;
};

#endif
