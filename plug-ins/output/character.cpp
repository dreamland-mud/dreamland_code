/* $Id$
 *
 * ruffina, 2004
 */
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "npcharacter.h"
#include "room.h"

#include "act.h"
#include "colour.h"
#include "mudtags.h"
#include "descriptor.h"
#include "def.h"

static bool mprog_act( Character *ch, const char *msg )
{
    FENIA_CALL( ch, "Act", "s", msg );
    FENIA_NDX_CALL( ch->getNPC( ), "Act", "Cs", ch, msg );
    return false;
}

/****************************************************************************
 * text output 
 ****************************************************************************/
void Character::println( const DLString &txt )
{
    if (!txt.empty( )) {
        ostringstream buf;
        
        buf << txt << endl;
        send_to( buf );
    }
}

void Character::send_to( const DLString& txt )
{
    send_to( txt.c_str( ) );
}

void Character::send_to( ostringstream &stream )
{
    send_to( stream.str( ).c_str( ) );
}

/*
 * Write to one char with color
 */
void Character::send_to( const char *txt )
{
    ostringstream out;

    if (!txt || !desc)
        return;

    mudtags_convert( txt, out, this );
    desc->send( out.str( ).c_str( ) );
}

void Character::printf( const char *format, ... ) 
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, format );
    vsprintf( buf, format, ap );
    va_end( ap );
    
    send_to( buf );
}

void Character::vpecho( const char *f, va_list av)
{
    DLString r;
    
    if (( r = vfmt(this, f, av) ).empty( ))
        return;
           
    r += "\r\n";
    send_to( r );
    mprog_act( this, r.c_str( ) );
}

void Character::pecho( const char *f, ... )
{
    va_list av;

    va_start(av, f);
    vpecho(f, av);
    va_end(av);
}

void Character::pecho( int pos, const char *f, ... )
{
    if (position >= pos) {
        va_list av;

        va_start(av, f);
        vpecho(f, av);
        va_end(av);
    }
}

void Character::recho( int pos, const char *f, ... )
{
    va_list av;
    
    va_start(av, f);
    vecho( pos, TO_ROOM, NULL, f, av ); 
    va_end(av);
}

void Character::recho( const char *f, ... )
{
    va_list av;
    
    va_start(av, f);
    vecho( POS_RESTING, TO_ROOM, NULL, f, av ); 
    va_end(av);
}

void Character::recho( Character *vch, const char *f, ... )
{
    va_list av;
    
    va_start(av, f);
    vecho( POS_RESTING, TO_NOTVICT, vch, f, av ); 
    va_end(av);
}

void Character::echo( int pos, int type, Character *vch, const char *f, ... )
{
    va_list av;
    
    va_start(av, f);
    vecho( pos, type, vch, f, av ); 
    va_end(av);
}

void Character::vecho( int pos, int type, Character *vch, const char *f, va_list av )
{
    Character *to;
    typedef map<Character *, DLString> Targets;
    Targets targets;
    DLString r;

    if(in_room == NULL)
        return;
    
    if (type == TO_NOBODY)
        return;
    
    to = in_room->people;
    
    if (type == TO_VICT) {
        if (!vch || !vch->in_room)
            return;

        to = vch->in_room->people;
    }
    
    for ( ; to; to = to->next_in_room) {
        if (to->position < pos)
            continue;

        if (type == TO_CHAR && to != this)
            continue;

        if (type == TO_VICT && (to != vch || to == this))
            continue;

        if (type == TO_ROOM && to == this)
            continue;

        if (type == TO_NOTVICT && (to == vch || to == this))
            continue;

        if (type != TO_VICT && type != TO_CHAR)
            if (!to->can_sense( this ) || (vch && !to->can_sense( vch )))
                continue;
        
        if (( r = vfmt(to, f, av) ).empty( ))
            continue;
        
        r << "\r\n";
        targets[to] = r;
        to->send_to( r );
    }
        
    for (Targets::iterator t = targets.begin( ); t != targets.end( ); t++)
        mprog_act( t->first, t->second.c_str( ) );
}


