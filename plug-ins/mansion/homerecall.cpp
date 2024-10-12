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
#include "hometown.h"
#include "merc.h"
#include "arg_utils.h"

#include "def.h"

HOMETOWN(frigate);

/*----------------------------------------------------------------------------
 * 'homerecall' command 
 *---------------------------------------------------------------------------*/
COMMAND(HomeRecall, "homerecall")
{
    DLString cmd;
    DLString arguments = constArguments;
    PCharacter *pch = ch->getPC( );

    if (ch->is_npc( )) {
        ch->pecho( "В лес!" );
        return;
    }
    
    if (arguments.empty( )) {
        doRecall( pch, DLString::emptyString );
        return;
    }
    
    cmd = arguments.getOneArgument( );

    if (!pch->is_immortal( )) {
        if (arg_is_list( cmd )) 
            doListMortal( pch );
        else if (arg_is_help( cmd )) 
            doUsage( pch ); 
        else
            doRecall( pch, cmd );
        return;
    }
   
    if (arg_is_list( cmd ))
        doList( pch );
    else if (cmd.strPrefix( "set" ))
        doSet( pch, arguments );
    else if (arg_is_show( cmd ))
        doShow( pch, arguments );
    else if (cmd.strPrefix( "remove" ))
        doRemove( pch, arguments );
    else if (arg_is_help( cmd )) 
        doUsage( pch ); 
    else 
        doRecall( pch, cmd );
}

class HomeRecallMovement : public RecallMovement {
public:
    HomeRecallMovement( Character *ch, const DLString &label )
               : RecallMovement( ch )
    {
        this->label = label;
    }
    HomeRecallMovement( Character *ch, Character *actor, Room *to_room )
               : RecallMovement( ch )
    {
        this->actor = actor;
        this->to_room = to_room;
    }
    
protected:
    DLString label;

    virtual bool findTargetRoom( )
    {
        XMLAttributeHomeRecall::Pointer attr;
        
        if (to_room)
            return true;

        if (ch->is_npc( )) {
            msgSelf( ch, "В лес!" );
            return false;
        }

        if (ch->getPC( )->getHometown( ) == home_frigate) {
            msgSelf( ch, "Близость Хаоса возмущает эфир! Похоже, Галеон сейчас твой единственный вид транспорта." );
            return false;
        }

        attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" );
        if (!attr) {
            msgSelf( ch, "У тебя нет своего дома." );
            return false;
        }

        int vnum = attr->getLabeledPoint( label );
        if (vnum <= 0 && !label.empty( )) {
            msgSelf( ch, "У тебя нету дома, помеченного такой меткой." );
            return false;
        }

        if (!( to_room = get_room_instance( vnum ) )) {
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
            msgRoomNoParty( wch, 
                            "%1$^C1 появляется рядом с тобой." ,
                            "%1$^C1 и %2$C1 появляются рядом с тобой." );
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

void HomeRecall::doRecall( PCharacter * ch, const DLString& label )
{
    HomeRecallMovement( ch, label ).move( );
}

void HomeRecall::doSet( PCharacter * ch, DLString &arg )
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    DLString vnumArg = arg.getOneArgument( );
    DLString label = arg; 

    try {
        vnum = vnumArg.toInt( );
    } catch (const ExceptionBadType& e) {
        ch->pecho( "<room vnum> должно быть числом." );
        return;
    }
    
    pci = PCharacterManager::find( name );
    if (!pci) {
        ch->pecho( "Жертва не найдена." );
        return;
    }
   
    Room *target =  get_room_instance( vnum );
    if (!target) {
        ch->pecho( "Комнаты с таким номером не существует." );
        return;
    }

    pci->getAttributes( ).getAttr<XMLAttributeHomeRecall>( "homerecall" )->setPoint( vnum, label );
    PCharacterManager::saveMemory( pci );

    if (label.empty( ))
        ch->pecho( "Персонажу %s установлен основной дом в комнате [%d] %s.", 
                pci->getName( ).c_str( ), vnum, target->getName()  );
    else
        ch->pecho( "Персонажу %s установлен дом с меткой %s в комнате [%d] %s.", 
                pci->getName( ).c_str( ), label.c_str( ), vnum, target->getName()  );
}

static void print_room( int vnum, ostringstream &buf )
{
    Room *room = get_room_instance( vnum );
    if (!room) {
        buf << "[" << vnum << "] не существует!" << endl;
        return;
    }

    buf << "[" << vnum << "] " << room->getName() << " (" << room->areaName() << ")" << endl;
}

void HomeRecall::doShow( PCharacter * ch, DLString &arg )
{
    XMLAttributeHomeRecall::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    
    pci = PCharacterManager::find( name );
    if (!pci) {
        ch->pecho( "Жертва не найдена." );
        return;
    }
    
    attr = pci->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
        ch->pecho( "%s бездомное..", pci->getName( ).c_str( ) );
        return;
    }
    
    ostringstream buf;
    buf << "Основной дом: ";
    print_room( attr->getPoint( ), buf );

    for (XMLAttributeHomeRecall::LabeledPoints::const_iterator l = attr->getLabeled( ).begin( ); l != attr->getLabeled( ).end( ); l++) {
        buf << "Дом с меткой '" << l->first << "': ";
        print_room( l->second, buf );
    }
    ch->send_to( buf );
}

void HomeRecall::doRemove( PCharacter * ch, DLString &arg )
{
    XMLAttributeHomeRecall::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arg.getOneArgument( );
    
    pci = PCharacterManager::find( name );
    if (!pci) {
        ch->pecho( "Жертва не найдена." );
        return;
    }
    
    attr = pci->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
        ch->pecho( "%s бездомное..", pci->getName( ).c_str( ) );
        return;
    }

    pci->getAttributes( ).eraseAttribute( "homerecall" );
    PCharacterManager::saveMemory( pci );

    ch->pecho( "Done." );
}

void HomeRecall::doList( PCharacter *ch ) 
{
    int point;
    Room * room;
    PCharacterMemoryList::const_iterator i;
    XMLAttributeHomeRecall::Pointer attr;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
   
    ch->pecho( "Список всех персонажей, имеющих homerecalls: \r\n");
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        attr = i->second->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 

        if (!attr)
            continue;
        
        point = attr->getPoint( );
        room = get_room_instance( point );
        
        ch->pecho("%-15s [%-5d] %-25.25s (%s)", 
                 i->second->getName( ).c_str( ), point, 
                 (room ? room->getName() : "{Rnull!{x"),
                 (room ? room->areaName().c_str() : "") );
    }
}

static void print_room_mortal( int vnum, ostringstream &buf )
{
    Room *room = get_room_instance( vnum );
    if (!room) {
        buf << "не существует!" << endl;
        return;
    }

    buf << room->getName() << " (" << room->areaName() << ")" << endl;
}

void HomeRecall::doListMortal( PCharacter * ch )
{
    XMLAttributeHomeRecall::Pointer attr = ch->getAttributes( ).findAttr<XMLAttributeHomeRecall>( "homerecall" ); 
    if (!attr) {
        ch->pecho( "У тебя нет своего дома." );
        return;
    }
    
    ostringstream buf;
    buf << "Основной дом: ";
    print_room_mortal( attr->getPoint( ), buf );

    for (XMLAttributeHomeRecall::LabeledPoints::const_iterator l = attr->getLabeled( ).begin( ); l != attr->getLabeled( ).end( ); l++) {
        buf << "Дом с меткой '" << l->first << "': ";
        print_room_mortal( l->second, buf );
    }
    ch->send_to( buf );
}

void HomeRecall::doUsage( PCharacter * ch )
{
    std::basic_ostringstream<char> buf;

    buf << "Синтаксис: " << endl
        << "{Wдомой     {x             - переносит в дом" << endl
        << "{Wдомой метка     {x       - переносит в дом с указанной меткой" << endl;
    if (ch->is_immortal( ))
        buf << "{Whomerecall set{x <name> <room vnum>   - установить игроку комнату для homerecall" << endl
            << "в идеале это комната снаружи дома, от которого он может купить ключ" << endl
            << "{Whomerecall set{x <name> <room vnum> <label>  " << endl
            << "                                    - установить homerecall с указанной меткой" << endl
            << "{Whomerecall show{x <name>              - посмотреть чей-то homerecall" << endl
            << "{Whomerecall remove{x <name>            - отобрать возможность рекаллиться домой" << endl
            << "{Whomerecall list{x                     - список всех игроков, имеющих homerecall" << endl;
    else 
        buf << "{Wдомой список   {x        - показать список твоих домов и меток" << endl;

    
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
    return point;
}

int XMLAttributeHomeRecall::getLabeledPoint( const DLString &label ) const
{
    if (label.empty( ))
        return point.getValue( );
    else {
        LabeledPoints::const_iterator l = labeled.find( label );
        if (l == labeled.end( ))
            return 0;
        else
            return l->second;
    }
        
}

void XMLAttributeHomeRecall::setPoint( int point, const DLString &label ) 
{
    if (label.empty( ))
        this->point = point;
    else
        labeled[label] = point;
}
    
const XMLAttributeHomeRecall::LabeledPoints & XMLAttributeHomeRecall::getLabeled( ) const
{
    return labeled;
}

