/* $Id: confirm.cpp,v 1.1.2.21.6.6 2009/09/05 18:30:47 rufina Exp $
 *
 * ruffina, 2003
 */

#include "confirm.h"

#include "class.h"
#include "logstream.h"

#include "pcmemoryinterface.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "race.h"
#include "xmlattributes.h"

#include "clanreference.h"
#include "wiznet.h"
#include "ban.h"
#include "interp.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "messengers.h"
#include "replay.h"
#include "websocketrpc.h"
#include "descriptor.h"
#include "def.h"
    
CLAN(none);

static XMLAttributeConfirm::Pointer get_confirm_attr(PCMemoryInterface *pcm)
{
    return pcm->getAttributes( ).getAttr<XMLAttributeConfirm>( "confirm" );
}

static XMLAttributeConfirm::Pointer find_confirm_attr(PCMemoryInterface *pcm)
{
    return pcm->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );
}

/*----------------------------------------------------------------------------- 
 * 'confirm' command
 *---------------------------------------------------------------------------*/
COMMAND(Confirm, "confirm")
{
    DLString arguments = constArguments;
    DLString cmd;
    
    arguments.stripWhiteSpace( );
    cmd = arguments.getOneArgument( );
    
    if (ch->is_npc( )) {
        ch->send_to( "Тебе нельзя.\n\r" );
        return;
    }
    
    if (cmd.empty( )) {
        usage( ch );
    }
    else if (cmd.strPrefix( "request" ) 
             || cmd.strPrefix( "попросить" )) 
    {
        doRequest( ch );
    }
    else if ((cmd.strPrefix( "accept" ) 
              || cmd.strPrefix( "принять" )) && ch->is_immortal( ))
    {
        doAccept( ch, arguments );
    }
    else if ((cmd.strPrefix( "remove" ) 
             || cmd.strPrefix( "reject" ) 
             || cmd.strPrefix( "отклонить" )) && ch->is_immortal( ))
    {
        doReject( ch, arguments );
    }
    else if ((cmd.strPrefix( "delete" ) 
              || cmd.strPrefix( "удалить" )) && ch->is_immortal( ))
    {
        doDelete( ch, arguments );
    }
    else if (arg_is_list( cmd ) && ch->is_immortal( )) 
    {
        doList( ch, arg_oneof(arguments, "new") );
    }
    else if (arg_is_show( cmd ) && ch->is_immortal( ))
    {
        doShow( ch, arguments );
    }
    else 
    { 
        usage( ch );
    }
}

void Confirm::doRequest( Character *ch ) 
{
    XMLAttributeConfirm::Pointer attr;
    DLString descr;    

    if (IS_SET( ch->act, PLR_CONFIRMED )) {
        ch->send_to( "Твой персонаж уже подтвержден.\n\r" );
        return;
    }
    
    if (ch->desc && banManager->checkVerbose( ch->desc, BAN_CONFIRM )) 
        return;
    
    if (ch->getDescription( )) {
        descr = ch->getDescription( );
        descr.stripWhiteSpace( );
    }
    
    if (descr.empty( )) {
        ch->send_to( "Прочитай внимательно '{lRсправка подтверждение{lEhelp confirm{lx' и '{lRсправка описание{lEhelp description{lx'.\r\n" );
        return;
    }

    attr = get_confirm_attr(ch->getPC());
    
    ch->println( "Твое описание отправлено Бессмертным на рассмотрение." );
    wiznet( WIZ_CONFIRM, 0, 0,
            "%^C1 просит подтверждения своему персонажу ({y{hcconfirm list new{x).", ch );    
    send_telegram("Вниманию богов: кто-то попросил подтверждения своему персонажу.");

    attr->update( ch ); 
    PCharacterManager::saveMemory( ch->getPC( ) );
}

void Confirm::doAccept( Character *ch, DLString& arguments ) 
{
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    PCharacter *victim;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
        ch->send_to( "Подтвердить кого?\r\n" );
        return;
    }

    pci = PCharacterManager::find( name );
    
    if (!pci) {
        ch->send_to( "Player not found. Misspeled name?\r\n" );
        return;
    }

    victim = pci->getPlayer( );

    if (victim && IS_SET(victim->act, PLR_CONFIRMED)) {
        ch->send_to( "Этот персонаж уже подтвержден.\r\n" );
        return;
    }
            
    attr = find_confirm_attr(pci);

    if (!attr) { 
        if (!victim) {
            ch->send_to( "Может, хотя бы взглянешь на него?\r\n" );
            return;
        } 
        
        if (!ch->isCoder( )) {
            ch->printf( "От %s не было заявки на подтверждение персонажа.\n\r", pci->getName( ).c_str( ) );
            return;
        }

        attr = get_confirm_attr(pci);
        attr->update( victim );        
    } 

    attr->responsible.setValue( ch->getNameP( ) );
    attr->reason.setValue( arguments );
    attr->accepted.setValue( true );

    PCharacterManager::saveMemory( pci );

    ch->send_to( "Ok.\r\n" );

    wiznet( WIZ_CONFIRM, 0, 0,
            "Персонаж %^N1 подтвержден %C5.", pci->getName( ).c_str( ), ch );

    if (victim) 
        attr->run( victim );
}

void Confirm::doReject( Character *ch, DLString& arguments ) 
{
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    PCharacter *victim;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
        ch->send_to( "Unconfirm whom?\r\n" );
        return;
    }

    if (arguments.empty( )) {
        ch->send_to( "Нужно указать причину отказа.\r\n" );
        return;
    }

    pci = PCharacterManager::find( name );
    
    if (!pci) {
        ch->send_to( "Player not found. Misspeled name?\r\n" );
        return;
    }

    attr = find_confirm_attr(pci);
    victim = pci->getPlayer( );
    
    if (!attr) { 
        if (!victim) { 
            ch->send_to( "Может, хотя бы взглянешь на него?\r\n" );
            return;
        } else { 
            attr = get_confirm_attr(pci);
            attr->update( victim );        
        }
    }

    attr->responsible.setValue( ch->getNameP( ) );
    attr->reason.setValue( arguments );
    attr->accepted.setValue( false );

    PCharacterManager::saveMemory( pci );

    ch->send_to( "Ok.\r\n" );

    wiznet( WIZ_CONFIRM, 0, 0,
            "%^C1 отказывает в подтверждении персонажу %^N1.", ch, pci->getName( ).c_str( ) );

    if (victim) 
        attr->run( victim );
}

void Confirm::doDelete( Character *ch, DLString& arguments ) 
{
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
        ch->send_to( "Чью заявку удалить?\r\n" );
        return;
    }

    pci = PCharacterManager::find( name );
    
    if (!pci) {
        ch->send_to( "Player not found. Misspeled name?\r\n" );
        return;
    }

    attr = find_confirm_attr(pci);
    
    if (!attr) {
        ch->send_to( "Этот игрок не посылал заявки на подтверждение.\r\n" );
        return;
    }
    
    pci->getAttributes( ).eraseAttribute( "confirm" );
    PCharacterManager::saveMemory( pci );

    ch->send_to( "Ok.\r\n" );
}

void Confirm::doUnread( Character *ch )
{
    const PCharacterMemoryList &players = PCharacterManager::getPCM( );    
    int newRequests = 0;

    for (auto &p: players) {
        XMLAttributeConfirm::Pointer attr = find_confirm_attr(p.second);
        
        if (attr && attr->responsible.empty())
            newRequests++;
    }

    if (newRequests > 0)
        ch->pecho("Тебя ожида%1$Iет|ют|ют %1$d нов%1$Iая|ые|ых заяв%1$Iка|ки|ок на подтверждение персонажа ({y{hcconfirm list new{x).",
                  newRequests);
}

void Confirm::doList( Character *ch, bool newOnly ) 
{
    PCharacterMemoryList::const_iterator i;
    XMLAttributeConfirm::Pointer attr;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    ostringstream buf;
    int totalRequests = 0, newRequests = 0;
    
    const DLString lineFormat = web_cmd(ch, "confirm show $1", "%-15s") + " %-13s  %-9s %s\r\n";
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        attr = find_confirm_attr(i->second);
        
        if (!attr)
            continue;

        DLString rp = attr->responsible.getValue( );

        totalRequests++;
        if (rp.empty())
            newRequests++;

        if (newOnly && !rp.empty())
            continue;

        buf << dlprintf(lineFormat.c_str(), 
                i->second->getName( ).c_str( ),
                attr->date.getTimeAsString("%H:%M %b %d" ).c_str( ),
                rp == "" ? "" :  (attr->accepted.getValue( ) ? "accepted" : "rejected"),
                rp.c_str( ) );        
    }

    if (!buf.str().empty()) {
        ch->printf( "{W%-15s  %-15s %-9s %s{x\r\n", "Name", "Date", "State", "Responsible" );
        ch->send_to(buf);
    }

    if (totalRequests == 0) {
        ch->println("Нет ни одной заявки на подтверждение персонажа.");
    } else if (newRequests != 0) {
        ch->pecho("Найден%1$Iа|о|о %1$d нов%1$Iая|ые|ых заяв%1$Iка|ки|ок, всего заявок: %2$d.",
                  newRequests, totalRequests);
    } else {
        ch->pecho("Нерассмотренных заявок на подтверждение нет. Всего заявок: %1$d.", totalRequests);
    }
}

void Confirm::doShow( Character *ch, DLString& argument ) 
{
    std::basic_ostringstream<char> buf;
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = argument.getOneArgument( );

    pci = PCharacterManager::find( name );

    if (!pci) {
        ch->send_to( "Player not found. Misspeled name?\r\n" );
        return;
    }

    attr = find_confirm_attr(pci);
    
    if (!attr) {
        ch->send_to( "Этот игрок не посылал заявки на подтверждение.\r\n" );
        return;
    }

    buf << "{W" << pci->getName( ) << "{x, "
        << "{W" << pci->getRussianName().getFullForm() << "{x, "
        << "level " << pci->getLevel( ) << ", "
        << sex_table.name( pci->getSex( ) ) << " "
        << pci->getRace( )->getName( ) << " "
        << pci->getProfession( )->getName( ).c_str( );

    if (pci->getClan( ) != clan_none)
        buf << ", clan " << pci->getClan( )->getShortName( );
    
    buf << endl;

    if (!attr->responsible.getValue( ).empty( )) {
        if (attr->accepted.getValue( ))
            buf << "Accepted by ";
        else
            buf << "Rejected by ";

        buf << "{C" << attr->responsible.getValue( ) << "{x" << endl;

        if (!attr->reason.getValue( ).empty( ))
            buf << "Reason: " << attr->reason.getValue( ) << endl;
    }

    buf << endl << pci->getDescription() << endl;
    buf << "{G[{x" << web_cmd(ch, "confirm accept " + pci->getName(), "Подтвердить") << "{G] "
        << "{R[{x" << web_cmd(ch, "confirm reject " + pci->getName() + " Прочти 'справка описание'", "Отклонить") << "{R]{x"
        << endl;

    ch->send_to( buf );
}

void Confirm::usage( Character *ch ) 
{
    std::basic_ostringstream<char> buf;
   
    buf << "Формат:" << endl
        << "{lEconfirm{lRподтверждение{lx {lErequest{lRпопросить{x  - послать заявку на подтверждение персонажа" << endl;

    if (!ch->is_immortal( )) {
        ch->send_to( buf );
        return;
    }

    buf << endl << "Для бессмертных:" << endl
        << "confirm list     - показать список заявок" << endl
        << "confirm list new - показать только нерассмотренные заявки" << endl
        << "confirm accept <player> [<reason>] - подтвердить персонаж" << endl
        << "confirm reject <player> <reason>   - отказать в подтверждении" << endl
        << "confirm delete <player>            - удалить заявку из списка" << endl
        << "confirm show   <player>            - показать детали заявки" << endl;

    ch->send_to( buf );
}

/*----------------------------------------------------------------------------- 
 * XMLAttributeConfirm
 *---------------------------------------------------------------------------*/

void XMLAttributeConfirm::run( Character *ch ) 
{
    std::basic_ostringstream<char> buf;
    
    if (responsible.getValue( ).empty( )) 
        return;
    
    if (accepted.getValue( )) {
        buf << "{WТвой персонаж подтвержден Богами.{x" << endl;
        SET_BIT( ch->act, PLR_CONFIRMED );
    }
    else {
        buf << "{RТвоему персонажу отказано в подтверждении.{x" << endl;
        REMOVE_BIT( ch->act, PLR_CONFIRMED );
    }
    
    if (!reason.getValue( ).empty( )) 
        buf << reason.getValue( );

    remember_history_private(ch->getPC(), buf.str());
    buf << endl;
    ch->send_to( buf );

    ch->getPC( )->getAttributes( ).eraseAttribute( "confirm" );
}

void XMLAttributeConfirm::update( Character *ch ) 
{
    date.setTime( Date::getCurrentTime( ) );
    responsible.setValue( "" );
    reason.setValue( "" );
    accepted.setValue( false );
}

/*---------------------------------------------------------------------------- 
 * XMLAttributeConfirmListenerPlugin
 *---------------------------------------------------------------------------*/

void XMLAttributeConfirmListenerPlugin::run( int oldState, int newState, Descriptor *d ) 
{
    std::basic_ostringstream<char> buf;
    Character *ch;
    XMLAttributeConfirm::Pointer attr;
    
    if (newState != CON_PLAYING)
        return;
    
    ch = d->character;
    if (!ch)
        return;

    if (ch->is_immortal( ))
        Confirm::doUnread(ch);

    attr = find_confirm_attr(ch->getPC());
    if (attr)
        attr->run( ch );

}
