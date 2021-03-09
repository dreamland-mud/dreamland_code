
/* $Id: mkey.cpp,v 1.1.2.9.6.5 2009/09/06 21:48:28 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mkey.h"

#include "logstream.h"
#include "class.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "behavior_utils.h"
#include "pcharactermanager.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"

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
        if (arg_oneof( cmd, "give", "grant", "дать" )) {
            doGrant( ch, arguments );
            return;
        }
        else if (arg_oneof( cmd, "remove", "забрать" )) {
            doRemove( ch, arguments );
            return;
        }
        else if (arg_is_show( cmd )) {
            doShow( ch, arguments );
            return;
        }
    }
    
    if (arg_is_list( cmd ) || arg_oneof( cmd, "buy", "купить" )) {
        MansionKeyMaker::Pointer maker;

        maker = find_people_behavior<MansionKeyMaker>( ch->in_room );

        if (!maker) {
            ch->pecho("Здесь нет ключника.");
            return;
        }

        if (ch->is_npc( )) {
            ch->pecho("Ты бездомное.");
            return;
        }
        
        if (arg_is_list( cmd ))
            maker->doList( ch->getPC( ) );
        else
            maker->doBuy( ch->getPC( ), arguments );

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
        ch->pecho("Неправильный vnum ключа.");
        return;
    }

    if ( (pci = PCharacterManager::find( name )) == 0) {
        ch->pecho("Неправильное имя.");
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
    
    ch->pecho("Такой ключ не найден.");
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
        ch->pecho("Такого ключа не существует.");
        return;
    }

    if ( (pci = PCharacterManager::find( name )) == 0) {
        ch->pecho("Неправильное имя.");
        return;
    }
    
    XMLAttributes *attributes = &pci->getAttributes( );
    XMLAttributeMansionKey::Pointer attr = attributes->getAttr<XMLAttributeMansionKey>( "mkey" );
    XMLVectorBase<XMLInteger>::iterator i;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++)
        if (i->getValue( ) == vnum) {
            ch->pecho("Такой ключ у него уже есть.");
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
        ch->pecho("Неправильное имя.");
        return;
    }
    
    attributes = &pci->getAttributes( );
    attr = attributes->findAttr<XMLAttributeMansionKey>( "mkey" );

    if (!attr) {
        ch->pecho("Ключей не найдено.");
        return;
    }

    ch->printf( "%s владеет такими ключами: \r\n", pci->getName( ).c_str( ) );

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
        int vnum = i->getValue( );
        OBJ_INDEX_DATA *pKeyIndex = get_obj_index( vnum );

        if (!pKeyIndex) {
            LogStream::sendError( ) << "Wrong key vnum " << vnum << " for character " << ch->getName( ) << endl;
            continue;
        }
        
        ch->printf( "[%-4d] %-25s [%s]\r\n", 
                    vnum, 
                    russian_case( pKeyIndex->short_descr, '1' ).c_str( ),
                    pKeyIndex->name );
    }
}


void MKey::usage( Character *ch ) 
{
    std::basic_ostringstream<char> buf;

    buf << "{W{lRключи список{lEmkey list{lx{w" << endl
        << "     - показать список твоих ключей" << endl
        << "{W{lRключи купить{lEmkey buy{lx{w <имя ключа>" << endl
        << "     - приобрести ключ" << endl;
    
    if (ch->is_immortal( )) 
        buf << "{W{lRключи показать{lEmkey show{lx{w <victim>" << endl
            << "     - показать список ключей жертвы" << endl
            << "{W{lRключи дать{lEmkey give{lx{w <victim> <key vnum>" << endl
            << "     - дать ключ с заданным внумом жертве" << endl
            << "{W{lRключи забрать{lEmkey remove{lx{w <victim> <key vnum>" << endl
            << "     - забрать ключ" << endl;

    ch->send_to( buf );
}

/*-------------------------------------------------------------------------
 * MansionKeyMaker 
 *------------------------------------------------------------------------*/
void MansionKeyMaker::toStream( Character *client, ostringstream &buf ) 
{
    XMLAttributeMansionKey::Pointer attr;
    XMLVectorBase<XMLInteger>::iterator i;
    
    if (client->is_npc( ))
        return;

    attr = client->getPC( )->getAttributes( ).findAttr<XMLAttributeMansionKey>( "mkey" );
    
    if (!attr) 
        return;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
        int vnum = i->getValue( );
        OBJ_INDEX_DATA *pKeyIndex = get_obj_index( vnum );

        if (!pKeyIndex) 
            LogStream::sendError( ) << "Wrong key vnum " << vnum << " for character " << client->getName( ) << endl;
        else        
            buf << "     * " << russian_case( pKeyIndex->short_descr, '1' )
                << " ({c" << pKeyIndex->name << "{x)" << endl;
    }
}

void MansionKeyMaker::msgListEmpty( Character *client ) 
{
    say_act( client, getKeeper( ), "У тебя нет ключей ни от какого дома." );
}

void MansionKeyMaker::msgListBefore( Character *client ) 
{
    tell_dim( client, getKeeper( ), "Я могу изготовить для тебя такие ключи: " );
}

void MansionKeyMaker::msgListAfter( Character *client ) 
{
    client->pecho("");
    tell_dim( client, getKeeper( ), "За свою работу я потребую $n4.", price->toString( client ).c_str( ) );
}

void MansionKeyMaker::msgArticleNotFound( Character *client ) 
{
}

bool MansionKeyMaker::canServeClient( Character * )
{
    return true;
}

void MansionKeyMaker::msgListRequest( Character *client )
{
    act( "$c1 просит $C4 показать список ключей.", client, 0, getKeeper( ), TO_ROOM );
    act( "Ты просишь у $C4 показать список ключей.", client, 0, getKeeper( ), TO_CHAR );
}

void MansionKeyMaker::msgBuyRequest( Character *client )
{
}

void MansionKeyMaker::msgArticleTooFew( Character *, Article::Pointer )
{
}

Article::Pointer 
MansionKeyMaker::findArticle( Character *client, DLString &arg )
{
    MansionKeyArticle::Pointer article;
    int vnum ;
    
    if (client->is_npc( ))
        return article;

    if (!( vnum = findKeyVnum( client->getPC( ), arg ) ))
        return article;

    article.construct( );
    article->setPrice( price );
    article->setVnum( vnum );
    return article;
}

int MansionKeyMaker::findKeyVnum( PCharacter *client, const DLString& arg ) 
{
    XMLVectorBase<XMLInteger>::iterator i;
    XMLAttributeMansionKey::Pointer attr;

    attr = client->getAttributes( ).findAttr<XMLAttributeMansionKey>( "mkey" );

    if (!attr || attr->keys.empty( )) {
        tell_act( client, getKeeper( ), "Извини, $c1, но тебе не принадлежит ни одного ключа. " );
        return 0;
    }

    if (arg.empty( )) {
        tell_act( client, getKeeper( ), "Какой именно ключ ты хочешь купить?" );
        return 0;
    }

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
        OBJ_INDEX_DATA *pKeyIndex = get_obj_index( i->getValue( ) );

        if (!pKeyIndex)
            continue;

        if (!is_name( arg.c_str( ), pKeyIndex->name )) 
            continue;
        
        return i->getValue( );
    }

    tell_act( client, getKeeper( ), "Извини, $c1, но тебе не принадлежит ключ с таким именем." );
    return 0;
}

/*-------------------------------------------------------------------------
 * MansionKeyArticle
 *------------------------------------------------------------------------*/
bool MansionKeyArticle::purchase( Character *client, NPCharacter *maker, const DLString &, int ) 
{
    Object *key;
    
    if (!price->canAfford( client )) {
        tell_act( client, maker, 
                  "У тебя недостаточно $n2, чтобы оплатить мою работу, $c1.", 
                  price->toCurrency( ).c_str( ) );
        return false;
    }
    
    price->deduct( client );
    key = create_object( get_obj_index( vnum ), 1 );
    obj_to_char( key, client );

    act( "$C1 вручает тебе $o4.", client, key, maker, TO_CHAR );
    act( "$C1 вручает $c3 $o4." , client, key, maker, TO_ROOM );
    return true;
}

bool MansionKeyArticle::available( Character *client, NPCharacter *maker ) const
{
    return true;
}

int MansionKeyArticle::getQuantity( ) const
{
    return 1;
}
