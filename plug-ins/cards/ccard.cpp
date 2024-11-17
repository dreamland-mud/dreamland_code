/* $Id: ccard.cpp,v 1.1.2.8.6.4 2009/09/05 18:30:47 rufina Exp $
 *
 * ruffina, 2005
 */

#include "ccard.h"
#include "mobiles.h"
#include "xmlattributecards.h"

#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "room.h"

#include "merc.h"
#include "arg_utils.h"
#include "loadsave.h"
#include "save.h"
#include "def.h"

COMMAND(CCard, "card")
{
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    PCharacter *pch = ch->getPC( );

    if (!pch)
        return;
    
    if (!pch->is_immortal( )) {
        pch->pecho("Это не для тебя.");
        return;
    }
    
    if (cmd.empty( )) 
        usage( pch );
    else if (arg_is(cmd, "mob")) 
        doMob( pch, arguments );
    else if (arg_is(cmd, "char" )) 
        doChar( pch, arguments );
    else if (arg_is_list( cmd )) 
        doList( pch, arguments );
    else
        usage( pch );
}

void CCard::doMob( PCharacter *ch, DLString& arguments )
{
    Character *mob;
    DLString mobName = arguments.getOneArgument( );
    
    if (mobName.empty( )) {
        ch->pecho("Кого ты хочешь сделать шестеркой?");
        return;
    }
    
    mob = get_char_world( ch, mobName.c_str( ) );

    if (!mob) {
        ch->pecho("Mobile not found.");
        return;
    }
    
    CardStarterBehavior::Pointer bhv( NEW );
    bhv->setChar( mob->getNPC( ) );
    mob->getNPC( )->behavior.setPointer( *bhv );
    save_mobs( mob->in_room );

    ch->pecho( "%s из комнаты [%d] стал(о) шестеркой.",
                mob->getNameP( '1' ).c_str( ), mob->in_room->vnum );
}

void CCard::doChar( PCharacter *ch, DLString& arguments )
{
    int level;
    PCMemoryInterface *pci;
    XMLAttributes *attributes;
    XMLAttributeCards::Pointer card;
    DLString name, arg;
    
    name = arguments.getOneArgument( );
    pci = PCharacterManager::find( name );

    if (!pci) {
        ch->pecho("Жертва не найдена.");
        return;
    }

    attributes = &pci->getAttributes( );
    card = attributes->findAttr<XMLAttributeCards>( "cards" );
    
    if (!card)
        ch->pecho( "%C1 не состоит в Колоде.", pci );
    else
        ch->pecho( "%C1 - это %s из Колоды.", pci, card->getFace( '1' ).c_str( ) );


    arg = arguments.getOneArgument( );

    if (arg.empty( ) || !ch->isCoder( )) 
        return;
    
    if (arg == "clear" || arg == "off") {
        attributes->eraseAttribute( "cards" );
        PCharacterManager::saveMemory( pci );
        ch->pecho( "Он(а) выбывает из Колоды." );        
        return;
    }

    try {
        level = arg.toInt( );
        if (level < 0 || level > 8)
            throw Exception( );
    } catch (const Exception& ) {
        ch->pecho("<card level> должен быть числом от 0 до 8.");
        return;
    }
    
    if (!card)
        card = attributes->getAttr<XMLAttributeCards>( "cards" );

    card->setLevel( level );

    if (card->getSuit( ) < 0)
        card->setSuit( card->getRandomSuit( ) );

    PCharacterManager::saveMemory( pci );
    ch->pecho( "%C1 становится %s.", pci, card->getFace( '5' ).c_str( ) );
     
}

void CCard::doList( PCharacter *ch, DLString& arguments )
{
    int cnt;
    Character *wch;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
   
    ch->pecho("Список всех карт из Колоды: \r\nИгроки: ");
    cnt = 0;
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        PCMemoryInterface *pci;
        XMLAttributeCards::Pointer card;

        pci = i->second;
        card = pci->getAttributes( ).findAttr<XMLAttributeCards>( "cards" ); 

        if (card) {
            ch->pecho( "%20s %s", 
                        pci->getName( ).c_str( ),
                        card->getFace( '1' ).c_str( ) );
            cnt++;
        }
    }
    
    if (cnt > 0)
        ch->pecho( "Итого: %d тел", cnt );

    ch->pecho( "\r\nМобы-шестерки:", cnt );
    cnt = 0;
        
    for (wch = char_list; wch; wch = wch->next) {
        NPCharacter *mob;

        if (!wch->is_npc( ))
            continue;
        if (!wch->in_room)
            continue;
    
        mob = wch->getNPC( );

        if (!mob->behavior)
            continue;
        if (!mob->behavior.getDynamicPointer<CardStarterBehavior>( ))
            continue;
        
        
        ch->pecho( "[%5d] %-28s [%5d] %s",
                    mob->pIndexData->vnum, mob->getNameP( '1' ).c_str( ),
                    mob->in_room->vnum, mob->in_room->getName() );
        cnt++;
    }
    
    if (cnt > 0)
        ch->pecho( "Итого: %d тел", cnt );
}

void CCard::usage( PCharacter *ch )
{
    std::basic_ostringstream<char> buf;

    buf << "Синтаксис: " << endl
        << "{Wcard list{x  - посмотреть всю колоду" << endl
        << "{Wcard mob {x<name>   -  сделать моба карточной шестеркой" << endl
        << "{Wcard char {x<name>  - показать место в колоде для этого игрока" << endl;
    
    if (ch->isCoder( )) {
        buf << "{Wcard char {x<name> <level> - установить игроку уровень в колоде (0..8)" << endl
            << "{Wcard char {x<name> {Wclear{x - выгнать игрока из колоды" << endl;
    }

    ch->send_to( buf );
}

