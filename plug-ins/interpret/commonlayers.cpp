/* $Id$
 *
 * ruffina, 2004
 */
#include "interpretlayer.h"
#include "commandinterpreter.h"
#include "commandbase.h"

#include "lastlogstream.h"
#include "logstream.h"
#include "so.h"
#include "dlfileop.h"

#include "pcharacter.h"
#include "room.h"
#include "dreamland.h"
#include "descriptor.h"
#include "wiznet.h"
#include "merc.h"
#include "def.h"

class GrabWordInterpretLayer : public InterpretLayer {
public:

    virtual void putInto( )
    {
	interp->put( this, CMDP_GRAB_WORD, 10 );
    }

    virtual bool process( InterpretArguments &iargs )
    {
	iargs.splitLine( );
	return true;
    }
};

class LogInputInterpretLayer : public InterpretLayer {
public:
    virtual void putInto( )
    {
	interp->put( this, CMDP_LOG_INPUT, 10 );
    }

    virtual bool process( InterpretArguments &iargs )
    {
	LastLogStream::send( ) 
	    << iargs.ch->getNameP( ) << ": " << iargs.line << endl;

	if (dreamland->hasOption( DL_LOG_IMM ) && iargs.ch->is_immortal( )) {
	    DLFileAppend( dreamland->getBasePath( ), dreamland->getImmLogFile( ) )
	         .printf(
		        "[%s]:[%s] [%d] %s\n",
		         Date::getTimeAsString( dreamland->getCurrentTime( ) ).c_str( ),
			 iargs.ch->getNameP( ),
			 iargs.ch->in_room->vnum,
			 iargs.line.c_str( ) );
	}

	return true;
    }
};


class LogCommandInterpretLayer : public InterpretLayer {
public:
    virtual void putInto( )
    {
	interp->put( this, CMDP_LOG_CMD, 10 );
    }

    virtual bool process( InterpretArguments &iargs )
    {
	if (!iargs.line.empty( )) {
	    DLString line = iargs.cmdName + " " + iargs.cmdArgs;
	    logCommand( iargs.ch, iargs.pCommand->getLog( ), line );
	}
	
	return true;
    }

private:
    void logCommand( Character *ch, int log, const DLString &line )
    {
	if (log == LOG_NEVER) 
	    return;

	if ((!ch->is_npc( ) && IS_SET( ch->act, PLR_LOG ))
	      || dreamland->hasOption( DL_LOG_ALL ) 
	      || log == LOG_ALWAYS)
	{
	    wiznet( WIZ_SECURE, 0, ch->get_trust(), "Log %C1: %s [%d]",
		    ch, line.c_str( ), ch->in_room->vnum );
	}
    }
};

class FixStringInterpretLayer : public InterpretLayer {
public:    
    virtual void putInto( )
    {
	interp->put( this, CMDP_FIND, CMD_PRIO_FIRST );
    }
    
    virtual bool process( InterpretArguments &iargs )
    {
	iargs.cmdName.stripLeftWhiteSpace( );

	if (iargs.cmdName.empty( ))
	    return false;

	iargs.cmdName.substitute( '~', '-' );
	iargs.cmdArgs.substitute( '~', '-' );
	return true;
    }
};

class FindCommandInterpretLayer : public InterpretLayer {
public:    
    virtual void putInto( )
    {
	interp->put( this, CMDP_FIND, CMD_PRIO_LAST );
    }
    
    virtual bool process( InterpretArguments &iargs )
    {
	return false;
    }
};


extern "C"
{
    SO::PluginList initialize_commonlayers( )
    {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<GrabWordInterpretLayer>( ppl );
	Plugin::registerPlugin<FixStringInterpretLayer>( ppl );
	Plugin::registerPlugin<LogInputInterpretLayer>( ppl );
	Plugin::registerPlugin<LogCommandInterpretLayer>( ppl );
	Plugin::registerPlugin<FindCommandInterpretLayer>( ppl );

	return ppl;
    }
}
