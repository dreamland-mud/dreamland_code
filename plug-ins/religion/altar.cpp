#include "religionattribute.h"
#include "defaultreligion.h"
#include "religionflags.h"

#include "commandtemplate.h"
#include "commonattributes.h"
#include "skillreference.h"
#include "skill.h"
#include "affecthandler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "liquid.h"
#include "affect.h"

#include "act.h"
#include "terrains.h"
#include "gsn_plugin.h"
#include "dreamland.h"
#include "loadsave.h"
#include "handler.h"
#include "fight.h"
#include "save.h"
#include "interp.h"
#include "wiznet.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define OBJ_VNUM_ALTAR 88

RELIG(none);
GSN(altar);

/*
 * 'altar' command
 */
CMDRUN(altar)
{
    if (ch->is_npc()) {
        ch->println("Изыди, глупое животное.");
        return;
    }

    if (IS_CHARMED(ch)) {
        ch->println("Боги вряд ли оценят такое.");
        return;
    }

    PCharacter *pch = ch->getPC();
    const Religion &religion = *(pch->getReligion());
    const char *rname = religion.getRussianName().c_str();

    if (religion.getIndex() == god_none) {
        ch->pecho("Но ты же закоренел%1$Gое|ый|ая атеист%1$G||ка.", ch);
        return;
    }

    DefaultReligion *drelig = dynamic_cast<DefaultReligion *>(pch->getReligion().getElement());
    if (!drelig || !drelig->flags.isSet(RELIG_CULT)) {
        ch->pecho("Похоже, %N1 совершенно равнодуш%gно|ен|на к жертвоприношениям.", 
                   rname, religion.getSex());
        return;
    }

    if (IS_VIOLENT(pch) || IS_KILLER(pch) || IS_THIEF(pch)) {
        ch->pecho("Ты слишком возбужден%Gо||а, чтобы воздвигать алтарь.", ch);
        return;
    }

    if (ch->isAffected(gsn_altar)) {
        ch->pecho("Ты все еще истощен%Gо||а постройкой предыдущего алтаря.", ch);
        return;
    }

    if (!ch->in_room->isCommon() 
          || IS_SET(ch->in_room->room_flags, ROOM_LAW)
          || IS_WATER(ch->in_room)) 
    {
        ch->println("Здесь неподходящее место для воздвигания алтарей.");
        return;
    }
    
    if (get_obj_room_vnum(ch->in_room, OBJ_VNUM_ALTAR)) {
        ch->println("Но здесь уже есть какой-то алтарь!");
        return;
    }

    Object *altar = create_object(get_obj_index(OBJ_VNUM_ALTAR), 0);
    obj_to_room(altar, ch->in_room);
    altar->timer = 60;
    altar->setOwner(religion.getName().c_str());
    
    ch->pecho("Ты сооружаешь %O4 для подношений %N3.", altar, rname);
    ch->recho("%^C1 сооружает %O4 для подношений своему божеству.", ch, altar);
    postaffect_to_char(ch, gsn_altar, 30);
    ch->setWaitViolence(2);
}

