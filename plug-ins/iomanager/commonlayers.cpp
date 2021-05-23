/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>
#include "interpretlayer.h"
#include "commandinterpreter.h"
#include "commandbase.h"
#include "translit.h"
#include "json/json.h"
#include "lasthost.h"
#include "websocketrpc.h"
#include "lastlogstream.h"
#include "logstream.h"
#include "so.h"
#include "dlfileop.h"
#include "profiler.h"
#include "plugininitializer.h"

#include "pcharacter.h"
#include "room.h"
#include "dreamland.h"
#include "descriptor.h"
#include "wiznet.h"
#include "merc.h"
#include "def.h"

const char *ttype_name( int ttype );

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
            << iargs.ch->getNameC() << ": " << iargs.line << endl;
        return true;
    }
};

/** Return true if logging is enabled for this character and command. */
static bool can_log_input(InterpretArguments &iargs) 
{
    // Hide personal commands from logging.
    if (iargs.pCommand && iargs.pCommand->getLog() == LOG_NEVER)        
        return false;

    if (iargs.ch->is_npc())
        return false;

    // Individual logging.
    if (IS_SET( iargs.ch->act, PLR_LOG ))
        return true;

    // Commands that always need to be logged, such as 'clan'.
    if (iargs.pCommand && iargs.pCommand->getLog() == LOG_ALWAYS)
        return true;

    // Exclude immortals from common log unless explicitly turned on.
    if (iargs.ch->is_immortal() && !dreamland->hasOption(DL_LOG_IMM))
        return false;

    // Log everyone to a separate file.
    if (dreamland->hasOption( DL_LOG_ALL ))
        return true;

    return false;
}

/** Append one entry to the common log file, for matched or missing command. */
static bool create_log_entry(InterpretArguments &iargs)
{
    Json::Value log;
    log["time"] = Date::getTimeAsString(dreamland->getCurrentTime());

    log["ch"]["id"] = iargs.ch->getID();
    log["ch"]["lvl"] = iargs.ch->getLevel();
    log["ch"]["remort"] = (int)iargs.ch->getPC()->getRemorts().size();
    log["ch"]["room"] = iargs.ch->in_room->vnum;

    DLString defaultPrompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг {W%X{xоп Вых:{g%d{x>%c";
    if (!IS_SET(iargs.ch->comm, COMM_PROMPT))
        log["cfg"]["prompt"] = "off";
    else if (iargs.ch->prompt != defaultPrompt)
        log["cfg"]["prompt"] = "custom";
    else
        log["cfg"]["prompt"] = "default";

    log["cfg"]["web"] = is_websock(iargs.ch);
    log["cfg"]["blind"] = (bool)(IS_SET(iargs.ch->getPC()->config, CONFIG_SCREENREADER));
    log["cfg"]["mudlet"] = iargs.ch->desc && iargs.ch->desc->telnet.ttype != 0;
    log["cfg"]["nanny"] = (iargs.ch->desc && iargs.ch->desc->connected != CON_PLAYING);
    log["cfg"]["brief"] = (bool)(IS_SET(iargs.ch->comm, COMM_BRIEF));

    log["i"]["raw"] = iargs.line;
    log["i"]["name"] = iargs.cmdName;    
    log["i"]["args"] = iargs.cmdArgs;
    if (iargs.pCommand) {
        log["i"]["cmd"] = iargs.pCommand->getName();
    } else {
        log["i"]["cmd"] = "none";
    }

    DLString ip = iargs.ch->getPC()->getLastAccessHost();
    bool unique = XMLAttributeLastHost::isUnique(iargs.ch->getName(), ip);
    log["ip"] = unique ? "new" : "old";

    try {
        Json::FastWriter serializer;
        DLFileAppend(dreamland->getBasePath(), dreamland->getImmLogFile())
            .write(serializer.write(log));
    } catch (const ExceptionDBIO &ex) {
        LogStream::sendError() << ex.what() << endl;
        return false;
    }

    return true;
}

class LogCommandInterpretLayer : public InterpretLayer {
public:
    virtual void putInto( )
    {
        interp->put( this, CMDP_LOG_CMD, 10 );
    }

    virtual bool process( InterpretArguments &iargs )
    {
//        ProfilerBlock p("log command");

        if (can_log_input(iargs))
            create_log_entry(iargs);

        return true;
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

        if (can_log_input(iargs))
            create_log_entry(iargs);

        if (!iargs.ch->desc || iargs.ch->desc->connected != CON_PLAYING)
            return false;
            
        ostringstream buf; 
        int total_hints = iargs.hints1.size() + iargs.hints2.size() + iargs.translit.size();

        if (total_hints == 0) {
            iargs.ch->pecho("Что? Для справки наберите {y{hc{lRкоманды{lEcommands{x или {y{hc{lRсправка{lEhelp{x.");
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


PluginInitializer<GrabWordInterpretLayer> initGrabWord;
PluginInitializer<FixStringInterpretLayer> initFixString;
PluginInitializer<LogInputInterpretLayer> initLogInput;
PluginInitializer<LogCommandInterpretLayer> initLogCommand;
PluginInitializer<SayWhatInterpretLayer> initSayWhat;
