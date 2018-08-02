/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "olcstate.h"
#include "olc.h"
#include "sedit.h"
#include "pcharacter.h"
#include "security.h"
#include "mercdb.h"

/*--------------------------------------------------------------------------
 * OLCCommand
 *-------------------------------------------------------------------------*/
OLCCommand::OLCCommand( const DLString &n ) : name( n )
{
}

const DLString& OLCCommand::getName( ) const
{
    return name;
}

short OLCCommand::getLog( ) const
{
    return LOG_ALWAYS;
}

bool OLCCommand::matches( const DLString &argument ) const
{
    return !argument.empty( ) && argument.strPrefix( name );
}

bool OLCCommand::properOrder( Character * )
{
    return false;
}

bool OLCCommand::dispatch( const InterpretArguments &iargs )
{
    return true;
}

bool OLCCommand::dispatchOrder( const InterpretArguments &iargs )
{
    return false;
}

void OLCCommand::run( Character *ch, const DLString &cArguments )
{
    char args[MAX_STRING_LENGTH];

    strcpy( args, cArguments.c_str( ) );
    run( ch->getPC( ), args );	
}

/*--------------------------------------------------------------------------
 * OLCInterpretLayer 
 *-------------------------------------------------------------------------*/
void
OLCInterpretLayer::putInto( )
{
    interp->put( this, CMDP_FIND, 5 );
}

bool 
OLCInterpretLayer::process( InterpretArguments &iargs )
{
    OLCState::Pointer state;
    Descriptor *d;
    
    if (iargs.ch->is_npc( ))
	return true;
    
    d = iargs.d;

    if (!d || d->handle_input.empty( ))
	return true;
    
    state = d->handle_input.front( ).getDynamicPointer<OLCState>( );

    if (!state)
	return true;
    
    if(iargs.cmdName.empty())
	iargs.cmdName = "show";

    if(iargs.cmdName == "?")
	iargs.cmdName = "olchelp";

    iargs.pCommand = state->findCommand( iargs.ch->getPC( ), iargs.cmdName );

    if (iargs.pCommand)
	iargs.advance( );
    
    return true;
}

/*--------------------------------------------------------------------------
 * OLCState 
 *-------------------------------------------------------------------------*/
OLCState::OLCState() : inSedit(false), strEditor(*this)
{
}

int 
OLCState::handle(Descriptor *d, char *arg)
{
    int rc = 0;
    
    owner = d;
    if(inSedit.getValue( ))
	strEditor.eval(arg);
    else
	rc = InterpretHandler::handle(d, arg);
    owner = 0;

    return rc;
}

void OLCState::prompt( Descriptor *d )
{
    if(inSedit.getValue( ))
	strEditor.prompt(d);
    else
	statePrompt(d);
}

void OLCState::attach( PCharacter *ch ) 
{
    if (ch->desc)
	ch->desc->handle_input.push_front( this );
}

void OLCState::detach( PCharacter *ch ) 
{
    if (!ch->desc)
	return;

    handle_input_t::iterator i;
    handle_input_t &hi = ch->desc->handle_input;

    for(i = hi.begin(); i != hi.end(); i++)
	if(**i == this) {
	    hi.erase(i);
	    return;
	}
}


bool OLCState::can_edit( Character *ch, int vnum )
{
    if (!ch->is_npc( )) {
	XMLAttributeOLC::Pointer attr;
	int sec = ch->getPC( )->getSecurity();
	
	if (sec <= 0)
	    return false;
	else if (sec > 9)
	    return true;
	    
	attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );
	
	if (attr) {
	    XMLAttributeOLC::RangeList::iterator i;
	    for (i = attr->vnums.begin( ); i != attr->vnums.end( ); i++) 
		if (i->minVnum <= vnum && vnum <= i->maxVnum)
		    return true;
	}
    }

    return false;
}

bool OLCState::can_edit( Character *ch, AREA_DATA *pArea )
{
    if (!ch->is_npc( )) {
	XMLAttributeOLC::Pointer attr;
	int a = pArea->min_vnum, b = pArea->max_vnum;
	int sec = ch->getPC( )->getSecurity();
	
	if (sec <= 0)
	    return false;
	else if (sec > 9)
	    return true;
	    
	attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );

	if (attr) {
	    XMLAttributeOLC::RangeList::iterator i;
	    for (i = attr->vnums.begin( ); i != attr->vnums.end( ); i++) 
		if ((a <= i->minVnum && i->maxVnum <= b)
		    || (i->minVnum <= b && b <= i->maxVnum)
		    || (i->minVnum <= a && a <= i->maxVnum))
		{
		    return true;
		}
	}
		
    }

    return false;
}

/* returns corresponding area pointer for mob/room/obj vnum */
AREA_DATA *OLCState::get_vnum_area(int vnum)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next)
	if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
	    return pArea;
	
    return 0;
}


bool
OLCState::sedit(DLString &original)
{
    if(inSedit.getValue( )) {
	ostringstream os;
	strEditor.lines.tostream(os);
	original = os.str( );
	inSedit.setValue( false );
	return true;
    } else {
	strEditor.clear( );
	strEditor.setBuffer( original );
	strEditor.clear_undo( );
	inSedit.setValue( true );
	return false;
    }
}

bool
OLCState::sedit(XMLString &original)
{
    DLString orig = original.getValue( );

    if(!sedit(orig))
	return false;
    
    original.setValue(orig);
    return true;
}

bool
OLCState::sedit(char *&original)
{
    DLString orig = original;
    
    if(!sedit(orig))
	return false;
    
    free_string(original);
    original = str_dup(orig.c_str( ));
    return true;
}

bool
OLCState::xmledit(XMLDocument::Pointer &xml)
{
    ostringstream os;
    if(xml)
	xml->save( os );
    
    DLString buf = os.str( );
    
    if(!sedit(buf))
	return false;

    try {
	XMLDocument::Pointer doc(NEW);
	istringstream is( buf );
	doc->load( is );
	xml = doc;
	return true;
    } catch(const exception &e ) {
	owner->send((DLString("xml parse error: ") + e.what( ) + "\r\n").c_str( ));
    }
    return false;
}

void
OLCState::seditDone( )
{
    if(!owner) {
	LogStream::sendError() << "olc: seditDone: no owner" << endl;
	return;
    }
    
    Character *ch = owner->character;
    if(!ch) {
	LogStream::sendError() << "olc: seditDone: no character" << endl;
	return;
    }

    PCharacter *pch = ch->getPC( );
    
    CommandBase::Pointer cmd;
    
    cmd = findCommand(pch, lastCmd.getValue( ).c_str( ));

    if(!cmd) {
	LogStream::sendError() << "olc: seditDone: command not found to repeat" << endl;
	return;
    }
    
    cmd->run(pch, lastArgs.getValue( ).c_str( ));

    if(inSedit.getValue( )) {
	LogStream::sendError() << "olc: seditDone: still in sedit after command repeat" << endl;
	inSedit.setValue( false );
	return;
    }
    strEditor.clear( );
}


bool OLCState::mapEdit( Properties &map, DLString &args )
{
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args;

    Character *ch = owner->character;
    if(!ch) {
        LogStream::sendError() << "olc: mapEdit: no character" << endl;
        return false;
    }

    if (arg1.empty( )) {
        stc("Syntax: property <name> <value>\n\r"
            "        property <name>\n\r"
            "        property <name> clear\n\r", ch);
        return false;
    }

    if (arg2 == "clear") {
        Properties::iterator p = map.find( arg1 );
        if (p == map.end( )) {
            ptc(ch, "Property '%s' not found.\n\r", arg1.c_str( ));
        } else {
            map.erase( p );
            stc("Property cleared.\n\r", ch);
        }

        return false;
    }

    DLString &property = map[arg1];

    if (arg2.empty( )) {
        if (sedit(property)) {
            stc("Property set.\n\r", ch);
            return true;
        } else
            return false;
    }

    map[arg1] = arg2;
    stc("Property set.\n\r", ch);
    return false;
}
