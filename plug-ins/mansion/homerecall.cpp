/* $Id: homerecall.cpp,v 1.1.2.15.6.4 2008/07/26 19:08:53 rufina Exp $
 *
 * ruffina, 2004
 */

#include "homerecall.h"

#include "class.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "pcharactermanager.h"

#include "dreamland.h"
#include "recallmovement.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * 'homerecall' command 
 *---------------------------------------------------------------------------*/
COMMAND(HomeRecall, "homerecall")
{
    DLString cmd;
    DLString arguments = constArguments;
    PCharacter *pch = ch->getPC( );

    if (ch->is_npc( )) {
	ch->println( "В лес!" );
	return;
    }
    
    if (arguments.empty( )) {
	doRecall( pch );
	return;
    }
    
    if (!pch->is_immortal( )) {
	pch->println( "Эта команда не требует аргументов." );
	return;
    }
    
    cmd = arguments.getOneArgument( );
   
    if (cmd.strPrefix( "list" ))
	doList( pch );
    else if (arguments.empty( ))
	doUsage( pch );
    else if (cmd.strPrefix( "set" ))
	doSet( pch, arguments );
    else if (cmd.strPrefix( "show" ))
	doShow( pch, arguments );
    else if (cmd.strPrefix( "remove" ))
	doRemove( pch, arguments );
    else
	doUsage( pch ); 
}

class HomeRecallMovement : public RecallMovement {
public:
    HomeRecallMovement( Character *ch )
               : RecallMovement( ch )
    {
    }
    HomeRecallMovement( Character *ch, Character *actor, Room *to_room )
               : RecallMovement( ch )
    {
	this->actor = actor;
	this->to_room = to_room;
    }
    
protected:
    virtual bool findTargetRoom( )
    {
	XMLAttributeHomeRecall::Pointer attr;
	
	if (to_room)
	    return true;

	if (ch->is_npc( )) {
	    msgSelf( ch, "В лес!" );
	    return false;
	}

	attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" );
	if (!attr) {
	    msgSelf( ch, "У тебя нет своего дома." );
	    return false;
	}
	
	if (!( to_room = get_room_index( attr->getPoint( ) ) )) {
	    msgSelf( ch, "Ты заблудил%1Gось|ся|ась." );
	    return false;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (ch != actor)
	    return true;
	else
	    return checkMount( )
		   && checkShadow( )
		   && checkBloody( wch )
		   && checkPumped( )
		   && checkSameRoom( )
		   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
	if (ch != actor)
	    return applyInvis( wch );
	else
	    return applyInvis( wch )
		   && applyMovepoints( )
		   && applyWaitstate( );
    }
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
	if (fLeaving)
	    msgRoomNoParty( wch, 
		            "%1$^C1 растворил%1$Gось|ся|ась в воздухе.",
		            "%1$^C1 и %2$C1 растворяются в воздухе." );
	else
	    msgRoomNoParty( wch, "%1$^C1 появляется рядом с тобой." );
    }
    virtual void msgOnStart( )
    {
	msgRoom( ch, "%1$^C1 просит Богов перенести %1$P2 в родной дом." );
	msgSelf( ch, "Ты просишь Богов перенести тебя в родной дом." );
    }
    virtual void movePet( NPCharacter *pet )
    {
	HomeRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
};

void HomeRecall::doRecall( PCharacter * ch )
{
    HomeRecallMovement( ch ).move( );
}

void HomeRecall::doSet( PCharacter * ch, DLString &arg )
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    
    arg = arg.getOneArgument( );
    try {
	vnum = arg.toInt( );
    } catch (const ExceptionBadType& e) {
	ch->println( "<room vnum> должно быть числом." );
	return;
    }
    
    pci = PCharacterManager::find( name );
    if (!pci) {
	ch->println( "Жертва не найдена." );
	return;
    }
    
    if (get_room_index( vnum ) == 0) {
	ch->println( "Комнаты с таким номером не существует." );
	return;
    }

    pci->getAttributes( ).getAttr<XMLAttributeHomeRecall>( "homerecall" )->setPoint( vnum );
    PCharacterManager::saveMemory( pci );

    ch->println( "Done." );
}

void HomeRecall::doShow( PCharacter * ch, DLString &arg )
{
    XMLAttributeHomeRecall::Pointer attr;
    PCMemoryInterface *pci;
    int point;
    Room *room;
    DLString name = arg.getOneArgument( );
    
    pci = PCharacterManager::find( name );
    if (!pci) {
	ch->println( "Жертва не найдена." );
	return;
    }
    
    attr = pci->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
	ch->printf( "%s бездомное..\r\n", pci->getName( ).c_str( ) );
	return;
    }
    
    point = attr->getPoint( );
    room = get_room_index( point );

    if (!room) {
	ch->printf( "%s имеет homerecall в несуществующую комнату #%d!", 
		    pci->getName( ).c_str( ), point );
	return;
    }
    
    ch->printf( "Homerecall для %s расположен в [%d] %s (%s).\r\n",
		pci->getName( ).c_str( ), room->vnum, room->name, room->area->name );
}

void HomeRecall::doRemove( PCharacter * ch, DLString &arg )
{
    XMLAttributeHomeRecall::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    
    pci = PCharacterManager::find( name );
    if (!pci) {
	ch->println( "Жертва не найдена." );
	return;
    }
    
    attr = pci->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
	ch->printf( "%s бездомное..\r\n", pci->getName( ).c_str( ) );
	return;
    }

    pci->getAttributes( ).eraseAttribute( "homerecall" );
    PCharacterManager::saveMemory( pci );

    ch->println( "Done." );
}

void HomeRecall::doList( PCharacter *ch ) 
{
    char buf[MAX_STRING_LENGTH];
    int point;
    Room * room;
    PCharacterMemoryList::const_iterator i;
    XMLAttributeHomeRecall::Pointer attr;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
   
    ch->println( "Список всех персонажей, имеющих homerecalls: \r\n");
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	attr = i->second->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 

	if (!attr)
	    continue;
	
	point = attr->getPoint( );
	room = get_room_index( point );
	
	sprintf( buf, "%-15s [%-5d] %-25.25s (%s)\r\n", 
		 i->second->getName( ).c_str( ), point, 
		 (room ? room->name : "{Rnull!{x"),
		 (room ? room->area->name : "") );

	ch->send_to( buf );
    }
}

void HomeRecall::doUsage( PCharacter * ch )
{
    std::basic_ostringstream<char> buf;

    buf << "Синтаксис: " << endl
        << "{Whomerecall{x                          - переносит в родной дом" << endl
	<< "{Whomerecall set{x <name> <room vnum>   - установить игроку комнату для homerecall" << endl
	<< "в идеале это комната снаружи дома, от которого он может купить ключ" << endl
	<< "{Whomerecall show{x <name>              - посмотреть чей-то homerecall" << endl
	<< "{Whomerecall remove{x <name>            - отобрать возможность рекаллиться домой" << endl
	<< "{Whomerecall list{x                     - список всех игроков, имеющих homerecall" << endl;
    
    ch->send_to( buf );
}

/*----------------------------------------------------------------------------
 * XMLAttributeHomeRecall
 *---------------------------------------------------------------------------*/
XMLAttributeHomeRecall::XMLAttributeHomeRecall( ) 
{
}

XMLAttributeHomeRecall::~XMLAttributeHomeRecall( ) 
{
}

int XMLAttributeHomeRecall::getPoint( ) const
{
    return point.getValue( );
}

void XMLAttributeHomeRecall::setPoint( int point ) 
{
    this->point = point;
}
    

