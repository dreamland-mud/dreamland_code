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

#include "wiznet.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

GSN(perception);
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

const char *dir_name[] = {"N","E","S","W","U","D"};
const char *dir_name_small[] = {"n","e","s","w","u","d"};
const char *ru_dir_name[] = {"С","В","Ю","З","П","О"};
const char *ru_dir_name_small[] = {"с","в","ю","з","п","о"};

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

static bool oprog_command( Object *obj, Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    FENIA_CALL(obj, "Command", "Css", actor, cmdName.c_str( ), cmdArgs.c_str( ));
    FENIA_NDX_CALL(obj, "Command", "OCss", obj, actor, cmdName.c_str( ), cmdArgs.c_str( ));
    BEHAVIOR_CALL(obj, command, actor, cmdName, cmdArgs); 
    return false;
}

static bool omprog_command( Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    Character *ch;
    Object *obj;
    
    for (ch = actor->in_room->people; ch; ch = ch->next_in_room) {
	if (mprog_command( ch, actor, cmdName, cmdArgs ))
	    return true;

	for (obj = ch->carrying; obj; obj = obj->next_content)
	    if (oprog_command( obj, actor, cmdName, cmdArgs ))
		return true;
    }

    return false;
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

    CommandInterpreter::getThis( )->run( iargs );
    
    if (!iargs.pCommand) {
	if (!iargs.cmdName.empty( ))
	    iargs.ch->send_to( "Что?\r\n" );

	return 0;
    }

    if (!iargs.pCommand->dispatch( iargs )) 
	return 0;
    
    iargs.cmdName = iargs.pCommand->getName( );

    if (rprog_command( iargs.ch->in_room, iargs.ch, iargs.cmdName, iargs.cmdArgs ))
	return 0;

    if (omprog_command( iargs.ch, iargs.cmdName, iargs.cmdArgs ))
	return 0;

    iargs.pCommand->run( iargs.ch, iargs.cmdArgs );
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

    if (ch->position != POS_FIGHTING)
	str = ch->prompt.c_str( );
    else
	str = ch->batle_prompt.c_str( );

    if ( !str || !*str ) {
	ch->printf( "<%dhp %dm %dmv> %s", 
		ch->hit.getValue( ), ch->mana.getValue( ), ch->move.getValue( ), ch->prefix );
	return;
    }

    if (IS_SET(ch->comm,COMM_AFK)) {
	ch->send_to("{C<AFK>{x ");
	return;
    }

    while( *str ) {
	ostringstream doors;
	Character *victim;
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
	    ruexits = ch->getPC( ) && ch->getPC( )->getConfig( )->ruexits;

	    for (int door = 0; door < DIR_SOMEWHERE; door++) {
		EXIT_DATA *pexit = ch->in_room->exit[door];
		
		if (!pexit || !ch->can_see( pexit ))
		    continue;

		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
		    if (number_percent( ) < gsn_perception->getEffective( ch ))
			doors << (ruexits ? ru_dir_name_small[door] : dir_name_small[door]);
		} else {
		    doors << (ruexits ? ru_dir_name[door] : dir_name[door]);
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

	case 'S' :
	    out << (ch->getSex( ) == SEX_MALE ? "Male":(!ch->getSex( ) ? "None":"Female"));
	    break;

	case 'y' :
	    if (ch->hit >= 0)
		out << HEALTH(ch) << "%";
	    else
		out << "BAD!!";
	    break;

	case 'o' :
	    if ((victim = ch->fighting) != 0)
	    {
		if (victim->hit >= 0)
		    out << HEALTH(victim) << "%";
		else
		    out << "BAD!!";
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
		
	case 'g' :
	    out << ch->gold;
	    break;

	case 's' :
	    out << ch->silver;
	    break;

	case 'a' :
	    out << align_table.name( ALIGNMENT(ch) );
	    break;

	case 'r' :
	    if (ch->in_room != 0)
		if (ch->getConfig( )->holy 
		    || ch->is_vampire( ) 
		    || IS_GHOST(ch) || IS_DEATH_TIME(ch) 
		    || (!ch->in_room->isDark( ) && !IS_AFFECTED(ch, AFF_BLIND))) 
		{
		    out << ch->in_room->name;
		}
		else
		    out << "темнота";
	    else
		out << " ";
	    break;

	case 'R' :
	    if ( ch->is_immortal() && ch->in_room != 0 )
		out << ch->in_room->vnum;
	    else
		out << " ";
	    break;

	case 'z' :
	    if ( ch->is_immortal() && ch->in_room != 0 )
		out << ch->in_room->area->name;
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
            ch->in_room && ch->in_room->area && ch->in_room->area->area_file && ch->in_room->area->area_file->file_name ?
                    ch->in_room->area->area_file->file_name : "");

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

    webPrompt(d, ch);

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
	buf << "{Bимеет несколько царапин.{x";
    else if( percent >= 75 )
	buf << "{Bимеет несколько небольших синяков и царапин.{x";
    else if( percent >= 50 )
	buf << "{Gимеет довольно много ран.{x";
    else if( percent >= 30 )
	buf << "{Yимеет несколько больших, опасных ран и царапин.{x";
    else if( percent >= 15 )
	buf << "{Mвыглядит сильно поврежденн%1$Gым|ым|ой.{x";
    else if( percent >= 0 )
	buf << "{Rв ужасном состоянии.{x";
    else
	buf << "{Rистекает кровью.{x";
    
    ch->pecho( buf.str( ).c_str( ), victim ); 
}

void InterpretHandler::close( Descriptor *d )
{
    Character *ch = d->character;

    if (!ch)
	return;
	
    act( "$c1 потеря$gло|л|ла связь с этим миром.", ch, 0, 0, TO_ROOM );
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

