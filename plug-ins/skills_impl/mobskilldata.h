/* $Id: mobskilldata.h,v 1.1.2.4.18.4 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __MOBSKILLDATA_H__
#define __MOBSKILLDATA_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlflags.h"
#include "skillsflags.h"

class NPCharacter;
class Skill;

class MobSkillData : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<MobSkillData> Pointer;

    MobSkillData( );
    virtual ~MobSkillData( );
    
    int visible( NPCharacter *, const Skill * ) const;
    int getLearned( NPCharacter *, const Skill * ) const;

    XML_VARIABLE XMLBoolean ordered;
    XML_VARIABLE XMLBoolean forbidden;
    XML_VARIABLE XMLFlagsNoEmpty offense;
    XML_VARIABLE XMLIntegerNoEmpty dice;
    XML_VARIABLE XMLIntegerNoEmpty bonus;
};

class MobProfSkillData : public MobSkillData {
XML_OBJECT
public:
    typedef ::Pointer<MobSkillData> Pointer;

    MobProfSkillData( );
    
    int visible( NPCharacter *, const Skill * ) const;

    XML_VARIABLE XMLBoolean professional;
};

#endif
