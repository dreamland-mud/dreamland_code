/* $Id$
 *
 * ruffina, 2004
 */
#include "json/json.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "char.h"

#include "ban.h"
#include "colour.h"
#include "telnet.h"
#include "interprethandler.h"
#include "webprompt.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "commandinterpreter.h"
#include "commandbase.h"
#include "playerattributes.h"
#include "objectbehavior.h"
#include "behavior_utils.h"
#include "room.h"
#include "skillreference.h"
#include "door_utils.h"
#include "websocketrpc.h"
#include "wiznet.h"
#include "act.h"

#include "def.h"

LIQ(none);

const        char         go_ahead_str        [] = { (char)IAC, (char)GA, '\0' };

const char * sunlight_ru [4] = { "темно", "светает", "светло", "сумерки" };    

static bool rprog_command( Room *room, Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    FENIA_CALL(room, "Command", "Css", actor, cmdName.c_str( ), cmdArgs.c_str( ));
    BEHAVIOR_CALL(room, command, actor, cmdName, cmdArgs);
    return false;
}

static bool mprog_command( Character *ch, Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    FENIA_CALL(ch, "Command", "Css", actor, cmdName.c_str( ), cmdArgs.c_str( ));
    FENIA_NDX_CALL(ch->getNPC( ), "Command", "CCss", ch, actor, cmdName.c_str( ), cmdArgs.c_str( ));
    BEHAVIOR_CALL(ch->getNPC( ), command, actor, cmdName, cmdArgs);
    return false;
}

static bool mprog_input( Character *ch, const DLString &line )
{
    FENIA_CALL(ch, "Input", "s", line.c_str());
    FENIA_NDX_CALL(ch->getNPC( ), "Input", "Cs", ch, line.c_str());
    return false;
}

static bool oprog_command( Object *obj, Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    FENIA_CALL(obj, "Command", "Css", actor, cmdName.c_str( ), cmdArgs.c_str( ));
    FENIA_NDX_CALL(obj, "Command", "OCss", obj, actor, cmdName.c_str( ), cmdArgs.c_str( ));
    BEHAVIOR_CALL(obj, command, actor, cmdName, cmdArgs);
    return false;
}

static bool omprog_command( Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    for (Object *obj = actor->in_room->contents; obj; obj = obj->next_content)
        if (oprog_command( obj, actor, cmdName, cmdArgs ))
            return true;

    for (Character *ch = actor->in_room->people; ch; ch = ch->next_in_room) {
        if (mprog_command( ch, actor, cmdName, cmdArgs ))
            return true;

        for (Object *obj = ch->carrying; obj; obj = obj->next_content)
            if (oprog_command( obj, actor, cmdName, cmdArgs ))
                return true;
    }

    return false;
}

static bool can_see_room_details(Character *ch)
{
    if (ch->getConfig( ).holy
        || ch->is_vampire( )
        || IS_GHOST(ch) || IS_DEATH_TIME(ch)
        || (ch->in_room->isDark( ) && IS_AFFECTED(ch, AFF_INFRARED))
        || (!ch->in_room->isDark( ) && !IS_AFFECTED(ch, AFF_BLIND)))
    {
        return true;
    }

    return false;
}

static bool can_see_sunlight(Character *ch)
{
    return !IS_SET(ch->in_room->room_flags, ROOM_INDOORS);
}

static char sector_type_color(int type)
{
    switch (type) {
        case SECT_CITY:
        case SECT_INSIDE:
            return 'W';
        case SECT_AIR: 
            return 'C';
        case SECT_FOREST: 
            return 'g';
        case SECT_FIELD:
        case SECT_HILLS: 
            return 'G';
        case SECT_MOUNTAIN: 
            return 'y';
        case SECT_WATER_SWIM: 
            return 'B';
        case SECT_WATER_NOSWIM:
        case SECT_UNDERWATER: 
            return 'b';            
        case SECT_DESERT: 
            return 'Y';
        default: 
            return 'D';
    }
}

int
InterpretHandler::handle(Descriptor *d, char *arg)
{
    InterpretArguments iargs;
    static int phases[] = {
        CMDP_PREFIX,
        CMDP_SUBST_ALIAS,
        CMDP_LOG_INPUT,
        CMDP_GRAB_WORD,
        CMDP_FIND,
        CMDP_LOG_CMD,
        0
    };

    iargs.d = d;
    iargs.ch = d->character;
    iargs.line = arg;
    iargs.phases = phases;

    if (mprog_input(iargs.ch, iargs.line))
        return 0;

    CommandInterpreter::getThis( )->run( iargs );

    if (!iargs.pCommand) 
        return 0;

    if (iargs.pCommand->dispatch( iargs ) != RC_DISPATCH_OK)
        return 0;

    iargs.cmdName = iargs.pCommand->getName( );

    if (rprog_command( iargs.ch->in_room, iargs.ch, iargs.cmdName, iargs.cmdArgs ))
        return 0;

    if (omprog_command( iargs.ch, iargs.cmdName, iargs.cmdArgs ))
        return 0;

    iargs.pCommand->entryPoint( iargs.ch, iargs.cmdArgs );
    return 0;
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * bust
 */
void InterpretHandler::normalPrompt( Character *ch )
{
    ostringstream out;
    const char *str;
    PCharacter *pch = ch->getPC( );
    Character *victim = ch->fighting;

    if (ch->position == POS_FIGHTING || victim != 0)
        str = ch->batle_prompt.c_str( );        
    else
        str = ch->prompt.c_str( );

    if ( !str || !*str ) {
        ch->printf( "<%dhp %dm %dmv> %s",
                ch->hit.getValue( ), ch->mana.getValue( ), ch->move.getValue( ), ch->prefix );
        return;
    }

    if (IS_SET(ch->comm,COMM_AFK)) {
        ch->send_to("{C<{leAFK{lrАФК{lx>{x ");
        return;
    }

    while( *str ) {
        ostringstream doors;
        bool ruexits = false;
        bool handled = false;

        if ( *str != '%' ) {
            out << *str++;
            continue;
        }

        if (*++str == '\0')
            break;

        /*
         * prompt-handling player attributes
         */
        if (!ch->is_npc( )
            && pch->getAttributes( ).handleEvent( PromptArguments( pch, *str, out ) ))
        {
            handled = true;
        }

        /*
         * prompt-handling object behaviors
         */
        if (!handled)
            for (Object *obj = ch->carrying; obj; obj = obj->next_content)
                if (obj->behavior && obj->behavior->prompt( ch, *str, out )) {
                    handled = true;
                    break;
                }

        if (handled) {
            ++str;
            continue;
        }

        switch( *str )
        {
        default :
            out << " ";
            break;

        case 'd':
        case 'e':
            ruexits = ch->getPC( ) && ch->getPC( )->getConfig( ).ruexits;

            for (int door = 0; door < DIR_SOMEWHERE; door++) {
                EXIT_DATA *pexit = ch->in_room->exit[door];

                if (!pexit || !ch->can_see( pexit ))
                    continue;

                if (IS_SET(pexit->exit_info, EX_CLOSED)) {
                    doors << (ruexits ? ru_dir_name_small[door] : dir_name_small[door]);
                } else {
                    doors << (ruexits ? ru_dir_name_big[door] : dir_name_big[door]);
                }
            }
            if (doors.str( ).empty( ))
                out << (ruexits ? "нет" : "none");
            else
                out << doors.str( );
            break;

        case 'c' :
            out << endl;
            break;

        case 'n' :
            out << ch->seeName( ch );
            break;

        case 'y' :
            if (ch->hit >= 0)
                out << HEALTH(ch) << "%";
            else
                out << "ПРИ СМЕРТИ!";
            break;

        case 'o' :
            if (victim != 0)
            {
                if (victim->hit >= 0)
                    out << HEALTH(victim) << "%";
                else
                    out << "ПРИ СМЕРТИ!";
            }
            else
                out << "нет";
            break;

        case 'h' :
            out << ch->hit;
            break;

        case 'H' :
            out << ch->max_hit;
            break;

        case 'm' :
            out << ch->mana;
            break;

        case 'M' :
            out << ch->max_mana;
            break;

        case 'v' :
            out << ch->move;
            break;

        case 'V' :
            out << ch->max_move;
            break;

        case 'x' :
            out << ch->exp;
            break;

        case 'X' :
            if (!ch->is_npc())
                out << ch->getPC()->getExpToLevel( );
            else
                out << " ";
            break;

        case 'l':
            out << ch->getRealLevel( );
            break;

        case 'L':
            if (ch->in_room && can_see_sunlight(ch)) {
                out << sunlight_ru[weather_info.sunlight];
            } else {
                out << "-";
            }
            break;

        case 'g' :
            out << ch->gold;
            break;

        case 's' :
            out << ch->silver;
            break;

        case 'S':
            if (ch->in_room && can_see_room_details(ch)) {
                int sector = ch->in_room->getSectorType();
                LiquidReference &liq = ch->in_room->getLiquid();

                bool indoors = IS_SET(ch->in_room->room_flags, ROOM_INDOORS);
                out << (indoors ? "(" : "")
                    << "{" << sector_type_color(sector) <<  sector_table.message(sector);
                    
                if (liq_none != liq)
                    out << "{D|{x" << liq->getShortDescr().ruscase('1');

                out << "{w"
                    << (indoors ? ")" : "");

            } else {
                out << "";
            }
            break;

        case 'r' :
            if (ch->in_room != 0) {
                if (can_see_room_details(ch)) {
                    out << ch->in_room->getName();
                } else {
                    out << "темнота";
                }
            } else {
                out << " ";
            }
            break;

        case 'W' :
            out << ch->wait << "W" << ch->daze << "D";
            break;

        case 'R' :
            if ( ch->is_immortal() && ch->in_room != 0 )
                out << ch->in_room->vnum;
            else
                out << " ";
            break;

        case 'z' :
            if ( ch->is_immortal() && ch->in_room != 0 )
                out << ch->in_room->areaName();
            else
                out << " ";
            break;

        case '%' :
            out << "%%";
            break;
        }

        ++str;
    }

    if (ch->prefix[0] != '\0')
        out << ch->prefix;

    ch->send_to( out.str( ) );
}


void
InterpretHandler::webPrompt(Descriptor *d, Character *ch)
{
    Json::Value prompt;
    prompt["command"] = "prompt";
    prompt["args"][0]["hit"] = ch->hit.getValue();
    prompt["args"][0]["max_hit"] = ch->max_hit.getValue();
    prompt["args"][0]["mana"] = ch->mana.getValue();
    prompt["args"][0]["max_mana"] = ch->max_mana.getValue();
    prompt["args"][0]["move"] = ch->move.getValue();
    prompt["args"][0]["max_move"] = ch->max_move.getValue();
    prompt["args"][0]["vnum"] = ch->in_room ? ch->in_room->vnum : 0;
    prompt["args"][0]["area"] = DLString(
            ch->in_room ? ch->in_room->areaIndex()->area_file->file_name : "");

    // Call various web prompt handlers to write out complex stuff defined in other plugins,
    // such as group information, weather, time etc.
    WebPromptManager::getThis( )->handle( d, ch, prompt );

    d->writeWSCommand(prompt);
}

void
InterpretHandler::prompt(Descriptor *d)
{
    Character *ch;

    if (d->connected != CON_PLAYING)
        return;

    /* XXX after each output, buffer overflow can occur.
     * we must throw an exception and not just close the descriptor
     */
    ch = d->character->getPC( );

    battlePrompt( ch );

    if (is_websock(d))
        webPrompt(d, d->character);

    if (!IS_SET( ch->comm, COMM_COMPACT ))
        d->send("\n\r");

    if (IS_SET( ch->comm, COMM_PROMPT ))
        normalPrompt( ch );

    if (IS_SET( ch->comm, COMM_TELNET_GA ))
        d->send(go_ahead_str);
}

/*
 * battle prompt
 */
void InterpretHandler::battlePrompt( Character *ch )
{
    Character *victim;
    int percent;
    ostringstream buf;

    victim = ch->fighting;

    if (!victim || !ch->can_see( victim ))
        return;

    if( victim->max_hit > 0 )
        percent = HEALTH(victim);
    else
        percent = -1;

    buf << "%1$^C1 ";

    if( percent >= 100 )
        buf << "{Cв прекрасном состоянии.{x";
    else if( percent >= 90 )
        buf << "{Bиме%1$nет|ют несколько царапин.{x";
    else if( percent >= 75 )
        buf << "{Bиме%1$nет|ют несколько небольших синяков и царапин.{x";
    else if( percent >= 50 )
        buf << "{Gиме%1$nет|ют довольно много ран.{x";
    else if( percent >= 30 )
        buf << "{Yиме%1$nет|ют несколько больших, опасных ран и царапин.{x";
    else if( percent >= 15 )
        buf << "{Mвыгляд%1$nит|ят сильно поврежденн%1$Gым|ым|ой|ыми.{x";
    else if( percent >= 0 )
        buf << "{Rв ужасном состоянии.{x";
    else
        buf << "{Rистека%1$nет|ют кровью.{x";

    ch->pecho( buf.str( ).c_str( ), victim );
}

void InterpretHandler::close( Descriptor *d )
{
    Character *ch = d->character;

    if (!ch)
        return;

    oldact("$c1 потеря$gло|л|ла связь с этим миром.", ch, 0, 0, TO_ROOM );
    wiznet( WIZ_LINKS, 0, ch->get_trust( ), "Net death has claimed %C1.", ch );
    ch->desc = 0;
}

void InterpretHandler::init( Descriptor *d )
{
    d->connected = CON_PLAYING;
    d->handle_input.clear( );
    d->handle_input.push_front( new InterpretHandler );
    d->incomm[0] = '\0';
    d->inbuf[0] = '\0';
    Descriptor::updateMaxOnline( );
}


