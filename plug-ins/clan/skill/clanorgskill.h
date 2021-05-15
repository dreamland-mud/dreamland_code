/* $Id: clanorgskill.h,v 1.1.2.1 2005/08/31 17:53:22 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __CLANORGSKILL_H__
#define __CLANORGSKILL_H__

#include "clanskill.h"

class SkillOrgInfo;

class ClanOrgSkill : public ClanSkill {
XML_OBJECT
public:
    typedef ::Pointer<ClanOrgSkill> Pointer;
    typedef XMLMapBase<SkillOrgInfo> Organizations;
    
    ClanOrgSkill( );

    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    
    // Online editing helpers.
    virtual bool accessFromString(const DLString &newValue, ostringstream &errBuf);
    virtual DLString accessToString() const;

protected:
    const SkillOrgInfo * getOrgInfo( PCMemoryInterface * ) const;

    XML_VARIABLE Organizations organizations;
};

class SkillOrgInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<SkillOrgInfo> Pointer;
    
    XML_VARIABLE XMLInteger clanLevel;
    XML_VARIABLE XMLString name;
};

#endif
