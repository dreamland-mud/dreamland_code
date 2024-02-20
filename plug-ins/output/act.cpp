/* $Id: act.cpp,v 1.1.2.18 2010-08-24 20:26:47 rufina Exp $
 *
 * ruffina, 2004
 * 'act' format from DreamLand 2.0
 */
#include <stdarg.h>

#include "act.h"
#include "msgformatter.h"
#include "mudtags.h"


#include "logstream.h"
#include "profiler.h"
#include "noun.h"
#include "grammar_entities_impl.h"
#include "russianstring.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "pcrace.h"
#include "object.h"
#include "room.h"
#include "skill.h"

#include "descriptor.h"
#include "colour.h"
#include "merc.h"

#include "def.h"

/*--------------------------------------------------------------------------
 * 'act' interface functions 
 *--------------------------------------------------------------------------*/
static const char * actChar_to_fmtChar(char c)
{
    switch (c) {
    case 'C':        return "%3$C";
    case 'G':        return "%3$G";
    case 'O':        return "%3$O";

    case 'E':        return "%3$P1";
    case 'S':        return "%3$P2";
    case 'M':        return "%3$P3";
    case 'X':        return "%3$P4";
    case 'Y':        return "%3$P5";
    case 'Z':        return "%3$P6";
    case 'U':        return "%3$P7";

    case 'c':        return "%1$C";
    case 'g':        return "%1$G";
    case 'o':        return "%2$O";

    case 'e':        return "%1$P1";
    case 's':        return "%1$P2";
    case 'm':        return "%1$P3";
    case 'x':        return "%1$P4";
    case 'y':        return "%1$P5";
    case 'z':        return "%1$P6";
    case 'u':        return "%1$P7";

    case 'T':        return "%3$s";
    case 'N':        return "%3$N";
    case 'd':        return "%3$S";
    case 't':        return "%2$s";
    case 'n':        return "%2$N";

    default:        return "";
    }
}

DLString act_to_fmt(const char *s)
{
    ostringstream buf;

    while (*s != '\0') {
        if (*s != '$') {
            buf << *s++;
            continue;
        }

        ++s;
        buf << actChar_to_fmtChar(*s);
        ++s;
    }
    
    // слово 'люди' пишется с большой буквы
    DLString rc = buf.str();
    DLString r = rc.colourStrip();
    if (r.length() > 2 && r.at(0) == '%' && isdigit(r.at(1)) && r.at(2) == '$') {
        DLString::size_type i = rc.find_first_of('$');
        rc = rc.substr(0, i+1) + "^" + rc.substr(i+1);
    }
        
    return rc;
}

void oldact( const char *format, Character *ch, const void *arg1,
          const void *arg2, int type )
{
    oldact_p( format, ch, arg1, arg2, type, POS_RESTING );
}

void oldact_p( const char *format, Character *ch, const void *arg1,
            const void *arg2, int type, int min_pos )
{
    if (!ch)
        return;

    if (format == 0 || !format[0]) 
        return;
    
    Character *vch = (Character *)arg2;
    DLString fmt = act_to_fmt(format);

    ch->echo( min_pos, type, vch, fmt.c_str(), ch, arg1, arg2 );    
}


/*--------------------------------------------------------------------------
 * fmt and vfmt (new format concept)
 *--------------------------------------------------------------------------*/
#define MAXARGS 256

struct VarArgFormatter : public MsgFormatter {
    VarArgFormatter(Character *to) : MsgFormatter(to) {
    }
    DLString vfmt(const char *format, va_list av) {
        this->format = format;
        va_copy(this->av, av);
        argcnt = 0;
        
//        if (to && !to->getPC())
//            return DLString::emptyString;

        DLString s = run();
        va_end(av);
        return s;
    }
protected:    
    virtual void nextArg() {
        d = args[argcnt++] = va_arg(av, arg_t);
    }
    virtual void shiftArg(int i) {
        while(argcnt < i)
            args[argcnt++] = va_arg(av, arg_t);

        d = args[i-1];
    }
    virtual char argChar() {
        return d.c;
    }
    virtual int argInt() {
        return d.number;
    }
    virtual unsigned int argUInt() {
        return d.unumber;
    }
    virtual float argFloat() {
        return d.fnumber;
    }
    virtual DLString argStr() {
        return DLString(d.string);
    }
    virtual const Skill * argSkill() {
        return d.skill;
    }
    virtual Grammar::Noun::Pointer argNoun(int nounFlags) {
        return dynamic_cast<const Grammar::NounHolder *>(d.obj)->toNoun(to, nounFlags);
    }
private:
    typedef union {
        char c;
        int number;
        unsigned int unumber;
        float fnumber;
        char *string;
        Character *ch;
        Object *obj;
        RussianString *rstr;
        const Skill *skill;
    } arg_t;
    arg_t args[MAXARGS], d;
    int argcnt;
    va_list av;
};

DLString fmt(Character *to, const char *f, ...)
{
    va_list av;

    va_start(av, f);
    
    DLString rc = vfmt(to, f, av);

    va_end(av);

    return rc;
}

DLString vfmt(Character *to, const char *format, va_list av)
{
    VarArgFormatter formatter(to);
    return formatter.vfmt(format, av);
}


/*--------------------------------------------------------------------------
 * tell-like output 
 *--------------------------------------------------------------------------*/
void tell_fmt( const char *msg, ... )
{
    va_list ap, ap0;
    ostringstream buf;
    typedef union { Character *ch; } arg_t;
    arg_t listener;
    
    buf << "%2$^C1 говорит тебе '{G" << msg << "{x'";
    va_start( ap, msg );
    va_copy( ap0, ap );
    listener = va_arg(ap, arg_t);
    listener.ch->vpecho( buf.str( ).c_str( ), ap0 );
    va_end( ap );
    va_end( ap0 );
}

void say_fmt( const char *msg, ... )
{
    va_list ap, ap0;
    ostringstream buf;
    typedef union { Character *ch; } arg_t;
    arg_t teller;
    
    buf << "%1$^C1 произносит '{g" << msg << "{x'";
    va_start( ap, msg );
    va_copy( ap0, ap );
    teller = va_arg(ap, arg_t);
    teller.ch->vecho( POS_RESTING, TO_ALL, NULL, buf.str( ).c_str( ), ap0 );
    va_end( ap );
    va_end( ap0 );
}

void tell_raw(Character *ch, NPCharacter *talker, const char *format, ...)
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);

    DLString messageStart = fmt(ch, "%^C1 говорит тебе '{G", talker);
    DLString messageMiddle = buf;
    DLString messageEnd = "{x'\n\r";

    ch->send_to(messageStart + messageMiddle + messageEnd);
}

void say_act( Character *listener, Character *teller, 
              const char *msg, const void *arg )
{
    ostringstream buf;

    buf << "$C1 произносит '{g" << msg << "{x'";
    oldact( buf.str( ).c_str( ), listener, arg, teller, TO_ALL );
}

void tell_act( Character *listener, Character *teller, 
               const char *msg, const void *arg )
{
    ostringstream buf;

    buf << "$C1 говорит тебе '{G" << msg << "{x'";
    oldact( buf.str( ).c_str( ), listener, arg, teller, TO_CHAR );
}

void tell_dim( Character *listener, Character *teller, 
               const char *msg, const void *arg )
{
    ostringstream buf;

    buf << "$C1 говорит тебе '{g" << msg << "{x'";
    oldact( buf.str( ).c_str( ), listener, arg, teller, TO_CHAR );
}

void hint_fmt(Character *ch, const char *format, ...)
{
    if (ch->is_npc())
        return;
    
    // Everyone below level 50 and living first life is a newbie.
    if (!ch->is_immortal()) {
        if (ch->getPC()->getRemorts().size() > 0)
            return;

        if (ch->getLevel() >= 50)
            return;
    }

    va_list av;
    va_start(av, format);
    DLString output = vfmt(ch, format, av);
    va_end(av);

    ch->pecho("{y[{GПодсказка{y]{x " + output);
}

void echo_master(Character *ch, const char *format, ...)
{
    va_list av;
    va_start(av, format);

    bool isCharmed = (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL);
    bool needsOutput = isCharmed && ch->master->getPC() && ch->master->getPC()->getAttributes().isAvailable("ordering");

    if (needsOutput)
        ch->master->pecho("{W%#^C1 {Wне может выполнить твой приказ, потому что видит следующее:{x", ch);

    ch->vpecho(format, av);

    if (needsOutput)
        ch->master->vpecho(format, av);

    va_end(av);
}



void echo_char(Character *ch, bool (needsOutput)(Character *), const char *format, ...)
{
    va_list av;
    
    va_start(av, format);
    ch->vecho( POS_RESTING, TO_CHAR, 0, format, av, needsOutput ); 
    va_end(av);
}

void echo_room(Character *ch, bool (needsOutput)(Character *), const char *format, ...)
{
    va_list av;
    
    va_start(av, format);
    ch->vecho( POS_RESTING, TO_ROOM, 0, format, av, needsOutput ); 
    va_end(av);
}

void echo_notvict(Character *ch, Character *victim, bool (needsOutput)(Character *), const char *format, ...)
{
    va_list av;
    
    va_start(av, format);
    ch->vecho( POS_RESTING, TO_NOTVICT, victim, format, av, needsOutput ); 
    va_end(av);
}
