
/* $Id: mkey.cpp,v 1.1.2.9.6.5 2009/09/06 21:48:28 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mkey.h"

#include "register-impl.h"
#include "reglist.h"

#include "logstream.h"
#include "class.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "interp.h"
#include "pcharactermanager.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "loadsave.h"

#include "def.h"
#include "l10n.h"

/*-------------------------------------------------------------------------
 * 'mkey' command
 *------------------------------------------------------------------------*/
COMMAND(MKey, "mkey")
{
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );

    if (cmd.empty( )) {
        usage( ch );
        return;
    }
    
    if (ch->is_immortal( )) {
        if (arg_is(cmd, "give")) {
            doGrant( ch, arguments );
            return;
        }
        else if (arg_is(cmd, "remove")) {
            doRemove( ch, arguments );
            return;
        }
        else if (arg_is_show( cmd )) {
            doShow( ch, arguments );
            return;
        }
    }
    
    // The key maker is a Fenia shop now (utils/keymaker): route to list/buy.
    if (arg_is_list( cmd )) {
        interpret_raw( ch, "list" );
        return;
    }

    if (arg_is(cmd, "buy")) {
        interpret_raw( ch, "buy", "%s", arguments.c_str( ) );
        return;
    }
        
    usage( ch );
}

void MKey::doRemove( Character *ch, DLString &arguments ) 
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
        usage( ch );
        return;
    }

    try {
        vnum = arguments.getOneArgument( ).toInt( );
    } catch (const ExceptionBadType& e) {
        ch->pecho(_("Неправильный vnum ключа."));
        return;
    }

    if ( (pci = PCharacterManager::find( name )) == 0) {
        ch->pecho(_("Неправильное имя."));
        return;
    }
    
    XMLAttributes *attributes = &pci->getAttributes( );
    XMLAttributeMansionKey::Pointer attr = attributes->getAttr<XMLAttributeMansionKey>( "mkey" );
    XMLVectorBase<XMLInteger>::iterator i;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++)
        if (i->getValue( ) == vnum) {
            attr->keys.erase( i );
            PCharacterManager::saveMemory( pci );
            ch->pecho("Ok.");
            return;
        }
    
    ch->pecho(_("Такой ключ не найден."));
}

void MKey::doGrant( Character *ch, DLString &arguments ) 
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
        usage( ch );
        return;
    }

    try {
        vnum = arguments.getOneArgument( ).toInt( );
    } catch (const ExceptionBadType& e) {
        usage( ch );
        return;
    }

    if (get_obj_index( vnum ) == 0) {
        ch->pecho(_("Такого ключа не существует."));
        return;
    }

    if ( (pci = PCharacterManager::find( name )) == 0) {
        ch->pecho(_("Неправильное имя."));
        return;
    }
    
    XMLAttributes *attributes = &pci->getAttributes( );
    XMLAttributeMansionKey::Pointer attr = attributes->getAttr<XMLAttributeMansionKey>( "mkey" );
    XMLVectorBase<XMLInteger>::iterator i;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++)
        if (i->getValue( ) == vnum) {
            ch->pecho(_("Такой ключ у него уже есть."));
            return;
        }
    
    attr->keys.push_back( vnum );
    PCharacterManager::saveMemory( pci );
    ch->pecho("Ok.");
}

void MKey::doShow( Character *ch, DLString &arguments ) 
{
    XMLAttributes *attributes;
    XMLAttributeMansionKey::Pointer attr;
    XMLVectorBase<XMLInteger>::iterator i;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if ( (pci = PCharacterManager::find( name )) == 0) {
        ch->pecho(_("Неправильное имя."));
        return;
    }
    
    attributes = &pci->getAttributes( );
    attr = attributes->findAttr<XMLAttributeMansionKey>( "mkey" );

    if (!attr) {
        ch->pecho(_("Ключей не найдено."));
        return;
    }

    ch->pecho( _("%s владеет такими ключами: "), pci->getName( ).c_str( ) );

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
        int vnum = i->getValue( );
        OBJ_INDEX_DATA *pKeyIndex = get_obj_index( vnum );

        if (!pKeyIndex) {
            LogStream::sendError( ) << "Wrong key vnum " << vnum << " for character " << ch->getNameC( ) << endl;
            continue;
        }
        
        ch->pecho( "[%-4d] %-25N1 [%s]",
                    vnum,
                    pKeyIndex->getShortDescr(viewerLang(ch)),
                    String::toString(pKeyIndex->keyword).c_str() );
    }
}


void MKey::usage( Character *ch ) 
{
    std::basic_ostringstream<char> buf;

    lang_t lang = viewerLang( ch );

    // Command name from the command table, sub-commands from the 'grammar'
    // synonyms arg_is() matches against, so every word printed here is a word
    // this viewer can actually type.
    DLString cmd = getNameFor( lang );
    DLString argList = lmsg( lang, "list", "список", "список" );
    DLString argBuy = lmsg( lang, "buy", "купить", "купити" );
    DLString argShow = lmsg( lang, "show", "показать", "показати" );
    DLString argGive = lmsg( lang, "give", "дать", "дати" );
    DLString argRemove = lmsg( lang, "remove", "забрать", "забрати" );
    DLString argVictim = lmsg( lang, "<victim>", "<жертва>", "<жертва>" );
    DLString argKeyName = lmsg( lang, "<key name>", "<имя ключа>", "<назва ключа>" );
    DLString argKeyVnum = lmsg( lang, "<key vnum>", "<внум ключа>", "<внум ключа>" );

    buf << "{W" << cmd << " " << argList << "{w" << endl
        << "     - " << lmsg( lang, "show the list of your keys",
                                    "показать список твоих ключей",
                                    "показати список твоїх ключів" ) << endl
        << "{W" << cmd << " " << argBuy << "{w " << argKeyName << endl
        << "     - " << lmsg( lang, "buy a key",
                                    "приобрести ключ",
                                    "придбати ключ" ) << endl;

    if (ch->is_immortal( ))
        buf << "{W" << cmd << " " << argShow << "{w " << argVictim << endl
            << "     - " << lmsg( lang, "show someone else's keys",
                                        "показать список ключей жертвы",
                                        "показати список ключів жертви" ) << endl
            << "{W" << cmd << " " << argGive << "{w " << argVictim << " " << argKeyVnum << endl
            << "     - " << lmsg( lang, "give the key with this vnum to the victim",
                                        "дать ключ с заданным внумом жертве",
                                        "дати ключ із заданим внумом жертві" ) << endl
            << "{W" << cmd << " " << argRemove << "{w " << argVictim << " " << argKeyVnum << endl
            << "     - " << lmsg( lang, "take the key away",
                                        "забрать ключ",
                                        "забрати ключ" ) << endl;

    ch->send_to( buf );
}

/*-------------------------------------------------------------------------
 * XMLAttributeMansionKey
 *------------------------------------------------------------------------*/
Scripting::Register XMLAttributeMansionKey::toRegister() const
{
    using namespace Scripting;
    Register attrReg = Register::handler<RegList>();
    RegList *attrList = attrReg.toHandler().getDynamicPointer<RegList>();

    for (auto &k: keys)
        attrList->push_back(Register(k.getValue()));

    return attrReg;
}
