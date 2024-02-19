/* $Id$
 *
 * ruffina, 2004
 */
#include "channels.h"

#include "class.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "fenia_utils.h"

#include "behavior_utils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "commandmanager.h"
#include "areaquestutils.h"
#include "act.h"

#include "infonet.h"
#include "descriptor.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

bool has_nochannel(Character *);

/*-----------------------------------------------------------------------
 * TellChannel
 *-----------------------------------------------------------------------*/
TellChannel::TellChannel( )
{
}

Character * TellChannel::findListener( Character *ch, const DLString &name ) const
{
    Character *victim = get_char_room(ch, name.c_str());
    if (!victim) 
        victim = get_player_world(ch, name.c_str(), false);

    if (!victim || (victim->is_npc() && victim->in_room != ch->in_room)) {
        ch->pecho( "Ты не находишь этого персонажа.");
        return NULL;
    }

    return victim;
}

static bool mprog_tell( Character *listener, Character *talker, const char *msg )
{
    FENIA_CALL( listener, "Tell", "Cs", talker, msg );
    FENIA_NDX_CALL( listener->getNPC( ), "Tell", "CCs", listener, talker, msg );
    BEHAVIOR_VOID_CALL( listener->getNPC( ), tell, talker, msg );
    return false;
}

void TellChannel::triggers( Character *ch, Character *victim, const DLString &msg ) const
{
    victim->reply = ch;
    PersonalChannel::triggers( ch, victim, msg );    
    mprog_tell( victim, ch, msg.c_str( ) );
}

/*-----------------------------------------------------------------------
 * ReplyChannel
 *-----------------------------------------------------------------------*/
ReplyChannel::ReplyChannel( )
{
}

Character * ReplyChannel::findListener( Character *ch, const DLString &name ) const
{
    Character *victim = ch->reply;
   
    if (!victim)
        ch->pecho( "Ты не находишь этого персонажа." );

    return victim;
}

bool ReplyChannel::parseArguments( Character *ch, const DLString &constArguments,
                                   DLString &msg, DLString &name ) const
{
    msg = constArguments;

    if (msg.empty( )) {
        ch->pecho( msgNoArg );
        return false;
    }

    return true;
}


/*-----------------------------------------------------------------------
 * PageChannel
 *-----------------------------------------------------------------------*/
PageChannel::PageChannel( )
{
}

Character * PageChannel::findListener( Character *ch, const DLString &name ) const
{
    Character *victim;
    
    victim = get_player_world( ch, name.c_str( ), false );

    if (!victim || (victim->is_immortal( ) && !ch->is_immortal( ))) {
        ch->pecho( "Информационное агенство не может найти данного абонента." );
        return NULL;
    }

    return victim;
}

void PageChannel::run( Character *ch, const DLString &constArguments ) 
{
    DLString arguments;

    arguments = constArguments;

    if (!get_pager( ch )) {
        ch->pecho( "Тебе определенно нужен хрустальный шар и то, что внутри него." );
        return;
    }

    if (arguments.getOneArgument( ) == "all")
        GlobalChannel::run( ch, arguments );
    else
        PersonalChannel::run( ch, constArguments );
}

bool PageChannel::isPersonalListener( Character *ch, Character *victim, const DLString &msg ) const
{
    if (!get_pager( victim )) {
        oldact_p("У $C2 нет хрустального шара.",ch,0,victim,TO_CHAR, position);
        return false;
    }

    return PersonalChannel::isPersonalListener( ch, victim, msg );
}

bool PageChannel::isGlobalListener( Character *ch, Character *victim ) const
{
    if (!GlobalChannel::isGlobalListener( ch, victim ))
        return false;

    if (!get_pager( victim ))
        return false;

    return true;
}

DLString PageChannel::outputVict( Character *ch, Character *victim, 
                              const DLString &format, const DLString &msg ) const
{
    DLString message = fmt( victim, format.c_str( ), ch, msg.c_str( ), victim, get_pager( victim ) );
    return message;
}

void PageChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    PersonalChannel::postOutput( outputTo, message );
}


/*-----------------------------------------------------------------------
 * SpeechChannel
 *-----------------------------------------------------------------------*/
SpeechChannel::SpeechChannel( )
{
}

static void rprog_speech( Room *room, Character *ch, const char *msg )
{
    FENIA_VOID_CALL( room, "Speech", "Cs", ch, msg );
}

static bool mprog_speech( Character *rch, Character *talker, const char *msg )
{
    if (IS_AWAKE(rch)) {
        aquest_trigger(rch, talker, "Speech", "CCs", rch, talker, msg);
        FENIA_CALL( rch, "Speech", "Cs", talker, msg );
        FENIA_NDX_CALL( rch->getNPC( ), "Speech", "CCs", rch, talker, msg );
        BEHAVIOR_VOID_CALL( rch->getNPC( ), speech, talker, msg );
    }
    return false;
}

static bool oprog_speech( Object *obj, Character *talker, const char *msg )
{
    aquest_trigger(obj, talker, "Speech", "OCs", obj, talker, msg);
    FENIA_CALL( obj, "Speech", "Cs", talker, msg );
    FENIA_NDX_CALL( obj, "Speech", "OCs", obj, talker, msg );
    BEHAVIOR_VOID_CALL( obj, speech, talker, msg );
    return false;
}

void SpeechChannel::triggers( Character *ch, const DLString &msg ) const
{
    Character *rch;
    Object *obj, *obj_next;
    const char *cmsg = msg.c_str( );
    
    if (!ch->is_npc( ) || IS_CHARMED(ch))
        RoomChannel::triggers( ch, msg );

    for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room) {
        if (rch != ch)
            mprog_speech( rch, ch, cmsg );

        for (obj = rch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;
            oprog_speech( obj, ch, cmsg );
        }
    }

    for (obj = ch->in_room->contents; obj != 0; obj = obj_next) {
        obj_next = obj->next_content;
        oprog_speech( obj, ch, cmsg );
    }

    rprog_speech( ch->in_room, ch, cmsg );

    gprog("onSpeech", "Cs", ch, cmsg);
}

/*-----------------------------------------------------------------------
 * EmoteChannel
 *-----------------------------------------------------------------------*/
EmoteChannel::EmoteChannel( )
{
}

void EmoteChannel::triggers( Character *ch, const DLString &txt ) const
{
    if (!ch->is_npc( ) || IS_CHARMED(ch))
        RoomChannel::triggers( ch, txt );

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        FENIA_VOID_CALL( rch, "Emote", "Cs", ch, txt.c_str( ) );
        FENIA_NDX_VOID_CALL( rch->getNPC( ), "Emote", "CCs", rch, ch, txt.c_str( ) );
    }
}

bool EmoteChannel::canTalkGlobally( Character *ch ) const
{
    if (!RoomChannel::canTalkGlobally( ch ))
        return false;

    if (IS_SET(ch->comm, COMM_NOEMOTE)) {
        ch->pecho( "Боги запретили тебе волноваться." );
        return false;
    }

    return true;
}

/*-----------------------------------------------------------------------
 * ChannelsCommand
 *-----------------------------------------------------------------------*/
void ChannelsCommand::initialization( )
{
    Channels::iterator c;
    
    Class::regMoc<SpeechChannel>( );
    Class::regMoc<EmoteChannel>( );
    Class::regMoc<TellChannel>( );
    Class::regMoc<ReplyChannel>( );
    Class::regMoc<PageChannel>( );
    CommandPlugin::initialization( );

    for (c = channels.begin( ); c != channels.end( ); c++) 
        commandManager->registrate( *c );
}

void ChannelsCommand::destruction( )
{
    Channels::iterator c;
    
    for (c = channels.begin( ); c != channels.end( ); c++) 
        commandManager->unregistrate( *c );

    CommandPlugin::destruction( );
    Class::unregMoc<SpeechChannel>( );
    Class::unregMoc<EmoteChannel>( );
    Class::unregMoc<TellChannel>( );
    Class::unregMoc<ReplyChannel>( );
    Class::unregMoc<PageChannel>( );
}

COMMAND(ChannelsCommand, "channels")
{
    ostringstream buf;
    Channels::iterator c;
    bool rus = ch->getConfig( ).rucommands;
    
    buf << "   канал     статус  " << endl
        << "---------------------" << endl;
    
    for (c = channels.begin( ); c != channels.end( ); c++) 
        if ((*c)->getOff( ) && (*c)->canHear( ch ))
            buf << dlprintf( "%-12s %s.", 
                             rus ? (*c)->getRussianName( ).c_str( )
                                 : (*c)->getName( ).c_str( ),
                             (IS_SET(ch->comm, (*c)->getOff( )) 
                                 ? "ВЫКЛ" 
                                 : "ВКЛ")
                            )
                << endl;
    
    
    if (ch->getClan( )->hasChannel( )) 
        buf << (rus ? "кланканал     " : "clantalk (cb) ")
            << (!IS_SET(ch->comm, COMM_NOCB) ? "ВКЛ." : "ВЫКЛ.")
            << endl;

    buf <<     (rus ? "аукцион       " : "auction       ")
        <<     (!IS_SET(ch->comm, COMM_NOAUCTION) ? "ВКЛ." : "ВЫКЛ.")
        <<     endl
        <<     (rus ? "глухота       " : "tells         ")
        <<     (!IS_SET(ch->comm, COMM_DEAF) ? "ВКЛ." : "ВЫКЛ.")
        <<     endl
        <<     (rus ? "тишина        " : "quiet mode    ")
        <<     (IS_SET(ch->comm, COMM_QUIET) ? "ВКЛ." : "ВЫКЛ.")
        <<     endl;

    if (IS_SET(ch->comm, COMM_SNOOP_PROOF))
        buf << "Ты защищен от подслушивания." << endl;

    if (IS_SET(ch->comm, COMM_NOTELL))
        buf << "Ты не можешь использовать {lRкоманду 'говорить'{Etell{lx." << endl;

    if (has_nochannel( ch ))
        buf << "Ты не можешь использовать каналы общения." << endl;

    if (IS_SET(ch->comm, COMM_NOEMOTE))
        buf << "Ты не можешь выражать эмоции." << endl;

    ch->send_to( buf );
}



