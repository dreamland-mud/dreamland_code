/* $Id: clanorgskill.cpp,v 1.1.2.1.6.1 2007/06/26 07:10:57 rufina Exp $
 *
 * ruffina, 2005
 */
#include "clanorgskill.h"

#include "commonattributes.h"
#include "pcharacter.h"

ClanOrgSkill::ClanOrgSkill( )
{
}

bool ClanOrgSkill::visible( Character * ch ) const
{
    if (!ClanSkill::visible( ch ))
        return false;

    if (ch->is_npc( ))
        return false;
    
    return (getOrgInfo( ch->getPC( ) ) != NULL);
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
ClanOrgSkill::getOrgInfo( PCharacter *pch ) const
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

