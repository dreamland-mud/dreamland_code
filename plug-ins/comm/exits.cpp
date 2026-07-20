#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "commandtemplate.h"
#include "skillreference.h"
#include "bonus.h"
#include "loadsave.h"
#include "directions.h"
#include "websocketrpc.h"
#include "morphology.h"
#include "skill_utils.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

GSN(perception);
BONUS(thief_skills);

#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))
#define CLR_EXIT0(ch)        (MILD(ch) ? "w" : "c")
#define CLR_EXIT1(ch)        (MILD(ch) ? "W" : "C")

/*---------------------------------------------------------------------------
 * 'exits' command 
 *--------------------------------------------------------------------------*/
/**
 * Return a webclient command to interact with the extra exit.
 * If the short description is not set (or not preferred by the caller), the keyword is used.
 */
static DLString cmd_extra_exit(Character *ch, EXTRA_EXIT_DATA *eexit, bool prefereShortDescr)
{
    DLString kw_en = Syntax::label_en(eexit->keyword);
    DLString kw_ru = Syntax::label_ru(eexit->keyword);

    DLString nameRus = prefereShortDescr ? eexit->short_desc_from.get(LANG_DEFAULT).ruscase('1') : "";
    nameRus = nameRus.empty() ? (kw_ru.empty() ? kw_en : kw_ru) : nameRus;

    DLString cmd = kw_ru.empty() ? "идти " + kw_en : "идти " + kw_ru;        
    return web_cmd(ch, cmd, nameRus);
}

/**
 * Return a webclient command to interact with the exit.
 */
static DLString cmd_exit(Character *ch, int door, EXIT_DATA *pexit)
{
    DLString cmd;
    // Keep this 2-valued (RU vs EN): 'ename' becomes a movement command the web
    // client sends back, so it must be a parser-recognized direction keyword.
    // UA viewers fall back to the Russian keyword rather than an unparseable one.
    DLString ename = (viewerLang(ch) != LANG_EN ? dirs[door].rname : dirs[door].name);

    if (IS_SET(pexit->exit_info, EX_LOCKED))
        return "отпереть " + ename;

    if (IS_SET(pexit->exit_info, EX_CLOSED))
        return  "открыть " + ename;
        
    return ename;
}

/**  
 * Check if the player has a chance to perceive hidden exits.
 */
static bool check_perception(Character *ch)
{
    if (!gsn_perception->usable(ch))
        return false;

    int chance = gsn_perception->getEffective(ch) + skill_level_bonus(*gsn_perception, ch);

    if (bonus_check_and_report(*bonus_thief_skills, ch))
        chance += number_range(20, 25);

    if (number_percent() < URANGE(1, chance, 100)) {
        gsn_perception->improve(ch, true);
        return true;

    } else {
        gsn_perception->improve(ch, false);
        return false;
    }
}

/**
 * Show exits from the room to the player, in the format:
 * [Exits: north east south west up down | extra1 extra2]
 */
void show_exits_to_char( Character *ch, Room *targetRoom )
{
    ostringstream buf;
    EXIT_DATA *pexit;
    DLString ename;
    DLString cmd;
    DLString star;
    PlayerConfig cfg;
    Room *room;
    bool found;
    auto clr0 = CLR_EXIT0(ch);
    auto clr1 = CLR_EXIT1(ch);
    int hiddenExits = 0;

    if (eyes_blinded( ch )) 
        return;

    buf << "{" << clr0 << "[{" << clr1 << web_cmd(ch, "выходы", l(ch, "Выходы")) << "{" << clr0 << ":{" << clr1;
    found = false;
    cfg = ch->getConfig( );

    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        if (!( pexit = targetRoom->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        // Check for room visibility separately, so we don't spam "there are hidden exits" for immortal rooms
        if (!ch->can_see(room))
            continue;
        if (!ch->can_see(pexit)) {
            hiddenExits++;
            continue;
        }

        ename = direction_word(viewerLang(ch), door, DIR_CASE_NOM);
        cmd = cmd_exit(ch, door, pexit);
        star = IS_SET(pexit->exit_info, EX_CLOSED|EX_LOCKED) ? "*" : "";

        buf << " " << star << web_cmd(ch, cmd, ename) << star;
        found = true;
    }
    
    if (!found)
        buf << lmsg(viewerLang(ch), " none", " нет", " немає");
            

    StringList extras;
    for (auto &eexit: targetRoom->extra_exits) {
        if (!(room = eexit->u1.to_room))
            continue;
        if (!ch->can_see(room))
            continue;
        if (!ch->can_see(eexit)) {
            hiddenExits++;
            continue;
        }

        extras.push_back(cmd_extra_exit(ch, eexit, false));
    }

    if (!extras.empty())
        buf << " {" << clr0 << "| {" << clr1 << extras.join(", ");

    buf << "{" << clr0 << "]{x";
    buf << endl;
    ch->send_to( buf );

    if (hiddenExits > 0 && check_perception(ch)) {
        ch->pecho("%s", l(ch, "Интуиция подсказывает тебе, что тут есть невидимые выходы."));
    }
}

CMDRUNP( exits )
{
    ostringstream buf;
    EXIT_DATA *pexit;
    DLString ename;
    DLString cmd;
    Room *room;
    PlayerConfig cfg;
    bool found;
    int door;
    int hiddenExits = 0;

    if (eyes_blinded( ch )) {
        oldact(_("Ты ослепле$gно|н|на и не видишь ничего вокруг себя!"), ch, 0, 0, TO_CHAR );
        return;
    }

    found = false;
    cfg = ch->getConfig( );

    if (cfg.holy)
        buf << l(ch, "Видимые выходы из комнаты ") << ch->in_room->vnum << ":" << endl;
    else
        buf << l(ch, "Видимые выходы:") << endl;

    for (door = 0; door <= 5; door++)
    {
        if (!( pexit = ch->in_room->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        if (!ch->can_see( room ))
            continue;
        if (!ch->can_see(pexit)) {
            hiddenExits++;
            continue;
        }

        found = true;   
        ename = direction_word(viewerLang(ch), door, DIR_CASE_NOM);
        cmd = cmd_exit(ch, door, pexit);

        if (!IS_SET(pexit->exit_info, EX_CLOSED|EX_LOCKED))
        {
            buf << "    {C" << fmt(0, web_cmd(ch, cmd, "%-8s").c_str(), ename.c_str()) << "{x - ";

            if (room->isDark( ) && !cfg.holy && !IS_AFFECTED(ch, AFF_INFRARED ))
                buf << l(ch, "Дорога ведет в темноту и неизвестность...");
            else
                buf << room->getName(viewerLang(ch));

            if (cfg.holy)
                buf << " (room " << room->vnum << ")";
            
            buf << endl;
        }
        else {
            ename = "*" + ename + "*";
            buf << "    {C" << fmt(0, web_cmd(ch, cmd, "%-8s").c_str(), ename.c_str()) << "{x - "
                << direction_doorname_langtext(pexit, '1').forLang(viewerLang(ch))
                << " (" << (IS_SET(pexit->exit_info, EX_LOCKED) ? l(ch, "заперто") : l(ch, "закрыто")) << ")";

            if (cfg.holy)
                buf << " (room " << room->vnum << ")";
            
            buf << endl;
        }
    }

    if (!found)
        buf << "    " << l(ch, "нет") << endl;

    ch->send_to( buf );

    found = false;
    buf.str("");

    for (auto &eexit: ch->in_room->extra_exits) {
        if (!(room = eexit->u1.to_room))
            continue;
        if (!ch->can_see(room))
            continue;
        if (!ch->can_see(eexit)) {
            hiddenExits++;
            continue;
        }

        buf <<  "    {C" << cmd_extra_exit(ch, eexit, true) << "{x " << endl;
        found = true;
    }

    if (found) {
        ch->pecho("\r\n%s", l(ch, "Дополнительные выходы:"));
        ch->send_to(buf);
    }

    if (hiddenExits > 0 && check_perception(ch)) {
        ch->pecho("%s", l(ch, "Интуиция подсказывает тебе, что тут есть невидимые выходы."));
    }
}

