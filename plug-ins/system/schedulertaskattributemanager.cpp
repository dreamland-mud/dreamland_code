/* $Id: schedulertaskattributemanager.cpp,v 1.5.2.8.18.1 2007/06/26 07:21:39 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
                          schedulertaskattributemanager.cpp  -  description
                             -------------------
    begin                : Mon Oct 22 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <list>

#include "scheduledxmlattribute.h"
#include "schedulertaskattributemanager.h"
#include "xmlattributes.h"
#include "xmlattribute.h"
#include "logstream.h"
#include "pcharacter.h"
#include "dlscheduler.h"
#include "class.h"

/*
 * SchedulerTaskAttributeManager
 */
SchedulerTaskAttributeManager* SchedulerTaskAttributeManager::thisClass = 0;

const DLString SchedulerTaskAttributeManager::PLUGIN_NAME = "SchedulerTaskAttributeManager";

SchedulerTaskAttributeManager::SchedulerTaskAttributeManager( )
{
    thisClass = this;
    Class::regClass( PLUGIN_NAME, 
                    Class::RegisterOneAllocateClass<SchedulerTaskAttributeManager>::Pointer( NEW ) );
}

SchedulerTaskAttributeManager::~SchedulerTaskAttributeManager( )
{
    Class::unRegClass( PLUGIN_NAME );
    thisClass = 0;
}

void SchedulerTaskAttributeManager::run( PCharacter* ch )
{
    typedef std::list<DLString> KeyList;
    KeyList list;
    XMLAttributes::iterator i;
    XMLAttributes &attributes = ch->getAttributes( );
    
    for (i = attributes.begin( ); i != attributes.end( ); i++) {
        ScheduledXMLAttribute::Pointer attr;
        
        attr = i->second.getDynamicPointer<ScheduledXMLAttribute>( );

        if (attr && attr->pull( ch ))
            list.push_back( i->first );
    }
    
    for (KeyList::iterator j = list.begin( ); j != list.end( ); j++) 
        attributes.eraseAttribute( *j );
}

void SchedulerTaskAttributeManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 60, SchedulerTaskAttributeManager::Pointer( this ) );
}

DLObject::Pointer 
SchedulerTaskAttributeManager::set( DLObject::Pointer arg1, DLObject::Pointer arg2 )
{
    return DLObject::Pointer( );
}

/*
 * ScheduledPCMemoryAttributeManager
 */
ScheduledPCMemoryAttributeManager* ScheduledPCMemoryAttributeManager::thisClass = 0;

const DLString ScheduledPCMemoryAttributeManager::PLUGIN_NAME = "ScheduledPCMemoryAttributeManager";

ScheduledPCMemoryAttributeManager::ScheduledPCMemoryAttributeManager( )
{
    thisClass = this;
    Class::regClass( PLUGIN_NAME, 
                    Class::RegisterOneAllocateClass<ScheduledPCMemoryAttributeManager>::Pointer( NEW ) );
}

ScheduledPCMemoryAttributeManager::~ScheduledPCMemoryAttributeManager( )
{
    Class::unRegClass( PLUGIN_NAME );
    thisClass = 0;
}

void ScheduledPCMemoryAttributeManager::run( PCMemoryInterface *pcm )
{
    typedef std::list<DLString> KeyList;
    KeyList list;
    XMLAttributes::iterator i;
    XMLAttributes &attributes = pcm->getAttributes( );
    
    for (i = attributes.begin( ); i != attributes.end( ); i++) {
        ScheduledPCMemoryXMLAttribute::Pointer attr;
        
        attr = i->second.getDynamicPointer<ScheduledPCMemoryXMLAttribute>( );
        
        if (attr && attr->pull( pcm )) 
            list.push_back( i->first );
    }
    
    for (KeyList::iterator j = list.begin( ); j != list.end( ); j++) 
        attributes.eraseAttribute( *j );
}


void ScheduledPCMemoryAttributeManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 60, ScheduledPCMemoryAttributeManager::Pointer( this ) );
}

DLObject::Pointer 
ScheduledPCMemoryAttributeManager::set( DLObject::Pointer arg1, DLObject::Pointer arg2 )
{
    return DLObject::Pointer( );
}
