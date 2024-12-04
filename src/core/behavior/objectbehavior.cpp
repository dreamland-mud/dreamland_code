/* $Id: objectbehavior.cpp,v 1.1.2.7.6.3 2007/09/11 00:00:29 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "objectbehavior.h"
#include "logstream.h"
#include "character.h"
#include "object.h"
#include "def.h"

template class XMLStub<ObjectBehavior>;

const DLString ObjectBehavior::NODE_NAME = "behavior";

ObjectBehavior::ObjectBehavior( ) : obj( NULL )
{
}

ObjectBehavior::~ObjectBehavior( ) 
{
}

void ObjectBehavior::setObj( Object *o ) 
{
    obj = o;
}

void ObjectBehavior::unsetObj( ) 
{
}
    
Object * ObjectBehavior::getObj( ) 
{
    return obj;
}

void ObjectBehavior::wear( Character *victim ) { 
}

void ObjectBehavior::equip( Character *victim ) { 
}

void ObjectBehavior::remove( Character *victim ) { 
}

void ObjectBehavior::get( Character *victim ) { 
}

bool ObjectBehavior::fetch( Character *victim, Object *item )
{
    return false;
}

bool ObjectBehavior::drop( Character *victim ) { 
    return false; 
}

bool ObjectBehavior::sac( Character *victim ) { 
    return false; 
}

void ObjectBehavior::entry( ) { 
}

void ObjectBehavior::give( Character *from, Character *to ) { 
}

void ObjectBehavior::greet( Character *victim ) { 
}

void ObjectBehavior::fight( Character *victim ) { 
}

bool ObjectBehavior::death( Character *victim ) { 
    return false; 
}

void ObjectBehavior::speech( Character *victim, const char *speech ) { 
}

void ObjectBehavior::show( Character *victim, ostringstream &buf )
{

}

bool ObjectBehavior::area( ) { 
    return false;
}

bool ObjectBehavior::hourly()
{
    return false;
}

bool ObjectBehavior::extract( bool ) {
    obj = NULL; 
    return false;
}

bool ObjectBehavior::quit( Character *, bool ) {
    return false;
}

bool ObjectBehavior::examine( Character *victim ) { 
    return false; 
}

bool ObjectBehavior::use( Character *user, const char * ) {
    return false; 
}

void ObjectBehavior::delete_( Character *ch ) {
}

bool ObjectBehavior::save( ) {
    return false;
}

bool ObjectBehavior::command( Character *, const DLString &, const DLString & ) {
    return false;
}

bool ObjectBehavior::visible( const Character * ) {
    return true;
}

DLString ObjectBehavior::extraDescription( Character *ch, const DLString &args ) {
    return DLString::emptyString;
}

bool ObjectBehavior::mayFloat( ) {
    return false;
}

bool ObjectBehavior::canSteal( Character * ) {
    return true;
}

bool ObjectBehavior::canConfiscate( ) {
    return true;
}

bool ObjectBehavior::canLock( Character *ch ) {
    return false;
}

bool ObjectBehavior::canEquip( Character * )
{
    return true;
}

bool ObjectBehavior::canDress( Character *, Character * )
{
    return false;
}

bool ObjectBehavior::hasTrigger( const DLString &trigger )
{
    return false;
}    
