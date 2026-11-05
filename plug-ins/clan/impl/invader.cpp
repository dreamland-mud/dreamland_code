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

#include "clanmobiles.h"
#include "clanrecords.h"

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
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "fight.h"
#include "loadsave.h"
#include "vnum.h"
#include "clanreference.h"
#include "magic.h"
#include "def.h"
#include "l10n.h"
#include "fenia_utils.h"

GSN(acid_arrow);
GSN(acid_blast);
GSN(blindness);
GSN(dispel_affects);
GSN(energy_drain);
GSN(fade);
GSN(golden_aura);
GSN(plague);
GSN(shadow_shroud);
GSN(soul_lust);
GSN(spellbane);
GSN(weaken);

CLAN(invader);

/*--------------------------------------------------------------------------
 * Neere 
 *-------------------------------------------------------------------------*/


SPELL_DECL(EyesOfIntrigue);
VOID_SPELL(EyesOfIntrigue)::run(Character *ch, const DLString &args, int sn, int level)
{
    Character *victim;

    if ((victim = get_char_world(ch, args, FFIND_DOPPEL)) == 0 || DIGGED(victim))
    {
        ch->pecho(_("Тьма не может обнаружить это существо."));
        return;
    }

    if (is_safe_nomessage(ch, victim))
    {
        ch->pecho(_("Тьма не может проникнуть туда, где скрывается это существо."));
        return;
    }

    if ( (victim->is_npc()) && (IS_SET(victim->act, ACT_NOEYE)) )
    {
        ch->pecho(_("Это существо защищено от твоего взора."));
        return;
    }
   
    if (victim->isAffected(gsn_golden_aura))
    {
        if (saves_spell(level, victim, DAM_OTHER, ch, DAMF_MAGIC))
        {
            ch->pecho(_("У тебя не хватает силы приказать темноте."));
            return;
        }

        victim->pecho(_("На миг тебя окружает темнота, из которой на тебя смотрит огромный глаз.\n\r"
                      "...И тихий звон {Yзолотой ауры{x рождает имя -- {D%#^C1{x."),
                      ch);
    }

    Room *was_in = ch->in_room;
    ch->in_room = victim->in_room;
    interpret_raw(ch, "look", "auto");
    ch->in_room = was_in;
}

SPELL_DECL(Shadowlife);
VOID_SPELL(Shadowlife)::run(Character *ch, Character *victim, int sn, int level)
{
    Affect af;

    if (victim->is_npc())
    {
        ch->pecho(_("Бесполезная трата сил и энергии..."));
        return;
    }

    if (ch->isAffected(sn))
    {
        ch->pecho(_("У тебя недостаточно энергии, чтобы создать тень."));
        return;
    }

    if (victim->isAffected(gsn_golden_aura) && saves_spell(level, victim, DAM_OTHER, ch, DAMF_MAGIC))
    {
        ch->pecho(_("Твое заклинание не может пробиться через защиту от заклинаний противника."));
        victim->pecho(_("Твоя золотая аура препятствует подлой попытке оживить твою тень!"));
        return;
    }

    oldact(_("Ты даешь жизнь тени $C2!"), ch, 0, victim, TO_CHAR);
    oldact(_("$c1 дает жизнь тени $C2!"), ch, 0, victim, TO_NOTVICT);
    oldact_p(_("$c1 дает жизнь твоей тени!"), ch, 0, victim, TO_VICT, POS_DEAD);

    victim->getPC()->shadow = 4 * ch->getModifyLevel() / 10;

    postaffect_to_char(ch, sn, 24);
    achievement( ch, "shadowlife" );
}

