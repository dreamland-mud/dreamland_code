/* $Id: command.cpp,v 1.1.2.12.6.12 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * logic based on interpret() from DreamLand 2.0
 */
#include <string.h>
#include "command.h"
#include "commandhelp.h"
#include "commandmanager.h"
#include "commandflags.h"

#include "dl_ctype.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "helpmanager.h"
#include "skillreference.h"
#include "room.h"

#include "descriptor.h"
#include "loadsave.h"
#include "merc.h"
#include "act.h"
#include "def.h"

GSN(manacles);

/*--------------------------------------------------------------------------
 * Command
 *-------------------------------------------------------------------------*/
Command::Command( ) 
    : extra(0, &command_flags), 
      position(POS_DEAD, &position_table),
      order(0, &order_flags),
      cat(CMD_CAT_MISC, &command_category_flags)
{
}

Command::~Command( ) 
{
    
}

const DLString& Command::getName( ) const
{
    return name.get(EN);
}
const Flags & Command::getExtra( ) const
{
    return extra;
}
short Command::getLevel( ) const
{
    return level.getValue( );
}
short Command::getLog( ) const
{
    return log.getValue( );
}
const Enumeration & Command::getPosition( ) const
{
    return position;
}
const Flags & Command::getCommandCategory( ) const
{
    return cat;
}
const Flags & Command::getOrder( ) const
{
    return order;
}
const DLString& Command::getHint( ) const
{
    return hint.get(RU);
}

::Pointer<CommandHelp> Command::getHelp( ) const
{
    return help;
}

const DLString & Command::getRussianName( ) const
{
    return name.get(RU);
}

bool Command::available( Character *ch ) const 
{
    if (getLevel( ) > ch->get_trust( ))
        return false;

    if (ch->desc && ch->desc->connected != CON_PLAYING)
        return getExtra( ).isSet( CMD_NANNY );

    return true;
}

bool Command::visible( Character *ch ) const
{
    if (!available( ch ))
        return false;
    
    if (getExtra( ).isSet( CMD_HIDDEN ))
        return false;

    return true;
}


bool Command::matchesExactly( const DLString &input ) const
{
    if (input.empty( ))
        return false;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        const DLString &cmdName = name.get(lang);

        if (!cmdName.empty())
            if (input == cmdName)
                return true;

        for (auto &alias: aliases.get(lang).split(" "))
            if (input == alias)
                return true;
    }

    return false;
}

bool Command::matches( const DLString& input ) const
{
    if (input.empty( ))
        return false;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        const DLString &cmdName = name.get(lang);

        if (!cmdName.empty())
            if (input.strPrefix(cmdName))
                return true;

        for (auto &alias: aliases.get(lang).split(" "))
            if (input.strPrefix(alias))
                return true;

    }

    return false;
}

int Command::properOrder( Character *ch ) const
{
    if (ch->is_immortal())
        return RC_ORDER_ERROR;

    if (getLevel() > LEVEL_MORTAL)
        return RC_ORDER_ERROR;

    if (!ch->is_npc( ) && getOrder( ).isSet( ORDER_ALLOW_RULER ) && ch->isAffected(gsn_manacles )) 
        return RC_ORDER_OK;

    if (getOrder().isSet(ORDER_NEVER))
        return RC_ORDER_ERROR;

    if (!ch->is_npc() && !getOrder().isSet(ORDER_EXCEPT_PK))
        return RC_ORDER_OK;
    
    if (getOrder( ).isSet( ORDER_FIGHT_ONLY ) && ch->fighting == 0)
        return RC_ORDER_NOT_FIGHTING;
    
    if (getOrder( ).isSet( ORDER_PLAYER_ONLY ))
        return RC_ORDER_NOT_PLAYER;
    
    if (getOrder( ).isSet( ORDER_THIEF_ONLY ) && !IS_SET(ch->act, ACT_THIEF))
        return RC_ORDER_NOT_THIEF;

    return RC_ORDER_OK;
}    

int Command::dispatchOrder( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (!ch->is_npc( ) 
        && IS_SET(ch->act, PLR_FREEZE)
        && !getExtra( ).isSet( CMD_FREEZE ))
    {
        ch->pecho("Ты полностью замороже%Gно|н|на!", ch);
        return RC_DISPATCH_PENALTY;
    }

    if (IS_SET( ch->comm, COMM_AFK ) && !getExtra( ).isSet( CMD_AFK )) {
        ch->pecho("Выйди сначала из {WAFK{x");
        return RC_DISPATCH_AFK;
    }

    if (getExtra( ).isSet( CMD_SPELLOUT ) && !matchesExactly( iargs.cmdName.toLower( ) )) {
        ch->pecho("Команду '%s' необходимо ввести полностью.", getName( ).c_str( ) );
        return RC_DISPATCH_SPELLOUT;
    }
                    
    if (IS_AFFECTED( ch, AFF_STUN ) && !getExtra( ).isSet( CMD_KEEP_HIDE )) {
        ch->pecho("Ты не в состоянии сделать это.");
        return RC_DISPATCH_STUN;
    }

    // prevent ghosts from doing a bunch of commands
    if (IS_GHOST( ch ) && !getExtra( ).isSet( CMD_GHOST )) {
        ch->pecho("У тебя нет тела... А твой немощный дух не в состоянии тебе помочь.");
        return RC_DISPATCH_GHOST;
    }

    if (getExtra().isSet(CMD_NO_DUNGEON) && IS_SET(ch->in_room->areaIndex()->area_flag, AREA_DUNGEON)) {
        ch->pecho("Эта команда сейчас недоступна.");
        return RC_DISPATCH_NOT_HERE;
    }

    // Character not in position for command?
    if (!checkPosition( ch ))
        return RC_DISPATCH_POSITION;

    // Come out of hiding for most commands
    visualize( ch );

    return RC_DISPATCH_OK;
}

int Command::dispatch( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (IS_CHARMED(ch) 
        && !(ch->is_npc( ) && ch->getNPC( )->switchedFrom)) 
    {
        ch->pecho( "Сперва спроси разрешения у любим%1$Gого|ого|ой хозя%1$Gина|ина|йки!" , ch->master );
        return RC_DISPATCH_CHARMED;
    }

    if (ch->isAffected(gsn_manacles ) && !getExtra( ).isSet( CMD_MANACLES )) {
        ch->pecho("Ты не можешь этого сделать - мешают кандалы!");
        return RC_DISPATCH_CHARMED;
    }

    return dispatchOrder( iargs );
}

void Command::visualize( Character *ch )
{
    if (!getExtra( ).isSet( CMD_KEEP_HIDE ))
        strip_hide_and_fade(ch);
   
    if (getPosition( ).getValue( ) == POS_FIGHTING)
        strip_improved_invisibility(ch);

    if (getPosition( ).getValue( ) > POS_RESTING || getExtra( ).isSet( CMD_UNDIG ))
        undig(ch);    

    // All active commands interrupt rituals.
    if (!ch->is_npc() && ch->act.isSet(PLR_RITUAL) && !getExtra().isSet(CMD_AFK)) {
        affect_bit_strip(ch, &plr_flags, PLR_RITUAL, true);
    }
}


bool Command::checkPosition( Character *ch )
{
    if (ch->position >= getPosition( ).getValue( ))
        return true;

    switch (ch->position.getValue( )) {
        case POS_DEAD:
            ch->pecho("Лежи смирно! Ты {RТРУП{x.");
            break;

        case POS_MORTAL:
        case POS_INCAP:
            ch->pecho("Даже не думай об этом! Ты в ужасном состоянии.");
            break;

        case POS_STUNNED:
            ch->pecho("Ты не в состоянии сделать это.");
            break;

        case POS_SLEEPING:
            ch->pecho("Во сне? Или может сначала проснешься...");
            break;

        case POS_RESTING:
            ch->pecho("Уфф... Но ведь ты отдыхаешь...");
            break;

        case POS_SITTING:
            ch->pecho("Сидя? Или может сначала встанешь...");
            break;

        case POS_FIGHTING:
            oldact_p("Куда! Ты долж$gно|ен|на сражаться.", ch, 0, 0, TO_CHAR, POS_FIGHTING );
            break;
    }
    
    return false;
}

void Command::entryPoint( Character *ch, const DLString &constArgs )
{
    run(ch, constArgs);
}

void Command::run( Character * ch, const DLString & constArguments ) 
{
    char argument[MAX_STRING_LENGTH];

    strcpy( argument, constArguments.c_str( ) );
    run( ch, argument );
}

void Command::run( Character *, char * ) 
{ 
}

