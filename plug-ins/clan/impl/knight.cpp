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
#include "gsn_plugin.h"
#include "merc.h"
#include "vnum.h"
#include "mercdb.h"
#include "fight.h"
#include "weapongenerator.h"
#include "act_move.h"
#include "handler.h"
#include "magic.h"
#include "def.h"

GSN(dispel_affects);
CLAN(knight);

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

    if (!gsn_guard->available(ch))
    {
        ch->pecho("Ась?");
        return;
    }

    if (!gsn_guard->usable(ch))
        return;

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

    if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == pch)
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
            pch->printf("%d рыцар%s, поставленных стык-в-стык, представляют собой потрясающее зрелище!\r\n",
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

SPELL_DECL(Dragonplate);
VOID_SPELL(Dragonplate)::run(Character *ch, char *target_name, int sn, int level)
{
    int plate_vnum;
    Object *plate;
    Affect af;

    plate_vnum = OBJ_VNUM_PLATE;

    plate = create_object(get_obj_index(plate_vnum), level + 5);
    plate->timer = 2 * level;
    plate->cost = 0;
    plate->level = ch->getRealLevel();

    af.type = sn;
    af.level = level;
    af.duration = -1;
    af.modifier = (level / 8);

    af.location = APPLY_HITROLL;
    affect_to_obj(plate, &af);

    af.location = APPLY_DAMROLL;
    affect_to_obj(plate, &af);

    obj_to_char(plate, ch);

    oldact("Ты взмахиваешь руками и создаешь $o4!", ch, plate, 0, TO_CHAR);
    oldact("$c1 взмахивает руками и создает $o4!", ch, plate, 0, TO_ROOM);
}

/*
 * golden weapon behavior
 */
bool KnightWeapon::death(Character *ch)
{
    bool wielded;

    wielded = (obj->wear_loc == wear_wield || obj->wear_loc == wear_second_wield);

    oldact_p("Твое золотое оружие исчезает.", ch, 0, 0, TO_CHAR, POS_DEAD);
    oldact("Золотое оружие $c2 исчезает.", ch, 0, 0, TO_ROOM);
    extract_obj(obj);

    if (!wielded || ch->is_npc() || chance(80))
        return false;

    ch->hit = 1;

    for (auto &paf: ch->affected.clone())
        affect_remove(ch, paf);

    ch->unsetLastFightTime();
    SET_DEATH_TIME(ch);
    return true;
}

void KnightWeapon::fight(Character *ch)
{
    int sn = -1;

    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
        return;

    if (chance(6))
        sn = gsn_heal;

    if (sn > 0)
    {
        oldact("$o1 загорается ярким голубым светом!", ch, obj, 0, TO_CHAR);
        oldact("$o1 $c2 загорается ярким голубым светом!", ch, obj, 0, TO_ROOM);

        spell(sn, ch->getModifyLevel(), ch, ch, FSPELL_BANE);
    }
}

/*
 * 'dragonsword' spell 
 */
SPELL_DECL(Dragonsword);
VOID_SPELL(Dragonsword)::run(Character *ch, char *target_name, int sn, int level)
{
    int sword_vnum;
    Object *sword;
    char arg[MAX_INPUT_LENGTH];

    target_name = one_argument(target_name, arg);

    if (arg_oneof(arg, "sword", "меч"))
        sword_vnum = OBJ_VNUM_DRAGONSWORD;
    else if (arg_oneof(arg, "mace", "булава"))
        sword_vnum = OBJ_VNUM_DRAGONMACE;
    else if (arg_oneof(arg, "dagger", "кинжал", "нож"))
        sword_vnum = OBJ_VNUM_DRAGONDAGGER;
    else if (arg_oneof(arg, "lance", "пика"))
        sword_vnum = OBJ_VNUM_DRAGONLANCE;
    else
    {
        ch->pecho("Какое именно {YОружие Золотого Дракона{x ты хочешь создать: меч, булаву, кинжал или пику?");
        return;
    }

    sword = create_object(get_obj_index(sword_vnum), level);
    sword->timer = level * 2;
    sword->cost = 0;
    sword->level = ch->getModifyLevel();

    WeaponGenerator()
        .item(sword)
        .valueTier(2)
        .hitrollTier(2)
        .damrollTier(3)
        .assignValues()
        .assignHitroll()
        .assignDamroll();

    SET_BIT(sword->extra_flags, (ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
    obj_to_char(sword, ch);

    oldact("Ты взмахиваешь руками и создаешь $o4!", ch, sword, 0, TO_CHAR);
    oldact("$c1 взмахивает руками и создает $o4!", ch, sword, 0, TO_ROOM);
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

SPELL_DECL(HolyArmor);
VOID_SPELL(HolyArmor)::run(Character *ch, Character *, int sn, int level)
{
    Affect af;

    if (ch->isAffected(sn))
    {
        ch->pecho("Священные силы уже защищают тебя от повреждений.");
        return;
    }

    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = (-max(10, 10 * (level / 5)));
    affect_to_char(ch, &af);
    oldact_p("Священные силы защищают $c4 от повреждений.",
          ch, 0, 0, TO_ROOM, POS_RESTING);
    ch->pecho("Священные силы защищают тебя от повреждений.");
}

SPELL_DECL_T(Squire, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, Squire)::createMobile(Character *ch, int level) const
{
    NPCharacter *mob = createMobileAux(ch, ch->getModifyLevel(),
                                       ch->max_hit, ch->max_mana,
                                       number_range(level / 20, level / 15),
                                       number_range(level / 4, level / 3),
                                       number_range(level / 10, level / 8));

    mob->setLongDescr(fmt(ch, mob->getLongDescr(), ch));
    mob->setDescription(fmt(ch, mob->getDescription(), ch));
    return mob;
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
    else if (arg_oneof(cmd, "induct", "принять"))
    {
        if (arg_is_self(arg))
            orgs->doSelfInduct(pch, arguments);
        else
            orgs->doInduct(pch, arg);
    }
    else if (arg_oneof(cmd, "remove", "выгнать", "уйти"))
    {
        if (arg_is_self(arg))
            orgs->doSelfRemove(pch);
        else
            orgs->doRemove(pch, arg);
    }
    else if (arg_oneof(cmd, "members", "члены"))
    {
        orgs->doMembers(pch);
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
        << "{Worden list{x        - посмотреть список орденов" << endl
        << "{Worden members{x     - посмотреть список членов ордена" << endl
        << "{Worden remove self{x - выйти из ордена" << endl
        << endl
        << "Для рекруитеров: " << endl
        << "{Worden induct <{xname{W>{x - принять кого-либо в орден" << endl
        << "{Worden remove <{xname{W>{x - выгнать кого-либо из ордена" << endl
        << endl
        << "Для лидера: " << endl
        << "{Worden induct self <{xorden name{W>{x - войти в указанный орден" << endl;

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
