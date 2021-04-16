/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          objectmanager.cpp  -  description
                             -------------------
    begin                : Tue Jun 5 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "class.h"
#include "objectmanager.h"
#include "object.h"

#include "dreamland.h"

ObjectManager* ObjectManager::thisClass = 0;
ObjectManager::ExtractList ObjectManager::extractList;

ObjectManager::ObjectManager( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

ObjectManager::~ObjectManager( )
{
    extractList.clear_delete( );
    thisClass = 0;
}

void ObjectManager::extract( Object* object )
{
    object->extract( );
    extractList.push_back( object );
}

Object* ObjectManager::getObject( )
{
    Object *object;
    
    if( !extractList.empty( ) )
    {
        object = *extractList.begin( );
        extractList.erase( extractList.begin( ) );
    }
    else
    {
        object = dallocate( Object );
    }
    
    object->setID( dreamland->genID( ) );
    return object;
}

Object* ObjectManager::find(long long ID) 
{
    if (ID == 0)
        return 0;
        
    for (Object *obj = object_list; obj; obj = obj->next)
        if (obj->getID() == ID)
            return obj;

    return 0;
}

