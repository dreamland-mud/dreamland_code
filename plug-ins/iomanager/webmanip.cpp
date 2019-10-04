/* $Id$
 *
 * ruffina, 2018
 */
#include <string.h>
#include "webmanip.h"
#include "character.h"
#include "descriptor.h"

WebManipCommand::~WebManipCommand( )
{
}

void WebManipCommand::initialization( )
{
    webManipManager->registrate( Pointer( this ) );
}

void WebManipCommand::destruction( ) 
{
    webManipManager->unregistrate( Pointer( this ) );
}

WebManipManager* webManipManager = NULL;

WebManipManager::WebManipManager( ) 
{
    checkDuplicate( webManipManager );
    webManipManager = this;
}

WebManipManager::~WebManipManager( )
{
    webManipManager = NULL;
}

void WebManipManager::registrate( WebManipCommand::Pointer m )
{
    manips[m->getName( )] = m;
}

void WebManipManager::unregistrate( WebManipCommand::Pointer m )
{
    WebManipMap::iterator i = manips.find(m->getName( ));
    if (i != manips.end( ))
            manips.erase(m->getName( ));
}


void WebManipManager::decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *ch, const DLString &pocket, int combined ) const
{
        static const DLString COMMAND_NAME = "decorateItem";
        ItemManipArgs args( ch, item, descr, pocket, combined );
        if (!run( buf, COMMAND_NAME, args ))
                buf << descr;
}

void WebManipManager::decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character *ch ) const
{
        static const DLString COMMAND_NAME = "decorateShopItem";
        ShopItemManipArgs args( ch, item, descr );
        if (!run( buf, COMMAND_NAME, args ))
                buf << descr;
}

void WebManipManager::decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const
{
        static const DLString COMMAND_NAME = "decoratePocket";
        PocketManipArgs args( ch, pocket, container );
        if (!run( buf, COMMAND_NAME, args ))
                buf << pocket;
}

void WebManipManager::decorateExtraDescr( ostringstream &buf, const char *desc, extra_descr_data *ed, Character *ch ) const
{
    static const DLString COMMAND_NAME = "decorateExtraDescr";
    ExtraDescrManipArgs args( ch, desc, ed );
    if (!run( buf, COMMAND_NAME, args ))
        buf << desc;
}

void WebManipManager::decorateCharacter( ostringstream &buf, const DLString &descr, Character *victim, Character *ch ) const
{
    static const DLString COMMAND_PC = "decoratePlayer";
    static const DLString COMMAND_NPC = "decorateMobile";
    bool result;

    if (victim->is_npc( )) {
        MobManipArgs args( ch, victim->getNPC( ), descr );
        result = run( buf, COMMAND_NPC, args );
    } else {
        PlayerManipArgs args( ch, victim->getPC( ), descr );
        result = run( buf, COMMAND_PC, args );
    }

    if (!result)
        buf << descr;
}

bool WebManipManager::run( ostringstream &buf, const DLString &command, const ManipCommandArgs &args ) const
{
        bool success = false;
        
        if (!args.target || !args.target->desc || args.target->desc->connected != CON_PLAYING)
            return success;

        WebManipMap::const_iterator i = manips.find( command );
        if (i != manips.end( )) {
                WebManipCommand::Pointer cmd = i->second;
                if (cmd->run( buf, args )) {
                        success = true;
                }
        }

        return success;
}

void WebManipManager::initialization( )
{
}

void WebManipManager::destruction( )
{
}


/**
 * Create command tag in the form of [cmd=...,see=...,nonce=...],
 * which is going to become a clickable action link in webcilent.
 * The client will compare nonce with the one communicated to it in
 * 'version' command, to ensure the link comes from the server and not
 * from random grigoriy in [ooc].
 */
DLString web_cmd(Character *ch, const DLString &cmd, const DLString &seeFmt)
{
    ostringstream buf;

    if (!ch->desc)
        return seeFmt;

    if (ch->desc->websock.state != WS_ESTABLISHED)
        return seeFmt;

    buf << "[cmd=" << cmd << ",see=" << seeFmt << ",nonce=" << ch->desc->websock.nonce << "]";
    return buf.str();
}
