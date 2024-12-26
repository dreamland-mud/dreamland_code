/* $Id: knight.cpp,v 1.1.6.10.4.18 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "knight.h"

#include "commonattributes.h"

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "dreamland.h"
#include "act.h"
#include "interp.h"

#include "merc.h"
#include "vnum.h"
#include "msgformatter.h"
#include "fight.h"
#include "weapongenerator.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "loadsave.h"
#include "magic.h"
#include "def.h"

CLAN(knight);
GSN(acid_arrow);
GSN(acid_blast);
GSN(caustic_font);
GSN(dispel_affects);
GSN(guard);
GSN(spellbane);

#define OBJ_VNUM_DRAGONDAGGER 80
#define OBJ_VNUM_DRAGONMACE 81
#define OBJ_VNUM_PLATE 82
#define OBJ_VNUM_DRAGONSWORD 83
#define OBJ_VNUM_DRAGONLANCE 99

/*--------------------------------------------------------------------------
 * clan item 
 *-------------------------------------------------------------------------*/
void ClanItemKnight::actDisappear()
{
    oldact("$o1 исчезает в серой дымке.",
        obj->getRoom()->people, obj, 0, TO_ALL);
}

/*--------------------------------------------------------------------------
 * clan altar 
 *-------------------------------------------------------------------------*/
void ClanAltarKnight::actAppear()
{
    oldact("{WЛучи света пронизывают комнату и в центре материализуется $o1.{x",
        obj->in_room->people, obj, 0, TO_ALL);
}

void ClanAltarKnight::actDisappear()
{
    oldact("{WСвет $o2 исчезает и он растворяется в воздухе!{x",
        obj->getRoom()->people, obj, NULL, TO_ALL);
}

void ClanAltarKnight::actNotify(Character *ch)
{
    oldact_p("{WХрамовый алтарь вашего замка был осквернен безбожниками!{x",
          ch, 0, 0, TO_CHAR, POS_DEAD);
}

/*--------------------------------------------------------------------------
 * Protector 
 *-------------------------------------------------------------------------*/
void ClanGuardKnight::actGreet(PCharacter *wch)
{
    do_say(ch, "Добро пожаловать, благородный рыцарь.");
}
void ClanGuardKnight::actPush(PCharacter *wch)
{
    oldact("$C1 кивает тебе, слегка хмурясь, взмахивает рукой.\n\r...и вот уже ты неторопливо несешься в воздухе.", wch, 0, ch, TO_CHAR);
    oldact("$C1 кивает $c3, слегка нахмурившись, взмахивает рукой.\n\r... и $c1 с диким восторгом в глазах улетает.", wch, 0, ch, TO_ROOM);
}

void ClanGuardKnight::actInvited(PCharacter *wch, Object *obj)
{
    do_say(ch, "{WНу что ж! Будь как дома - но не забывай, что ты в гостях!{x");
}

void ClanGuardKnight::actIntruder(PCharacter *)
{
    interpret_raw(ch, "cb", "БЕЗБОЖНИКИ... Безбожникам вход запрещен!");
}

void ClanGuardKnight::actGhost(PCharacter *)
{
    do_say(ch, "{WОбрети плоть и изгони дьявола из души своей сперва!{x");
}

void ClanGuardKnight::actGiveInvitation(PCharacter *wch, Object *obj)
{
    oldact("$c1 внимательно сверяется со списком.", ch, 0, 0, TO_ROOM);
    oldact("$c1 ставит Королевскую печать на $o6.", ch, obj, 0, TO_ROOM);
}

int ClanGuardKnight::getCast(Character *victim)
{
    int sn = -1;

    switch (dice(1, 16))
    {
    case 0:
    case 1:
        if (!victim->isAffected(gsn_spellbane))
            sn = gsn_dispel_affects;
        break;
    case 2:
    case 3:
        sn = gsn_acid_arrow;
        break;
    case 4:
    case 5:
        sn = gsn_caustic_font;
        break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        sn = gsn_acid_blast;
        break;
    default:
        sn = -1;
        break;
    }

    return sn;
}

/*
 * 'guard' skill command
 */

SKILL_RUNP(guard)
{
    char arg[MAX_INPUT_LENGTH];
    Character *vict;
    PCharacter *victim, *pch, *gch;
    int cnt;


    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        ch->pecho("Охранять кого?");
        return;
    }

    if ((vict = get_char_room(ch, arg)) == 0)
    {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (vict->is_npc())
    {
        oldact("$C1 не нуждается в твоей помощи!", ch, 0, vict, TO_CHAR);
        return;
    }

    victim = vict->getPC();
    pch = ch->getPC();

    if (arg_is_self(arg) || victim == pch)
    {
        if (pch->guarding == 0)
        {
            pch->pecho("Ты не можешь охранять себя же!");
            return;
        }
        else
        {
            guarding_stop(pch, pch->guarding);
            return;
        }
    }

    if (pch->guarding != 0)
    {
        pch->pecho("Но ты охраняешь кого-то другого!");
        return;
    }

    if (victim->guarded_by != 0)
    {
        oldact("$C4 уже кто-то охраняет.", pch, 0, victim, TO_CHAR);
        return;
    }

    if (!is_same_group(victim, pch))
    {
        oldact("Но ты не состоишь в той же группе, что и $C1.", pch, 0, victim, TO_CHAR);
        return;
    }

    if (IS_CHARMED(pch))
    {
        pch->pecho("Ты любишь сво%1$Gего|его|ю хозя%1$Gина|ина|йку так сильно, что не можешь охранять %2$C4!", pch->master, victim);
        return;
    }

    if (victim->fighting != 0)
    {
        pch->pecho("Почему бы тебе не позволить им сперва закончить сражение?");
        return;
    }

    if (pch->fighting != 0)
    {
        pch->pecho("Сперва закончи свое сражение, а потом беспокойся о защите кого-либо еще.");
        return;
    }

    for (gch = victim->guarding, cnt = 2; gch; gch = gch->guarding, cnt++)
        if (gch == pch)
        {
            pch->pecho("%d рыцар%s, поставленных стык-в-стык, представляют собой потрясающее зрелище!",
                        cnt, GET_COUNT(cnt, "ь", "я", "ей"));
            return;
        }

    oldact("Теперь ты охраняешь $C4.", pch, 0, victim, TO_CHAR);
    oldact("Теперь тебя охраняет $c4.", pch, 0, victim, TO_VICT);
    oldact("$c1 теперь охраняет $C4.", pch, 0, victim, TO_NOTVICT);

    pch->guarding = victim;
    victim->guarded_by = pch;
}

SKILL_APPLY(guard)
{
    int chance;
    PCharacter *pch = ch->getPC();

    if (ch->is_npc())
        return false;

    if (pch->guarded_by == 0 || pch->guarded_by->in_room != ch->in_room)
        return false;

    chance = (gsn_guard->getEffective(pch->guarded_by) -
              (int)(1.5 * (ch->getModifyLevel() - victim->getModifyLevel())));

    if (number_percent() < min(100, chance))
    {
        oldact("$c1 прыгает перед $C5!", pch->guarded_by, 0, ch, TO_NOTVICT);
        oldact("$c1 прыгает перед тобой!", pch->guarded_by, 0, ch, TO_VICT);
        oldact("Ты прыгаешь перед $C5!", pch->guarded_by, 0, ch, TO_CHAR);
        gsn_guard->improve(pch->guarded_by, true, victim);
        return true;
    }
    else
    {
        gsn_guard->improve(pch->guarded_by, false, victim);
        return false;
    }
}


SPELL_DECL(GoldenAura);
VOID_SPELL(GoldenAura)::run(Character *ch, Room *room, int sn, int level)
{
    Character *vch;
    Affect af;

    for (vch = room->people; vch != 0; vch = vch->next_in_room)
    {
        if (!is_same_group(vch, ch))
            continue;

        if (spellbane(ch, vch))
            continue;

        if (vch->isAffected(sn))
        {
            if (vch == ch)
                oldact("Ты уже окруже$gно|н|на {YЗолотой аурой{x.", ch, 0, 0, TO_CHAR);
            else
                oldact("$C1 уже окруже$Gно|н|на {YЗолотой аурой{x.", ch, 0, vch, TO_CHAR);
            continue;
        }

        af.type = sn;
        af.level = level;
        af.duration = 6 + level;

        af.bitvector.setTable(&affect_flags);
        af.bitvector.setValue(AFF_PROTECT_EVIL);        
        af.modifier = (level / 8);
        af.location = APPLY_HITROLL;
        affect_to_char(vch, &af);

        af.bitvector.setTable(&detect_flags);
        af.bitvector.setValue(DETECT_FADE|DETECT_EVIL);
        af.modifier = (0 - level / 8);
        af.location = APPLY_SAVING_SPELL;
        affect_to_char(vch, &af);

        vch->pecho("{YЗолотая аура{x окружает тебя.");
        if (ch != vch)
            oldact("{YЗолотая аура{x окружает $C4.", ch, 0, vch, TO_CHAR);
    }
}



/*-----------------------------------------------------------------
 * 'orden' command 
 *----------------------------------------------------------------*/
COMMAND(COrden, "orden")
{
    PCharacter *pch;
    const ClanOrgs *orgs;
    DLString arguments, cmd, arg;

    if (ch->is_npc())
        return;

    pch = ch->getPC();

    if (pch->getClan() != clan_knight)
    {
        pch->pecho("Ты не принадлежишь к клану Рыцарей.");
        return;
    }

    if (!(orgs = clan_knight->getOrgs()))
    {
        pch->pecho("Ордена сейчас недоступны.");
        return;
    }

    arguments = constArguments;
    cmd = arguments.getOneArgument();
    arg = arguments.getOneArgument();

    if (cmd.empty())
    {
        doUsage(pch);
    }
    else if (arg_is_list(cmd))
    {
        orgs->doList(pch);
    }
    else if (arg_is(cmd, "induct"))
    {
        if (arg_is_self(arg))
            orgs->doSelfInduct(pch, arguments);
        else
            orgs->doInduct(pch, arg);
    }
    else if (arg_is(cmd, "remove"))
    {
        if (arg_is_self(arg))
            orgs->doSelfRemove(pch);
        else
            orgs->doRemove(pch, arg);
    }
    else if (arg_is(cmd, "member"))
    {
        orgs->doMembers(pch, arg);
    }
    else
    {
        doUsage(pch);
    }
}

bool COrden::visible(Character *ch) const
{
    return !ch->is_npc() && ch->getPC()->getClan() == clan_knight;
}

void COrden::doUsage(PCharacter *pch)
{
    ostringstream buf;

    buf << "Для всех: " << endl
        << "орден{x список{x - посмотреть список групп" << endl
        << "орден{x члены{x - посмотреть список членов группы" << endl
        << endl
        << "Для руководства: " << endl
        << "орден{x члены{x [{Dгруппа{x] - посмотреть список членов своей или указанной группы" << endl
        << "орден{x принять{x {Dимя{x [{Dгруппа{x]- принять кого-то в свою или указанную группу" << endl
        << "орден{x выгнать{x {Dимя{x - выгнать кого-то из группы" << endl
        << "орден{x выгнать я{x - выйти из группы" << endl
        << "орден{x принять я{x {Dгруппа{x - принять себя в группу" << endl;
    pch->send_to(buf);
}

/*----------------------------------------------------------------------------
 * KnightOrder 
 *---------------------------------------------------------------------------*/
KnightOrder::KnightOrder()
    : classes(professionManager)
{
}

bool KnightOrder::canInduct(PCMemoryInterface *pci) const
{
    return classes.isSet(pci->getProfession());
}

const DLString &KnightOrder::getTitle(PCMemoryInterface *pci) const
{
    return titles.build(pci);
}
