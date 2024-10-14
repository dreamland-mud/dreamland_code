#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "commandtemplate.h"
#include "directions.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------
 *    scan
 *-------------------------------------------------------------------*/
#define MILD(ch) (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_SCAN_MOB(ch) (MILD(ch) ? "w" : "W")
#define CLR_SCAN_DIR(ch) (MILD(ch) ? "c" : "C")
#define CLR_SCAN_DOOR(ch) (MILD(ch) ? "w" : "W")

static void scan_people(Room *room, Character *ch, int depth, int door,
                        bool fShowDir, ostringstream &buf)
{
    Character *rch, *orig;
    bool found;
    bool fRus = ch->getConfig().ruexits;

    found = false;

    for (rch = room->people; rch != 0; rch = rch->next_in_room)
    {
        if (rch == ch)
            continue;
        if (rch->invis_level > ch->get_trust())
            continue;
        if (!ch->can_see(rch))
            continue;

        if (!found)
        {
            buf << "{" << CLR_SCAN_DIR(ch);

            if (door != -1)
            {
                if (fShowDir)
                    buf << (fRus ? dirs[door].where : dirs[door].name);
                else
                    buf << "Дальность " << depth;
            }
            else
                buf << "Здесь";

            buf << ":{x" << endl;
            found = true;
        }

        orig = rch->getDoppel(ch);

        buf << "    {" << CLR_SCAN_MOB(ch) << ch->sees(orig, '1') << ".{x";

        if (IS_SET(orig->comm, COMM_AFK))
            buf << " {w[{CАФК{x{w]{x";

        buf << endl;
    }
}

static Room *scan_room(Room *start_room, Character *ch, int depth, int door,
                       bool fShowDir, ostringstream &buf)
{
    EXIT_DATA *pExit;
    Room *room;
    bool fRus = ch->getConfig().ruexits;

    pExit = start_room->exit[door];

    if (!pExit || !ch->can_see(pExit))
        return NULL;

    room = pExit->u1.to_room;

    if (IS_SET(pExit->exit_info, EX_CLOSED))
    {
        buf << "{" << CLR_SCAN_DIR(ch);

        if (fShowDir)
            buf << (fRus ? dirs[door].where : dirs[door].name);
        else
            buf << "Дальность " << depth;

        buf << ":{x" << endl
            << "    {" << CLR_SCAN_DOOR(ch) << russian_case(direction_doorname(pExit), '1') << " (закрыто).{x" << endl;

        return NULL;
    }

    if (IS_SET(pExit->exit_info, EX_NOSCAN))
    {
        buf << "{" << CLR_SCAN_DIR(ch);

        if (fShowDir)
            buf << (fRus ? dirs[door].where : dirs[door].name);
        else
            buf << "Дальность " << depth;

        buf << ":{x" << endl
            << "    Невозможно что-либо разглядеть." << endl;
        return NULL;
    }

    scan_people(room, ch, depth, door, fShowDir, buf);
    return room;
}

CMDRUNP(scan)
{
    ostringstream buf;
    char arg1[MAX_INPUT_LENGTH];
    Room *room;
    int door, depth;
    int range;

    if (ch->position < POS_SLEEPING)
    {
        ch->pecho("Ты ничего не видишь, кроме звезд...");
        return;
    }

    if (ch->position == POS_SLEEPING)
    {
        ch->pecho("Ты спишь и можешь видеть только сны.");
        return;
    }

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
        oldact("$c1 осматривает все вокруг.", ch, 0, 0, TO_ROOM);
        buf << "Осмотревшись, ты видишь:" << endl;
        scan_people(ch->in_room, ch, 0, -1, true, buf);

        for (door = 0; door < DIR_SOMEWHERE; door++)
            scan_room(ch->in_room, ch, 1, door, true, buf);

        ch->send_to(buf);
        return;
    }

    door = direction_lookup(arg1);

    if (door < 0)
    {
        ch->pecho("В какую сторону?");
        return;
    }

    oldact("Ты пристально смотришь $T.", ch, 0, dirs[door].leave, TO_CHAR);
    oldact("$c1 пристально смотрит $T.", ch, 0, dirs[door].leave, TO_ROOM);

    range = max(1, ch->getModifyLevel() / 10);
    room = ch->in_room;

    for (depth = 1; depth <= range; depth++)
    {
        room = scan_room(room, ch, depth, door, false, buf);

        if (!room)
            break;
    }

    if (!buf.str().empty())
    {
        ch->pecho("Ты видишь:");
        ch->send_to(buf);
    }
}

