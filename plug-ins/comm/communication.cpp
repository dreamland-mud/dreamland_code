/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"

#include "commandtemplate.h"
#include "commonattributes.h"
#include "replay.h"

#include "pcharacter.h"
#include "room.h"
#include "save.h"
#include "merc.h"

#include "act.h"
#include "interp.h"
#include "arg_utils.h"
#include "comm.h"
#include "def.h"

CMDRUNP( deaf )
{
        if (IS_SET(ch->comm,COMM_DEAF))
        {
                ch->pecho("Ты опять можешь говорить с кем-либо.");
                REMOVE_BIT(ch->comm,COMM_DEAF);
        }
        else
        {
                ch->pecho("С этого момента ты не хочешь ни с кем говорить.");
                SET_BIT(ch->comm,COMM_DEAF);
        }
}

CMDRUNP( quiet )
{
        if (IS_SET(ch->comm,COMM_QUIET))
        {
                ch->pecho("Режим полного молчания выключен.");
                REMOVE_BIT(ch->comm,COMM_QUIET);
        }
        else
        {
                ch->pecho("С этого момента ты будешь слышать только то, что произносят в комнате.");
                SET_BIT(ch->comm,COMM_QUIET);
        }
}


/**
 * Syntax:
 * replay help          помощь ?     -- эта справка
 * replay                            -- сообщения сохраненные за время боя, нахождения без связи или в AFK
 * replay private       личные       -- последние X личных сообщений
 * replay near          рядом        -- последние X сообщений и эмоций рядом или в той же арии, групповой канал
 * replay public        публичные    -- последние X сообщений в общих каналах, хрустальном шаре, клановом канале
 * replay all           все          -- все сохраненные сообщения в хронологическом порядке
 * replay <keyword> -N               -- последние N сообщений в данной категории
 *
 * X = 10, MAX for all = 100
 *
 * After fight with autostore:
 * Тебе было послано %d сообщений. Используй 'replay|прослушать' для просмотра.
 * 
 * On returning from afk:
 * Режим AFK|ВОК выключен. Используй 'replay|прослушать' для просмотра %d личных сообщений, 
 * полученных за время отсутствия, 'replay help' для справки.
 * или
 * Режим AFK|ВОК выключен. Личных сообщений за время твоего отсутствия не было.
 * Используй 'replay help' для справки о том, как просмотреть все недавние сообщения.
 * 
 * On reconnect:
 * Соединение восстановлено. Используй 'replay|прослушать' для просмотра пропущенных сообщений.
 *
 * On replay with no args:
 *  - if tells > 0: show separately stored tells for fight|afk|reconnect and delete them
 *  - show 'see also' 
 *
 * On 'replay private':
 * Запомненные личные сообщения:
 *  - show tells, pager private
 *  - show 'see also' 
 *
 * On 'replay near':
 * Запомненные сообщения рядом с тобой:
 *  - show say, social&emote, gtalk, yell
 *  - show 'see also' 
 *
 * On 'replay public':
 * Запомненные сообщения в общих каналах:
 *  - show clantalk, grats, ooc, ic, gossip, pager public
 *  - show 'see also' 
 *
 *
 * TODO:
 * timestamps like in notes
 */

static void replay_summary( PCharacter *ch )
{
    ostringstream buf;

    buf << "Используй команду {yвоспр ?{x для справки," << endl
        << "{yпрослушать личн{x для просмотра последних личных сообщений," << endl
        << "{yпрослушать рядом{x для просмотра сообщений рядом с тобой," << endl
        << "{yпрослушать общ{x для просмотра последних сообщений в общих каналах," << endl
        << "{yпрослушать все{x для просмотра всех сообщений." << endl
        << "Добавление параметра -N выдаст последние N сообщений в каждой категории." << endl;

    ch->send_to( buf );
}

static void replay_hint( PCharacter *ch )
{
    ch->pecho( "Также смотри {hc{yпрослушать ?{x." );
}

static void replay_help( PCharacter *ch )
{
    ostringstream buf;
    int c = DEFAULT_REPLAY_SIZE;

    buf << "Использование:" << endl
        << "    {Wпрослушать ?        {x  - эта справка" << endl
        << "    {Wпрослушать          {x  - вывести сообщения, сохраненные за время боя, нахождения без связи или в AFK" << endl
        << "    {Wпрослушать личные   {x  - вывести последние " << c << " личных сообщений" << endl
        << "    {Wпрослушать рядом    {x  - вывести последние " << c << " сообщений и эмоций рядом с тобой, а также групповой канал" << endl
        << "    {Wпрослушать общие    {x  - вывести последние " << c << " сообщений в общих каналах, хрустальном шаре, клановом канале" << endl
        << "    {Wпрослушать все      {x  - последние " << c << " сохраненных сообщений в хронологическом порядке" << endl
        << "    {Wпрослушать <тип> -N {x  - выдаст последние N сообщений в данной категории" << endl;
    ch->send_to( buf );
}

CMDRUNP( replay )
{
    PCharacter *pch;
    DLString arguments( argument );
    DLString arg = arguments.getOneArgument( );
    DLString arg2 = arguments.getOneArgument( );
    int limit = DEFAULT_REPLAY_SIZE;

    if (ch->is_npc( )) {
        ch->pecho("Ты не можешь использовать команду replay.");
        return;
    }

    pch = ch->getPC( );

    // Replay stored tells, show command summary.
    if (arg.empty( )) {
        ostringstream buf;

        if (ReplayAttribute::playAndErase( buf, pch )) {
            pch->pecho( "Сообщения, полученные за время твоего отсутствия или боя:" );
            pch->send_to( buf );
        } else {
            pch->pecho( "Все сообщения, полученные за время отсутствия, уже прочитаны." );
        }

        replay_hint( pch );
        return;
    }

    // Display command syntax.
    if (arg_is_help( arg )) {
        replay_help( pch );
        return;
    }

    // Support -N syntax
    if (arg2.size( ) > 1 && arg2.at(0) == '-') {
        try {
            Integer newLimit( arg2.substr( 1 ) );
            if (newLimit > 0)
                limit = newLimit;
        } catch( ExceptionBadType &e) {
        }
    }

    // Display all messages in chronological order.
    if (arg_is_all( arg )) {
        ostringstream buf;

        if (!replay_history_all( buf, pch, limit )) {
            pch->pecho( "За последнее время никто ничего не говорил." );
        } else {
            pch->pecho( "Все запомненные сообщения в хронологическом порядке:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display private messages only.
    if (arg_is(arg, "private")) {
        ostringstream buf;

        if (!replay_history_private( buf, pch, limit )) {
            pch->pecho( "За последнее время тебе никто ничего не говорил." );
        } else {
            pch->pecho( "Запомненные личные сообщения:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display public messages only.
    if (arg_is(arg, "public")) {
        ostringstream buf;

        if (!replay_history_public( buf, pch, limit )) {
            pch->pecho( "За последнее время никто ничего не говорил в общих каналах." );
        } else {
            pch->pecho( "Запомненные сообщения в общих каналах:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display nearby messages and socials.
    if (arg_is(arg, "nearby")) {
        ostringstream buf;

        if (!replay_history_near( buf, pch, limit )) {
            pch->pecho( "Рядом с тобой ничего не происходило." );
        } else {
            pch->pecho( "Запомненные сообщения рядом с тобой:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }


    // Argument not recognized.
    pch->pecho( "Неправильный параметр." );
    replay_summary( pch ); 
}


CMDRUNP( afk )
{
    PCharacter *pch = ch->getPC( );
    
    if (ch->is_npc( )) {
        ch->pecho("Вдали от чего?!");
        return;
    }
    
    if (IS_SET(pch->comm,COMM_AFK))
    {
        pch->pecho("Режим AFK выключен.");
        pch->getAttributes().handleEvent(AfkArguments(pch, false));
        REMOVE_BIT(pch->comm,COMM_AFK);
        pch->getAttributes( ).eraseAttribute( "afk" );        
    }
    else
    {
        SET_BIT(pch->comm,COMM_AFK);
        
        if (argument[0] != '\0') {
            pch->getAttributes( ).getAttr<XMLStringAttribute>( "afk" )->setValue( argument );
            pch->pecho("Ты в режиме AFK: {c%s.{x", argument);
        }
        else
            pch->pecho("Режим AFK включен.");

        pch->getAttributes().handleEvent(AfkArguments(pch, true));
    }
}



