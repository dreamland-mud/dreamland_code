/* $Id: twitlist.cpp,v 1.1.2.1.8.2 2009/08/16 02:50:30 rufina Exp $
 *
 * ruffina, 2005
 */

#include <ostream>
#include <algorithm>

#include "twitlist.h"
#include "commonattributes.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "arg_utils.h"

static const DLString TWIT_ATTR_NAME = "twit";

COMMAND(CTwit, "twit")
{
    PCharacter *pch;
    DLString cmd;
    DLString arguments = constArguments;

    arguments.stripWhiteSpace( );
    cmd = arguments.getOneArgument( );
    
    if (ch->is_npc( )) {
        ch->pecho("Тебе нельзя.");
        return;
    }
    
    pch = ch->getPC( );

    if (cmd.empty( ))
        doUsage( pch );
    else if (arg_oneof( cmd, "add", "добавить" ))
        doAdd( pch, arguments );
    else if (arg_is_list( cmd ))
        doList( pch );
    else if (arg_oneof( cmd, "remove", "удалить" ))
        doRemove( pch, arguments );
    else
        doUsage( pch );
}

void CTwit::doAdd( PCharacter *ch, DLString &arg )
{
    DLString name;
    PCMemoryInterface *pci;

    if (arg.empty( )) {
        ch->pecho("Использование: {lRигнор добавить{lEtwit add{lx <имя>");
        return;
    }

    if (( pci = PCharacterManager::find( arg ) ))
        name = pci->getName( );
    else {
        ch->pecho( "Жертва с таким именем не обнаружена." );
        return;
    }
    
    auto twitAttr = ch->getAttributes( ).getAttr<XMLStringListAttribute>( TWIT_ATTR_NAME );

    if (std::find(twitAttr->begin(), twitAttr->end(), name) != twitAttr->end()) {
        ch->pecho( "Это имя уже в списке.");
        return;
    }
    
    twitAttr->push_back( name );
    ch->pecho( "Имя добавлено в список." );
}

void CTwit::doRemove( PCharacter *ch, DLString &arg )
{
    if (arg.empty( )) {
        ch->pecho("Использование: {lRигнор удалить{lEtwit rem{lx <имя> ");
        return;
    }
    
    auto twitAttr = ch->getAttributes( ).findAttr<XMLStringListAttribute>( TWIT_ATTR_NAME );

    if (!twitAttr || twitAttr->size( ) == 0) {
        ch->pecho("Список и так пуст.");
        return;
    }
    
    for (auto iter = twitAttr->begin( ); iter != twitAttr->end( ); iter++) {
        auto blockedPerson = PCharacterManager::find(iter->getValue());

        if (arg.equalLess(iter->getValue( )) 
            || (blockedPerson && blockedPerson->getNameP('1').equalLess(arg))) {

            ch->pecho( "Имя удалено из списка.", iter->getValue( ).c_str( ) );
            twitAttr->erase( iter );
            return;
        }
    }
            
    ch->pecho( "Но имени \"%s\" нет в списке.", arg.c_str( ) );
}

void CTwit::doList( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;

    auto twitAttr = ch->getAttributes( ).findAttr<XMLStringListAttribute>( TWIT_ATTR_NAME );
    
    if (!twitAttr || twitAttr->size( ) == 0) {
        buf << "Список пуст." << endl;
    } else {
        buf << "Список персонажей, которых вы не желаете слышать: " << endl;
        
        for (auto blockedName: **twitAttr) {
            auto blockedPerson = PCharacterManager::find(blockedName);

            if (blockedPerson)
                buf << blockedPerson->getNameP('1') << endl;
            else
                buf << blockedName << " (персонаж не найден)" << endl;
        }
    }

    ch->send_to( buf );
}

void CTwit::doUsage( PCharacter *ch )
{
    ch->pecho( "{lRигнор добавить{lEtwit add{lx <имя> \r\n"
                 "{lRигнор удалить{lEtwit rem{lx <имя> \r\n"
                 "{lRигнор список{lEtwit list{x" );
}

bool talker_is_ignored( PCharacter *ch, PCharacter *talker )
{
    auto twitAttr = ch->getAttributes( ).findAttr<XMLStringListAttribute>( TWIT_ATTR_NAME );

    if (!twitAttr)
        return false;

    auto talkerName = talker->getName();

    return std::find(twitAttr->begin(), twitAttr->end(), talkerName) != twitAttr->end();
}

