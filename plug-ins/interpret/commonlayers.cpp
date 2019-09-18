/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>
#include "interpretlayer.h"
#include "commandinterpreter.h"
#include "commandbase.h"
#include "translit.h"

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

class SayWhatInterpretLayer : public InterpretLayer {
public:    
    virtual void putInto( )
    {
        interp->put( this, CMDP_FIND, CMD_PRIO_LAST );
    }
    
    virtual bool process( InterpretArguments &iargs )
    {
        if (iargs.pCommand)
            return true;

        if (iargs.cmdName.empty( ))
            return false;

        if (!iargs.ch->desc || iargs.ch->desc->connected != CON_PLAYING)
            return false;
            
        ostringstream buf; 
        int total_hints = iargs.hints1.size() + iargs.hints2.size() + iargs.translit.size();

        if (total_hints == 0) {
            iargs.ch->send_to( "Что? Для справки наберите {y{hc{lRкоманды{lEcommands{x или {y{hc{lRсправка{lEhelp{x.\r\n" );
            return false;
        }

        buf << "Возможно, имелось в виду:" << endl;
       
        if (total_hints <= 3) {
            const DLString &args = iargs.cmdArgs;
            DLString kuzdnArgs = translit(args);
            show_hint_and_argument(buf, iargs.translit, kuzdnArgs);
            show_hint_and_argument(buf, iargs.hints1, args);
            show_hint_and_argument(buf, iargs.hints2, args);
        } else {
            int hint_cnt = 0; 
            show_hint_columns(hint_cnt, buf, iargs.translit);
            show_hint_columns(hint_cnt, buf, iargs.hints1);
            show_hint_columns(hint_cnt, buf, iargs.hints2);
            if (hint_cnt % 2 != 0)
                buf << endl;
        }

        buf << "Для справки наберите {y{hc{lRкоманды{lEcommands{x или {y{hc{lRсправка{lEhelp{x." << endl;
        iargs.ch->send_to(buf);
        return false;
    }

private:
    void show_hint_and_argument(ostringstream &buf, const StringList &hints, const DLString &args)
    {
        for (StringList::const_iterator h = hints.begin(); h != hints.end(); h++) {
            buf << "    {hc{y" << *h;
            if (!args.empty())
                buf  << "{w " << args;
            buf  << "{x" << endl;
        }
    }

    void show_hint_columns(int &hint_cnt, ostringstream &buf, const StringList &hints)
    {
        for (StringList::const_iterator h = hints.begin(); h != hints.end(); h++) {
            DLString str(*h);
            str << "{x";

            buf << "    {hc{y" << setiosflags(ios::left) << setw(20) << str <<  resetiosflags(ios::left) << "{x";
            if (++hint_cnt % 2 == 0)
                buf << endl;
        }
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
        Plugin::registerPlugin<SayWhatInterpretLayer>( ppl );

        return ppl;
    }
}
