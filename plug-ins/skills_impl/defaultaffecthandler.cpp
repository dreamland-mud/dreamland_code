/* $Id: defaultaffecthandler.cpp,v 1.1.2.6.10.7 2008/07/26 19:05:18 rufina Exp $
 *
 * ruffina, 2004
 */
#include "defaultaffecthandler.h"
#include "fenia/exceptions.h"
#include "skill.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "merc.h"
#include "act.h"
#include "def.h"

DefaultAffectHandler::DefaultAffectHandler( )
               : dispelled( false ), cancelled( false )
{
}

long long DefaultAffectHandler::getID() const
{
    int myId = 0;

    if (getSkill()->getSkillHelp())
        myId = getSkill()->getSkillHelp()->getID();

    if (myId <= 0)
        throw Scripting::Exception(getSkill()->getName() + ": affect handler ID not found or zero");

    return (myId << 4) | 6;
}

void DefaultAffectHandler::setSkill( SkillPointer skill )
{
    this->skill = skill;
}

void DefaultAffectHandler::unsetSkill( )
{
    skill.clear( );
}

void DefaultAffectHandler::remove( Character *ch )
{
    if (!removeCharSelf.empty( ))
        ch->pecho( POS_DEAD, removeCharSelf.c_str( ), ch );
        
    if (!removeCharOthers.empty( ))
        ch->recho( removeCharOthers.c_str( ), ch );
}

void DefaultAffectHandler::remove( Object *obj )
{
    if (removeObj.empty( ))
        return;

    if (obj->carried_by != 0) 
        obj->carried_by->pecho( removeObj.c_str( ), obj );

    if (obj->in_room != 0)
        obj->in_room->echo( POS_RESTING, removeObj.c_str( ), obj );
}

void DefaultAffectHandler::remove( Room * room )
{
    if (!removeRoom.empty( ))
        room->echo( POS_RESTING, removeRoom.c_str( ) );
}

void DefaultAffectHandler::dispel( Character *ch )
{

}

SkillPointer DefaultAffectHandler::getSkill( ) const
{
    return skill;
}
bool DefaultAffectHandler::isDispelled( ) const
{
    return dispelled.getValue( );
}
bool DefaultAffectHandler::isCancelled( ) const
{
    return cancelled.getValue( );
}

