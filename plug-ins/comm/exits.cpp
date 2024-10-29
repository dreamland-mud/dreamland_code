#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "commandtemplate.h"
#include "loadsave.h"
#include "directions.h"
#include "websocketrpc.h"
#include "morphology.h"
#include "act.h"
#include "merc.h"
#include "def.h"

#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))
#define CLR_EXIT0(ch)        (MILD(ch) ? "w" : "c")
#define CLR_EXIT1(ch)        (MILD(ch) ? "W" : "C")

/*---------------------------------------------------------------------------
 * 'exits' command 
 *--------------------------------------------------------------------------*/
static DLString cmd_extra_exit(Character *ch, EXTRA_EXIT_DATA *eexit, bool prefereShortDescr)
{
    DLString kw_en = Syntax::label_en(eexit->keyword);
    DLString kw_ru = Syntax::label_ru(eexit->keyword);

    DLString nameRus = prefereShortDescr ? eexit->short_desc_from.get(LANG_DEFAULT).ruscase('1') : "";
    nameRus = nameRus.empty() ? (kw_ru.empty() ? kw_en : kw_ru) : nameRus;

    DLString cmd = kw_ru.empty() ? "идти " + kw_en : "идти " + kw_ru;        
    return web_cmd(ch, cmd, nameRus);
}

static DLString cmd_exit(Character *ch, int door, EXIT_DATA *pexit)
{
    DLString cmd;
    PlayerConfig cfg = ch->getConfig();
    DLString ename = (cfg.ruexits ? dirs[door].rname : dirs[door].name);

    if (IS_SET(pexit->exit_info, EX_LOCKED))
        return "отпереть " + ename + " | " + ename;

    if (IS_SET(pexit->exit_info, EX_CLOSED))
        return  "открыть " + ename + " | " + ename;
        
    return ename;
}

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

    if (eyes_blinded( ch )) 
        return;

    buf << "{" << clr0 << "[{" << clr1 << web_cmd(ch, "выходы", "Выходы") << "{" << clr0 << ":{" << clr1;
    found = false;
    cfg = ch->getConfig( );

    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        if (!( pexit = targetRoom->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        if (!ch->can_see( room ))
            continue;

        ename = (cfg.ruexits ? dirs[door].rname : dirs[door].name);
        cmd = cmd_exit(ch, door, pexit);
        star = IS_SET(pexit->exit_info, EX_CLOSED|EX_LOCKED) ? "*" : "";

        buf << " " << star << web_cmd(ch, cmd, ename) << star;
        found = true;
    }
    
    if (!found)
        buf << (cfg.ruexits ? " нет" : " none");
            

    StringList extras;
    for (auto &eexit: targetRoom->extra_exits)
        if (ch->can_see(eexit))
            extras.push_back(cmd_extra_exit(ch, eexit, false));

    if (!extras.empty())
        buf << " {" << clr0 << "| {" << clr1 << extras.join(", ");

    buf << "{" << clr0 << "]{x";
    buf << endl;
    ch->send_to( buf );
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

    if (eyes_blinded( ch )) {
        oldact("Ты ослепле$gно|н|на и не видишь ничего вокруг себя!", ch, 0, 0, TO_CHAR );
        return;
    }

    found = false;
    cfg = ch->getConfig( );

    if (cfg.holy)
        buf << "Видимые выходы из комнаты " << ch->in_room->vnum << ":" << endl;
    else
        buf << "Видимые выходы:" << endl;

    for (door = 0; door <= 5; door++)
    {
        if (!( pexit = ch->in_room->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        if (!ch->can_see( room ))
            continue;

        found = true;   
        ename = (cfg.ruexits ? dirs[door].rname : dirs[door].name);
        cmd = cmd_exit(ch, door, pexit);

        if (!IS_SET(pexit->exit_info, EX_CLOSED|EX_LOCKED))
        {
            buf << "    {C" << fmt(0, web_cmd(ch, cmd, "%-8s").c_str(), ename.c_str()) << "{x - ";

            if (room->isDark( ) && !cfg.holy && !IS_AFFECTED(ch, AFF_INFRARED ))
                buf << "Дорога ведет в темноту и неизвестность...";
            else
                buf << room->getName();

            if (cfg.holy)
                buf << " (room " << room->vnum << ")";
            
            buf << endl;
        }
        else {
            ename = "*" + ename + "*";
            buf << "    {C" << fmt(0, web_cmd(ch, cmd, "%-8s").c_str(), ename.c_str()) << "{x - "
                << russian_case(direction_doorname(pexit), '1') 
                << " (" << (IS_SET(pexit->exit_info, EX_LOCKED) ? "заперто" : "закрыто") << ")";

            if (cfg.holy)
                buf << " (room " << room->vnum << ")";
            
            buf << endl;
        }
    }

    if (!found)
        buf << "    нет" << endl;

    ch->send_to( buf );

    found = false;
    buf.str("");

    for (auto &eexit: ch->in_room->extra_exits) {
        if (ch->can_see(eexit)) {
            buf <<  "    {C" << cmd_extra_exit(ch, eexit, true) << "{x " << endl;
            found = true;
        }
    }

    if (found) {
        ch->pecho("\r\nДополнительные выходы:");
        ch->send_to(buf);
    }
}

