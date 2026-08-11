#include <sstream>

#include "character.h"
#include "pcharacter.h"
#include "room.h"
#include "area.h"
#include "commandtemplate.h"
#include "wrappertarget.h"
#include "wrapperbase.h"
#include "fight.h"
#include "arg_utils.h"
#include "descriptor.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

/* 'path' refuses outright in these areas, so a link here would only ever answer
 * with that refusal. Same flag set the command guards itself with. */
static bool where_area_allows_path( Character *ch )
{
    return !IS_SET(ch->in_room->area->area_flag,
                   AREA_CLAN|AREA_DUNGEON|AREA_MANSION|AREA_WIZLOCK|AREA_SYSTEM);
}

/* The room name doubles as a link that plots the route there, so a click
 * answers "and how do I get to them". Web only: the tag collapses to the plain
 * name for telnet and screen readers, exactly as before.
 *
 * The link carries the vnum rather than the name -- exact, and free of the
 * multiple-match problem a name like "Коридор" has. No link where 'path' would
 * answer with a refusal or a shrug: special rooms (isCommon is the engine's own
 * definition of special) and the viewer's own room. */
static DLString format_where_room( Character *ch, Room *room, bool areaAllowsPath )
{
    DLString name = room->getName( viewerLang( ch ) );

    if (!areaAllowsPath || room == ch->in_room || !room->isCommon( ))
        return name;

    ostringstream buf;
    buf << "{hc'path " << room->vnum << "'" << name << "{x";
    return buf.str( );
}

static void format_where( Character *ch, Character *victim, bool areaAllowsPath )
{
    bool fPK, fAfk;

    fPK = (!victim->is_npc( )
            && victim->getModifyLevel( ) >= PK_MIN_LEVEL
            && !is_safe_nomessage( ch, victim->getDoppel( ch ) ));
    fAfk = IS_SET(victim->comm, COMM_AFK);

    // The room name is the last column, so its old %-42s width only ever added
    // trailing blanks -- and a width counts the markup bytes, not the letters.
    ch->pecho( "%-25C1 {x%s{x%s %s{x",
                victim,
                fPK  ? "({rPK{x)"  : "    ",
                fAfk ? "[{CAFK{x]" : "     ",
                format_where_room( ch, victim->in_room, areaAllowsPath ).c_str( ) );
}

/* "в {hh1392Мидгаарде{x", "on the {hh2001Chessboard{x": the particle is per-area
 * data (nothing in the name tells you a mountain takes "на"/"on" while a city
 * takes "в"/"in"), and the name is flexed into the 6th case.
 *
 * The anchor carries the help ID. The idless {hh form makes the client resolve
 * the article by the anchor TEXT (mudtags.cpp, hyper_tag_start), and a declined
 * name matches no help keyword -- so an area with no article gets no link at
 * all rather than a dead one. */
static DLString where_area_phrase( Character *ch )
{
    AreaIndexData *area = ch->in_room->areaIndex( );
    lang_t lang = viewerLang( ch );
    ostringstream buf;
    int helpId = 0;

    for (auto &article: area->helps)
        if (article->getID( ) > 0) {
            helpId = article->getID( );
            break;
        }

    buf << area->getPreposition( lang ) << " {W";
    if (helpId > 0)
        buf << "{hh" << helpId;
    buf << area->getName( lang, '6' ) << "{x";

    return buf.str( );
}

static bool rprog_where( Character *ch, const char *arg )
{
    FENIA_CALL( ch->in_room, "Where", "Cs", ch, arg );
    return false;
}

CMDRUNP( where )
{
    Character *victim = 0;
    Descriptor *d;
    bool found;
    bool fPKonly = false;
    DLString arg( argument );

    ch->setWaitViolence( 1 );

    if (eyes_blinded( ch )) {
        ch->pecho( _("Ты не можешь видеть вещи!") );
        return;
    }
    
    if (eyes_darkened( ch )) {
        ch->pecho( _("Ты ничего не видишь! Слишком темно!") );
        return;
    }

    arg.stripWhiteSpace( );
    arg.toLower( );
    
    if (arg_is_pk( arg ))
        fPKonly = true;
    
    if (rprog_where( ch, arg.c_str( ) ))
        return;

    DLString areaPhrase = where_area_phrase( ch );
    bool areaAllowsPath = where_area_allows_path( ch );

    if (arg.empty( ) || fPKonly)
    {
        ch->pecho( _("Ты находишься %s. Недалеко от тебя:"), areaPhrase.c_str( ) );
        found = false;

        for ( d = descriptor_list; d; d = d->next )
        {
            if (d->connected != CON_PLAYING)
                continue;
            if (( victim = d->character ) == 0)
                continue;
            // a player possessing a mob via switch shows up as their own (vulnerable) body
            if (victim->is_npc( )) {
                if (victim->getPC( ))
                    victim = victim->getPC( );
                else
                    continue;
            }
            if (!victim->in_room || victim->in_room->area != ch->in_room->area)
                continue;
            if (IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
                continue;
            if (!ch->can_see( victim ))
                continue;
            if (fPKonly && is_safe_nomessage( ch, victim ))
                continue;
            
            found = true;
            format_where( ch, victim, areaAllowsPath );
        }

        if (!found)
            ch->pecho(_("Никого."));
    }
    else
    {
        // No "Недалеко от тебя" here: a name search still only covers the
        // current area, but the matches can sit anywhere in it.
        ch->pecho( _("Ты находишься %s."), areaPhrase.c_str( ) );
        found = false;
        for ( victim = char_list; victim != 0; victim = victim->next )
        {
            if ( victim->in_room != 0
                    && victim->in_room->area == ch->in_room->area
                    && ( !victim->is_npc()
                    || ( victim->is_npc() && !IS_SET(victim->act, ACT_NOWHERE) ) )
                    && ch->can_see( victim )
                    && is_name( arg.c_str(), victim->getNameP( '7' ).c_str() )
                    && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
            {
                found = true;
                format_where( ch, victim, areaAllowsPath );
            }
        }

        if (!found)
            oldact(_("Ты не находишь $T."), ch, 0, arg.c_str(), TO_CHAR);
    }
}


