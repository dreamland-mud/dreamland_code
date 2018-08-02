/* $Id$
 *
 * ruffina, 2004
 */
#include "affecthandler.h"

AffectHandler::~AffectHandler( )
{
}

void AffectHandler::remove( Character *ch )
{
}

void AffectHandler::remove( Object *obj )
{
}

void AffectHandler::remove( Room * room )
{
}

void AffectHandler::update( Character * ch, Affect * )
{
}

void AffectHandler::update( Object *obj, Affect * )
{
}

void AffectHandler::update( Room *room, Affect * )
{
}

void AffectHandler::entry( Character *, Affect * )
{
}

void AffectHandler::entry( Room *room, Character *, Affect * )
{
}

void AffectHandler::leave( Room *room, Character *, Affect * )
{
}

void AffectHandler::look( Character *, Character *, Affect * )
{
}

bool AffectHandler::smell( Character *, Character *, Affect * )
{
    return false;
}

void AffectHandler::dispel( Character *ch )
{
}

void AffectHandler::toStream( ostringstream &, Affect * ) 
{
}

void AffectHandler::saves( Character *, Character *, int &, int, Affect * )
{
}

void AffectHandler::stopfol( Character *, Affect * )
{
}

bool AffectHandler::isDispelled( ) const
{
    return false;
}

bool AffectHandler::isCancelled( ) const
{
    return false;
}

