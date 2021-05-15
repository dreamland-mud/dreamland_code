/* $Id: clanorgskill.cpp,v 1.1.2.1.6.1 2007/06/26 07:10:57 rufina Exp $
 *
 * ruffina, 2005
 */
#include "clanorgskill.h"
#include "clanorg.h"

#include "commonattributes.h"
#include "pcharacter.h"

ClanOrgSkill::ClanOrgSkill( )
{
}

bool ClanOrgSkill::visible( CharacterMemoryInterface * ch ) const
{
    if (!ClanSkill::visible( ch ))
        return false;

    if (!ch->getPCM())
        return false;
    
    return (getOrgInfo( ch->getPCM( ) ) != NULL);
}

bool ClanOrgSkill::available( Character * ch ) const
{
    const SkillOrgInfo *org;
    
    if (!ClanSkill::available( ch ))
        return false;
        
    if (ch->is_npc( ))
        return false;

    org = getOrgInfo( ch->getPC( ) );
    
    if (!org)
        return false;

    return ch->getPC( )->getClanLevel( ) >= org->clanLevel.getValue( );
}

const SkillOrgInfo * 
ClanOrgSkill::getOrgInfo( PCMemoryInterface *pch ) const
{
    XMLStringAttribute::Pointer attr;
    Organizations::const_iterator o;
    XMLAttributes &attributes = pch->getAttributes( );

    for (o = organizations.begin( ); o != organizations.end( ); o++) 
        if (attributes.isAvailable( o->first )) {
            attr = attributes.findAttr<XMLStringAttribute>( o->first );
            
            if (attr && attr->getValue( ) == o->second.name.getValue( ))
                return &o->second;
        }

    return NULL;
}

/*--------------------------------------------------------------------------
 * OLC helpers
 *--------------------------------------------------------------------------*/

// Parse a string of clan and orden restrictions, separated by a semicolon.
bool ClanOrgSkill::accessFromString(const DLString &newValue, ostringstream &errBuf)
{
    StringList values;
    values.split(newValue, ";");
    DLString clanValues = values.front().stripWhiteSpace();
    DLString orgValues = values.size() > 1 ? values.back().stripWhiteSpace() : DLString::emptyString;

    ClanSkill::accessFromString(clanValues, errBuf);
    organizations.clear();

    if (!orgValues.empty()) {
        for (auto &c: clans) {
            const ClanOrgs *orgs = clanManager->find(c.first)->getOrgs();
            for (auto &o: *orgs) {
                if (o.first == orgValues) {
                    organizations[ClanOrgs::ATTR_NAME].name = o.first;
                    break;
                }
            }
        }
    }

    errBuf.str("");
    errBuf << "Новые клановые и организационные ограничения: " << accessToString() << endl;
    return true;
}

// Returns a string combining clan and orden information, for example: shalafi 10; occultism
DLString ClanOrgSkill::accessToString() const
{
    StringList orgAccess;

    for (auto &o: organizations)
        orgAccess.push_back(o.second.name);

    return ClanSkill::accessToString() +  ";" + orgAccess.join(", ");
}
