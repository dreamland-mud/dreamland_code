/* $Id: invader.cpp,v 1.1.6.7.6.20 2010-09-01 21:20:44 rufina Exp $
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

#include "invader.h"
#include "clanorg.h"

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "act.h"

#include "merc.h"

#include "fight.h"
#include "handler.h"
#include "vnum.h"
#include "clanreference.h"
#include "magic.h"
#include "def.h"

GSN(acid_arrow);
GSN(acid_blast);
GSN(blindness);
GSN(dispel_affects);
GSN(energy_drain);
GSN(evil_spirit);
GSN(fade);
GSN(golden_aura);
GSN(mental_attack);
GSN(mental_knife);
GSN(plague);
GSN(shadow_cloak);
GSN(shadow_shroud);
GSN(soul_lust);
GSN(spellbane);
GSN(weaken);

CLAN(invader);

/*--------------------------------------------------------------------------
 * Neere 
 *-------------------------------------------------------------------------*/
void ClanGuardInvader::actGreet(PCharacter *wch)
{
    say_fmt("Приветствую тебя, идущ%2$Gее|ий|ая по Пути Тьмы.", ch, wch);
}
void ClanGuardInvader::actPush(PCharacter *wch)
{
    oldact("$C1 зверски ухмыляется тебе...\n\rТы теряешь рассудок от страха и куда-то несешься.", wch, 0, ch, TO_CHAR);
    oldact("$C1 сверлит глазами $c4, и $c1 с испугу куда-то уносится.", wch, 0, ch, TO_ROOM);
}
int ClanGuardInvader::getCast(Character *victim)
{
    int sn = -1;

    switch (dice(1, 16))
    {
    case 0:
    case 1:
        sn = gsn_blindness;
        break;
    case 2:
    case 3:
        if (!victim->isAffected(gsn_spellbane))
            sn = gsn_dispel_affects;
        break;
    case 4:
    case 5:
        sn = gsn_weaken;
        break;
    case 6:
    case 7:
        sn = gsn_energy_drain;
        break;
    case 8:
    case 9:
        sn = gsn_plague;
        break;
    case 10:
    case 11:
        sn = gsn_acid_arrow;
        break;
    case 12:
    case 13:
    case 14:
        sn = gsn_acid_blast;
        break;
    case 15:
        if (ch->hit < (ch->max_hit / 3))
            sn = gsn_shadow_cloak;
        else
            sn = -1;
        break;
    default:
        sn = -1;
        break;
    }

    return sn;
}

/*
 * 'fade' skill command
 */

SKILL_RUNP(fade)
{
   
    if (MOUNTED(ch))
    {
        ch->pecho("Ты не можешь спрятаться в тенях, когда в седле.");
        return;
    }

    if (RIDDEN(ch))
    {
        ch->pecho("Ты не можешь спрятаться в тенях, когда ты оседлан%Gо||а.", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_FAERIE_FIRE))
    {
        ch->pecho("Ты не можешь спрятаться в тенях, когда светишься.");
        return;
    }

 
    int k = ch->getLastFightDelay();

    if (k >= 0 && k < FIGHT_DELAY_TIME)
        k = k * 100 / FIGHT_DELAY_TIME;
    else
        k = 100;

    if (number_percent() < gsn_fade->getEffective(ch) * k / 100)
    {
        SET_BIT(ch->affected_by, AFF_FADE);
        ch->pecho("Ты прячешься в тенях.");        
        gsn_fade->improve(ch, true);
    }
    else {        
        ch->pecho("Ты пытаешься спрятаться в тенях, но безуспешно.");        
        gsn_fade->improve(ch, false);
    }
    return;
}

SPELL_DECL(EyesOfIntrigue);
VOID_SPELL(EyesOfIntrigue)::run(Character *ch, const DLString &args, int sn, int level)
{
    Character *victim;

    if ((victim = get_char_world(ch, args, FFIND_DOPPEL)) == 0 || DIGGED(victim))
    {
        ch->pecho("Тьма не может обнаружить это существо.");
        return;
    }

    if (is_safe_nomessage(ch, victim))
    {
        ch->pecho("Тьма не может проникнуть туда, где скрывается это существо.");
        return;
    }

    if ( (victim->is_npc()) && (IS_SET(victim->act, ACT_NOEYE)) )
    {
        ch->pecho("Это существо защищено от твоего взора.");
        return;
    }
   
    if (victim->isAffected(gsn_golden_aura))
    {
        if (saves_spell(level, victim, DAM_OTHER, ch, DAMF_MAGIC))
        {
            ch->pecho("У тебя не хватает силы приказать темноте.");
            return;
        }

        victim->pecho("На миг тебя окружает темнота, из которой на тебя смотрит огромный глаз.\n\r"
                      "...И тихий звон {Yзолотой ауры{x рождает имя -- {D%#^C1{x.",
                      ch);
    }

    do_look_auto(ch, victim->in_room);
}

SPELL_DECL(ShadowCloak);
VOID_SPELL(ShadowCloak)::run(Character *ch, Character *victim, int sn, int level)
{
    Affect af;
    DLString msgChar, msgVict, orgCh, orgVict;

    if (ch->is_npc() || victim->is_npc() || ch->getClan() != victim->getClan())
    {
        ch->pecho("Это заклинание ты можешь произнести только на члена твоего клана.");
        return;
    }

    orgCh = ClanOrgs::getAttr(ch->getPC());
    orgVict = ClanOrgs::getAttr(victim->getPC());

    if (orgCh != orgVict)
    {
        ch->pecho("Это заклинание ты можешь произнести только на члена твоей организации.");
        return;
    }

    if (victim->isAffected(gsn_soul_lust))
    {
        ch->pecho(ch == victim ? "Жажда душ уже горит в тебе." : "Жажда душ уже горит в %C6.", victim);
        return;
    }

    if (victim->isAffected(sn) || victim->isAffected(gsn_shadow_shroud))
    {
        ch->pecho(ch == victim ? "Призрачная мантия уже защищает тебя." : "Призрачная мантия уже защищает %C4.", victim);
        return;
    }   

    if (gsn_shadow_shroud->usable(ch))
    {
        msgVict = "Призрачная мантия окутывает тебя. Ты погружаешься во тьму.";
        msgChar = "%2$C1 окутывается тьмой.";
        sn = gsn_shadow_shroud;
    }
    else if (gsn_soul_lust->usable(ch))
    {
        msgVict = "В тебе загорается огонь, жаждущий душ ангелов.";
        msgChar = "В %2$C6 загорается огонь, жаждущий душ ангелов.";
        sn = gsn_soul_lust;
    }
    else
    {
        msgVict = "Призрачная мантия окутывает тебя.";
        msgChar = "Призрачная мантия окутывает %2$C4.";
    }

    af.type = sn;
    af.level = level;
    af.duration = 24;

    af.bitvector.setTable(&detect_flags);
    af.bitvector.setValue(DETECT_GOOD | DETECT_FADE);
    af.location = APPLY_SAVING_SPELL;
    af.modifier = (0 - level / 9);
    affect_to_char(victim, &af);

    af.bitvector.setTable(&affect_flags);
    af.bitvector.setValue(IS_AFFECTED(victim, AFF_PROTECT_GOOD) ? 0 : AFF_PROTECT_GOOD);
    af.modifier = (-level * 5 / 2);
    af.location = APPLY_AC;
    affect_to_char(victim, &af);

    victim->pecho(msgVict.c_str(), ch, victim);
    if (ch != victim)
        ch->pecho(msgChar.c_str(), ch, victim);
}

SPELL_DECL(Shadowlife);
VOID_SPELL(Shadowlife)::run(Character *ch, Character *victim, int sn, int level)
{
    Affect af;

    if (victim->is_npc())
    {
        ch->pecho("Бесполезная трата сил и энергии...");
        return;
    }

    if (ch->isAffected(sn))
    {
        ch->pecho("У тебя недостаточно энергии, чтобы создать тень.");
        return;
    }

    if (victim->isAffected(gsn_golden_aura) && saves_spell(level, victim, DAM_OTHER, ch, DAMF_MAGIC))
    {
        ch->pecho("Твое заклинание не может пробиться через защиту от заклинаний противника.");
        victim->pecho("Твоя золотая аура препятствует подлой попытке оживить твою тень!");
        return;
    }

    oldact("Ты даешь жизнь тени $C2!", ch, 0, victim, TO_CHAR);
    oldact("$c1 дает жизнь тени $C2!", ch, 0, victim, TO_NOTVICT);
    oldact_p("$c1 дает жизнь твоей тени!", ch, 0, victim, TO_VICT, POS_DEAD);

    victim->getPC()->shadow = 4 * ch->getModifyLevel() / 10;

    postaffect_to_char(ch, sn, 24);
}

AFFECT_DECL(EvilSpirit);
VOID_AFFECT(EvilSpirit)::update(Character *ch, Affect *paf)
{
    DefaultAffectHandler::update(ch, paf);

    if (ch->getClan() == clan_invader)
        return;

    Spell::Pointer spell = gsn_mental_knife->getSpell();

    if (spell)
        spell->run(ch, ch, gsn_mental_attack, ch->getModifyLevel());
}

VOID_AFFECT(EvilSpirit)::update(Room *room, Affect *paf)
{
    Affect af;
    Character *vch;

    af.type = gsn_evil_spirit;
    af.level = paf->level;
    af.duration = number_range(1, (af.level / 30));

    for (vch = room->people; vch; vch = vch->next_in_room)
    {
        if (!saves_spell(vch->getModifyLevel() + 2, vch, DAM_MENTAL, 0, DAMF_MAGIC) && !vch->is_immortal() && !is_safe_rspell(vch->getModifyLevel() + 2, vch) && !vch->isAffected(gsn_evil_spirit) && number_bits(3) == 0)
        {
            vch->pecho("Злые духи овладевают тобой.");
            oldact("Злые духи овладевают $c1.", vch, 0, 0, TO_ROOM);
            affect_join(vch, &af);
        }
    }
}

VOID_AFFECT(EvilSpirit)::toStream(ostringstream &buf, Affect *paf)
{
    buf << fmt(0, "Злые духи воцарились здесь на {W%1$d{x ча%1$Iс|са|сов.", paf->duration)
        << endl;
}

/*-----------------------------------------------------------------
 * 'darkleague' command 
 *----------------------------------------------------------------*/
COMMAND(CDarkLeague, "darkleague")
{
    PCharacter *pch;
    const ClanOrgs *orgs;
    DLString arguments, cmd, arg;

    if (ch->is_npc())
        return;

    pch = ch->getPC();

    if (pch->getClan() != clan_invader)
    {
        pch->pecho("Ты не принадлежишь к Кабалу Захватчиков.");
        return;
    }

    if (!(orgs = clan_invader->getOrgs()))
    {
        pch->pecho("Попробуй позже.");
        return;
    }

    arguments = constArguments;
    cmd = arguments.getOneArgument();
    arg = arguments;

    if (cmd.empty() || arg_is_help(cmd))
    {
        doUsage(pch);
        return;
    }

    if (arg_is_list(cmd))
    {
        orgs->doList(pch);
        return;
    }

    if (!pch->getClan()->isRecruiter(pch))
    {
        pch->pecho("Твоих полномочий хватает только посмотреть список организаций.");
        return;
    }

    if (arg_oneof(cmd, "induct", "принять"))
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
        orgs->doMembers(pch, arg);
    }
    else
    {
        doUsage(pch);
    }
}

bool CDarkLeague::visible(Character *ch) const
{
    return !ch->is_npc() && ch->getPC()->getClan() == clan_invader;
}

void CDarkLeague::doUsage(PCharacter *pch)
{
    ostringstream buf;

    buf << "Для всех: " << endl
        << "{lEdarkleague{lRтемнаялига{x {lElist{lRсписок{x - посмотреть список групп" << endl
        << "{lEdarkleague{lRтемнаялига{x {lEmembers{lRчлены{x - посмотреть список членов группы" << endl
        << endl
        << "Для руководства: " << endl
        << "{lEdarkleague{lRтемнаялига{x {lEmembers{lRчлены{x [{Dгруппа{x] - посмотреть список членов своей или указанной группы" << endl
        << "{lEdarkleague{lRтемнаялига{x {lEinduct{lRпринять{x {Dимя{x [{Dгруппа{x]- принять кого-то в свою или указанную группу" << endl
        << "{lEdarkleague{lRтемнаялига{x {lEremove{lRвыгнать{x {Dимя{x - выгнать кого-то из группы" << endl
        << "{lEdarkleague{lRтемнаялига{x {lEremove self{lRвыгнать я{x - выйти из группы" << endl
        << "{lEdarkleague{lRтемнаялига{x {lEinduct self{lRпринять я{x {Dгруппа{x - принять себя в группу" << endl;

    pch->send_to(buf);
}


