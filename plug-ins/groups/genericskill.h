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
class XMLSkillParents;

class GenericSkill : public BasicSkill 
{
XML_OBJECT
friend class XMLSkillParents;
public:
    typedef ::Pointer<GenericSkill> Pointer;
    typedef XMLMapBase<SkillClassInfo> Classes;
    
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
    virtual MobSkillData *getMobSkillData();

    virtual bool canPractice( PCharacter *, ostream & buf ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );

    virtual void show( PCharacter *, ostream & buf ) const;

    virtual int getCategory( ) const;

    bool isProfessional() const;
    bool checkAlignEthos(Character *) const;
    inline const Classes & getClasses() const { return classes; }

    virtual bool accessFromString(const DLString &newValue, ostringstream &errBuf);
    virtual DLString accessToString() const;
    DLString skillClassesList() const;
    DLString skillRacesList() const;

protected:
    bool availableForAll( ) const;
    const SkillClassInfo * getClassInfo( CharacterMemoryInterface * ) const;
    bool hasRaceBonus( CharacterMemoryInterface * ) const;

    XML_VARIABLE XMLGlobalBitvector group;
    XML_VARIABLE MobProfSkillData mob;

    XML_VARIABLE XMLGlobalBitvector raceBonuses;
    XML_VARIABLE Classes classes;

    XML_VARIABLE XMLBooleanNoFalse hidden;
};


class SkillClassInfo : public XMLVariableContainer {
XML_OBJECT
friend class GenericSkill;
public:
    typedef ::Pointer<SkillClassInfo> Pointer;
    
    SkillClassInfo( );

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
};

#endif
