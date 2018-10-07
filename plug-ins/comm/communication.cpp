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
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "handler.h"
#include "comm.h"
#include "def.h"

CMDRUNP( deaf )
{
	if (IS_SET(ch->comm,COMM_DEAF))
	{
		ch->send_to("Ты опять можешь говорить с кем-либо.\n\r");
		REMOVE_BIT(ch->comm,COMM_DEAF);
	}
	else
	{
		ch->send_to("С этого момента ты не хочешь ни с кем говорить.\n\r");
		SET_BIT(ch->comm,COMM_DEAF);
	}
}

CMDRUNP( quiet )
{
	if (IS_SET(ch->comm,COMM_QUIET))
	{
		ch->send_to("Режим полного молчания выключен.\n\r");
		REMOVE_BIT(ch->comm,COMM_QUIET);
	}
	else
	{
		ch->send_to("С этого момента ты будешь слышать только то, что произносят в комнате.\n\r");
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
 * replay <keyword> -N               -- последние N сообщений в данной категории TODO
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
 * replay -N
 *
 */

static bool replay_tells( ostringstream &buf, PCharacter *ch )
{
    XMLStringListAttribute::iterator i;
    XMLStringListAttribute::Pointer tells
		= ch->getAttributes( ).findAttr<XMLStringListAttribute>( "tells" );
    
    if (!tells || tells->size( ) == 0)
	return false;

    for (i = tells->begin( ); i != tells->end( ); i++)
	buf << " ** " << *i << endl;
    
    ch->getAttributes( ).eraseAttribute( "tells" );
    return true;
}

static void replay_summary( PCharacter *ch )
{
    ostringstream buf;

    buf << "Используй команду {y{lEreplay{lRвоспр{lx ?{x для справки," << endl
        << "{y{lEreplay priv{lRпрослушать личн{x для просмотра последних личных сообщений," << endl
        << "{y{lEreplay near{lRпрослушать рядом{x для просмотра сообщений рядом с тобой," << endl
        << "{y{lEreplay pub{lRпрослушать общ{x для просмотра последних сообщений в общих каналах," << endl
        << "{y{lEreplay all{lRпрослушать все{x для просмотра всех сообщений." << endl
        << "Добавление параметра -N выдаст последние N сообщений в каждой категории." << endl;

    ch->send_to( buf );
}

static void replay_hint( PCharacter *ch )
{
    ch->println( "Также смотри {y{lRпрослушать ?{lEreplay help{lx{x." );
}

static void replay_help( PCharacter *ch )
{
    ostringstream buf;
    int c = DEFAULT_REPLAY_SIZE;

    buf << "Использование:" << endl
        << "    {W{lRпрослушать ?        {lEreplay ?        {lx{x  - эта справка" << endl
        << "    {W{lRпрослушать          {lEreplay          {lx{x  - вывести сообщения, сохраненные за время боя, нахождения без связи или в AFK" << endl
        << "    {W{lRпрослушать личные   {lEreplay private  {lx{x  - вывести последние " << c << " личных сообщений" << endl
        << "    {W{lRпрослушать рядом    {lEreplay near     {lx{x  - вывести последние " << c << " сообщений и эмоций рядом с тобой, а также групповой канал" << endl
        << "    {W{lRпрослушать общие    {lEreplay public   {lx{x  - вывести последние " << c << " сообщений в общих каналах, хрустальном шаре, клановом канале" << endl
        << "    {W{lRпрослушать все      {lEreplay all      {lx{x  - последние " << c << " сохраненных сообщений в хронологическом порядке" << endl
        << "    {W{lRпрослушать <тип> -N {lEreplay <тип> -N {lx{x  - выдаст последние N сообщений в данной категории" << endl;
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
	ch->println("Ты не можешь использовать команду replay.");
        return;
    }

    pch = ch->getPC( );

    // Replay stored tells, show command summary.
    if (arg.empty( )) {
        ostringstream buf;

        if (replay_tells( buf, pch )) {
            pch->println( "Сообщения, полученные за время твоего отсутствия:" );
            pch->send_to( buf );
        } else {
            pch->println( "Все сообщения, полученные за время отсутствия, уже прочитаны." );
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
            pch->println( "За последнее время никто ничего не говорил." );
        } else {
            pch->println( "Все запомненные сообщения в хронологическом порядке:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display private messages only.
    if (arg_oneof( arg, "личные", "персональные", "private", "personal" )) {
        ostringstream buf;

        if (!replay_history_private( buf, pch, limit )) {
            pch->println( "За последнее время тебе никто ничего не говорил." );
        } else {
            pch->println( "Запомненные личные сообщения:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display public messages only.
    if (arg_oneof( arg, "общие", "публичные", "public" )) {
        ostringstream buf;

        if (!replay_history_public( buf, pch, limit )) {
            pch->println( "За последнее время никто ничего не говорил в общих каналах." );
        } else {
            pch->println( "Запомненные сообщения в общих каналах:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display nearby messages and socials.
    if (arg_oneof( arg, "рядом", "nearby" )) {
        ostringstream buf;

        if (!replay_history_near( buf, pch, limit )) {
            pch->println( "Рядом с тобой ничего не происходило." );
        } else {
            pch->println( "Запомненные сообщения рядом с тобой:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }


    // Argument not recognized.
    pch->println( "Неправильный параметр." );
    replay_summary( pch ); 
}


CMDRUNP( afk )
{
    PCharacter *pch = ch->getPC( );
    
    if (ch->is_npc( )) {
	ch->send_to( "Вдали от чего?!\r\n" );
	return;
    }
    
    if (IS_SET(pch->comm,COMM_AFK))
    {
	XMLStringListAttribute::Pointer tells
		    = pch->getAttributes( ).findAttr<XMLStringListAttribute>( "tells" );

	if(tells && tells->size( ) > 0)
	{
            pch->pecho( "Режим AFK выключен. Тебя ожидает {R%1$d{x сообщен%1$Iие|ия|ий.\r\n"
                         "Используй команду {y{lRпрослушать{lEreplay{x, чтобы %1$Iего|их|их прочитать.", tells->size( ) );
	}
	else
	{
            pch->pecho( "Режим AFK выключен. Сообщений не было.\r\n"
                        "Используй {y{lRпрослушать ?{lEreplay help{x для справки о том, как просмотреть все недавние сообщения." );
	}

	REMOVE_BIT(pch->comm,COMM_AFK);
	pch->getAttributes( ).eraseAttribute( "afk" );	
    }
    else
    {
	SET_BIT(pch->comm,COMM_AFK);
	
	if (argument[0] != '\0') {
	    pch->getAttributes( ).getAttr<XMLStringAttribute>( "afk" )->setValue( argument );
	    pch->printf("Ты в режиме AFK: {c%s.{x\r\n", argument);
	}
	else
	    pch->send_to("Режим AFK включен.\n\r");
    }
}



