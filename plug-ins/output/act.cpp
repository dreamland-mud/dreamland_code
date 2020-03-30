/* $Id: act.cpp,v 1.1.2.18 2010-08-24 20:26:47 rufina Exp $
 *
 * ruffina, 2004
 * 'act' format from DreamLand 2.0
 */
#include <stdarg.h>

#include "act.h"
#include "msgformatter.h"
#include "mudtags.h"

#include "char.h"
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

#include "descriptor.h"
#include "colour.h"
#include "merc.h"
#include "mercdb.h"
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

void act( const char *format, Character *ch, const void *arg1,
          const void *arg2, int type )
{
    act_p( format, ch, arg1, arg2, type, POS_RESTING );
}

void act_p( const char *format, Character *ch, const void *arg1,
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
#define MAXARGS 60

struct VarArgFormatter : public MsgFormatter {
    VarArgFormatter(Character *to) : MsgFormatter(to) {
    }
    DLString vfmt(const char *format, va_list av) {
        this->format = format;
        va_copy(this->av, av);
        argcnt = 0;
        
        if (to && !to->getPC())
            return DLString::emptyString;

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



/*---------------------------------------------------------------------------
 * format various player data 
 *--------------------------------------------------------------------------*/
/*
 * %n              - name
 * %N              - pretitle+name
 * %c              - clan litera 
 * %C              - clan short name
 * %k              - [colored clan litera] 
 * %K              - colored clan short name
 * %l              - real level
 * %L              - clan level
 * %r              - race 3 literas 
 * %R              - race name
 * %P              - profession
 * %t              - clan title
 * %T              - [colored clan title]
 * %p              - parsed title
 * %b              - (remort count)
 * %B              - (colored remort count)
 *
 * %a              - last access time (in numbers)
 * %A              - last access time (in words)
 *
 */
void player_fmt( const DLString &format, PCMemoryInterface *pc, ostringstream &buf, Character *to )
{
    const char *str;
    PCharacter *pch = pc->getPlayer( );
    
    if (format.empty( ))
        return;
    
    str = format.c_str( );

    while (*str != '\0') {
        DLString word;
        bool left = false;
        int width = 0;

        if (*str != '%') {
            buf << *str++;
            continue;
        }

        ++str;

        if (*str == '%') {
            buf << *str++;
            continue;
        }

        if (*str == '-') {
            left = true;
            ++str;
        }         

        while (isdigit( *str )) {
            width += width * 10 + *str - '0';
            str++;
        }
       
        switch (*str) {
        default:  
            break;
            
        case 'n':
            word = pc->getName( );
            break;

        case 'N':
            if (pch && to)
                word = to->seeName( pch );
            break;
            
        case 'c':
            if (!pc->getClan( )->isHidden( ))
                word = pc->getClan( )->getShortName( ).at( 0 );
            break;

        case 'C':
            word = pc->getClan( )->getColor( );
            break;
        
        case 'k':
            if (!pc->getClan( )->isHidden( ))
                word += "{x[{" 
                         + pc->getClan( )->getColor( )
                         + pc->getClan( )->getShortName( ).at( 0 )
                         + "{x]";
            break;

        case 'K':
            if (!pc->getClan( )->isHidden( ))
                word += pc->getClan( )->getColor( )
                        + pc->getClan( )->getShortName( )
                        + "{x";
            break;
            
        case 'l':
            word = pc->getLevel( );
            break;

        case 'L':
            word = pc->getClanLevel( );
            break;

        case 'r':
            word = pc->getRace( )->getPC( )->getWhoNameFor( to );
            
            if (word.size( ) < 4)
                word = dlprintf( "  %-4s", word.c_str( ) );
            else
                word = dlprintf( " %-5s", word.c_str( ) );
            break;

        case 'R':
            word = pc->getRace( )->getName( );
            break;

        case 'P':
            word = pc->getProfession( )->getNameFor( to );
            break;

        case 't':
            word = pc->getClan( )->getTitle( pc );
            break;

        case 'T':
            if (!pc->getClan( )->isHidden( ))
                word += "{x[{" 
                       + pc->getClan( )->getColor( ) 
                       + pc->getClan( )->getTitle( pc )
                       + "{x]";
            break;

        case 'p':
            if (pch) {
                ostringstream titleBuf;
                vistags_convert( pch->getParsedTitle().c_str(), titleBuf, to );             
                word = titleBuf.str();
            }
            break;
        
        case 'b':
            if (pc->getRemorts( ).size( ) > 0)
                word += "(" + DLString(pc->getRemorts( ).size( )) + ")";
            else
                word = "   ";
            break;
        
        case 'B':
            if (pc->getRemorts( ).size( ) > 0)
                word += "{W({M" + DLString(pc->getRemorts( ).size( )) + "{W){x";
            else
                word = "   ";
            break;
            
        case 'a':
            word = pc->getLastAccessTime( ).getTimeAsString( "%d/%m/%y %H:%M" );
            break;

        case 'A':
            word = pc->getLastAccessTime( ).getTimeAsString( );
            break;
        }

        if (left)
            buf << word;
    
        for (int i = 0; i < width - (int) word.colorLength( ); i++)
            buf << " ";

        if (!left)
            buf << word;
        
        if (*str)
            ++str;
    }
}

/*---------------------------------------------------------------------
 * simplified printf, understands color length in formatting
 *--------------------------------------------------------------------*/
DLString dlprintf( const char *fmt, ... ) 
{
    DLString result; 
    va_list arglist;
    const char *p = fmt;
    
    va_start(arglist, fmt);
    
    while (*p) {
        DLString word;
        bool left = false;
        int width = 0;

        if (*p != '%') {
            result += *p++;
            continue;
        }
        
        ++p;
        
        if (*p == '%') {
            result += '%';
            p++;
            continue;
        }

        if (*p == '-') {
            left = true;
            ++p;
        }         

        while (isdigit( *p )) {
            width += width * 10 + *p - '0';
            p++;
        }
       
        switch( *p ) {
        case 's':
            word += va_arg( arglist, char * );
            break;
        case 'd':
        case 'i':
        case 'l':
            word += (int) va_arg( arglist, int ); /* XXX */
            break;
        default:
            word += va_arg( arglist, int );
            break;
        }

        if (left)
            result += word;
    
        for (int i = 0; i < width - (int) word.colorLength( ); i++)
            result += ' ';

        if (!left)
            result += word;
        
        if (*p)
            p++;
    }

    va_end( arglist );
    return result;
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

void tell_raw( Character *ch, NPCharacter *talker, const char *format, ... ) 
{
    char buf[MAX_STRING_LENGTH], buf0[MAX_STRING_LENGTH];
    va_list ap;
    
    sprintf( buf0, "%s говорит тебе '{G%s{x'\n\r", ch->seeFullNameD( talker, '1' ).c_str( ), format );
    
    va_start( ap, format );
    vsprintf( buf,  buf0, ap );
    va_end( ap );

    buf[0] = Char::upper(buf[0]);
    ch->send_to(buf);
}

void say_act( Character *listener, Character *teller, 
              const char *msg, const void *arg )
{
    ostringstream buf;

    buf << "$C1 произносит '{g" << msg << "{x'";
    act( buf.str( ).c_str( ), listener, arg, teller, TO_ALL );
}

void tell_act( Character *listener, Character *teller, 
               const char *msg, const void *arg )
{
    ostringstream buf;

    buf << "$C1 говорит тебе '{G" << msg << "{x'";
    act( buf.str( ).c_str( ), listener, arg, teller, TO_CHAR );
}

void tell_dim( Character *listener, Character *teller, 
               const char *msg, const void *arg )
{
    ostringstream buf;

    buf << "$C1 говорит тебе '{g" << msg << "{x'";
    act( buf.str( ).c_str( ), listener, arg, teller, TO_CHAR );
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

    ch->println("[{cПодсказка{x] " + output);
}