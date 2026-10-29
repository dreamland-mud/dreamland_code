/* $Id: objcast.cpp,v 1.1.2.17.4.13 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
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
#include "skill.h"
#include "skillmanager.h"
#include "spell.h"
#include "spelltarget.h"
#include "../skills_impl/defaultspell.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"

#include "dreamland.h"
#include "terrains.h"

#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "clanreference.h"
#include "vnum.h"
#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "interp.h"
#include "morphology.h"
#include "def.h"
#include "skill_utils.h"
#include "l10n.h"
#include "magicitems.h"
#include "chargedprops.h"
#include "wearloc_utils.h"
#include "fenia_utils.h"

CLAN(battlerager);
GSN(scrolls);
GSN(wands);
GSN(staff);
GSN(none);

/*
 * Scrolls, staves and wands are all used via the 'use' command (comm/use.cpp).
 * They used to be skill commands, so the skill gate, move cost and lag that
 * DefaultSkillCommand::run applied are repeated here.
 */
static bool magic_item_skill_gate( Character *ch, Skill &skill )
{
    // The old recite/zap/brandish commands were player_only for 'order';
    // 'use' is orderable, so charmed followers are refused here instead.
    if (ch->is_npc( ) && IS_CHARMED(ch)) {
        ch->master->pecho(_("%1$^C1 не умеет пользоваться такими вещами по приказу."), ch);
        return false;
    }

    if (!skill.usable( ch )) {
        ch->pecho(_("Ты не владеешь навыком {y{hh%s{x."), skill.getNameFor( ch ).c_str( ));
        return false;
    }

    int moves = skill.getMoves( ch );
    if (moves > 0 && ch->move < moves) {
        ch->pecho(_("Ты слишком уста%Gло|л|ла для этого."), ch);
        return false;
    }

    if (moves > 0)
        ch->move -= moves;

    ch->setWait( skill.getBeats( ch ) );
    return true;
}

static void recite_one_spell(Character *ch, Object *scroll, Spell::Pointer &spell, const DLString &args,
                             int &successfulSpells, int &successfulTargets)
{
    ostringstream errBuf;
    SpellTarget::Pointer t = spell->locateTargets( ch, args, errBuf );
    if (t->castFar && t->door != -1) {
        ch->pecho(_("На таком расстоянии жертва ничего не почувствует."));
        return;
    }

    // The preposition has to agree with the scroll's name, so it is chosen once
    // per language against that language's own name. The room broadcast below
    // carries all three through a %w LangText and the formatter picks one per
    // recipient; the messages to the reciter alone just take their own.
    DLString soEn = Morphology::preposition_with(LANG_EN, scroll->getShortDescr(LANG_EN));
    DLString soRu = Morphology::preposition_with(LANG_RU, scroll->getShortDescr(LANG_RU));
    DLString soUa = Morphology::preposition_with(LANG_UA, scroll->getShortDescr(LANG_UA));
    LangText soLang { soEn.c_str(), soRu.c_str(), soUa.c_str() };
    DLString so = soLang.get(viewerLang(ch));

    if (t->error != 0) {
        switch (t->error) {
        case TARGET_ERR_CAST_ON_WHOM:
            ch->pecho(_("Ты зачитываешь одно из заклинаний %s %O2, но оно не находит, на кого подействовать."), so.c_str(), scroll);
            break;
        case TARGET_ERR_CAST_ON_WHAT:
            ch->pecho(_("Ты зачитываешь одно из заклинаний %s %O2, но оно не находит, на что подействовать."), so.c_str(), scroll);
            break;
        default:
            ch->pecho(errBuf.str());
            break;
        }

        return;
    }

    ch->recho(_("%^C1 зачитывает заклинание %w %O2."), ch, &soLang, scroll);
    ch->pecho(_("Ты зачитываешь одно из заклинаний %s %O2."), so.c_str(), scroll);

    successfulTargets++;

    if (number_percent( ) >= gsn_scrolls->getEffective( ch )) {
        oldact(_("Ты не совлада$gло|л|ла с произношением."), ch, 0, 0, TO_CHAR);
        gsn_scrolls->improve( ch, false );
        return;
    }

    gsn_scrolls->improve( ch, true );

    try {
        bitstring_t recite_flags = FSPELL_BANE|FSPELL_CHECK_SAFE|FSPELL_ATTACK_CASTER;

        int slevel;
        slevel = scroll->value0() + skill_level_bonus(*gsn_scrolls, ch);
      
        if (::spell_nocatch(spell, slevel, ch, t, recite_flags)) {
            successfulSpells++;
        }

    } catch (const VictimDeathException &vde) {
        return;
    }
}

void use_scroll( Character *ch, Object *scroll, const DLString &args )
{
    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->pecho(_("Какие еще свитки-шмитки?! Ты же вои{Smн{Sfтельница{Sx клана Ярости, а не презренный МАГ!"));
        return;
    }

    if (get_wear_level( ch, scroll ) > ch->getRealLevel()) {
        ch->pecho(_("Этот свиток чересчур сложен для твоего понимания."));
        return;
    }

    if (!magic_item_skill_gate( ch, *gsn_scrolls ))
        return;
    
    int successfulSpells = 0, successfulTargets = 0;
    int totalSpells = 0;

    for (int i = 1; i <= 4; i++) {
        int sn = scroll->valueByIndex(i);
        if (sn < 0)
            continue;

        Skill *skill = SkillManager::getThis( )->find( sn );
        if (!skill || skill->getIndex() == gsn_none)
            continue;

        Spell::Pointer spell = skill->getSpell( );
        if (!spell) {
            LogStream::sendWarning() << "Scroll [" << scroll->pIndexData->vnum << "] has invalid spell for " << i << endl;
            continue;
        }

        totalSpells++;
        recite_one_spell(ch, scroll, spell, args, successfulSpells, successfulTargets);
    }

    if (successfulSpells > 0) {
        ch->pecho(_("%^O1 превращается в пыль."), scroll);
        extract_obj( scroll );
    }
}



static bool zap_cast( Character *ch, Object *wand, Spell::Pointer &spell, int slevel,
                      Skill &skill, const DLString &argument, bool hasCharge );

/*
 * Legacy staves roll the 'staff' weapon skill. Spells on a character or an object
 * take a target like a wand; room-wide and targetless spells strike the ground.
 */
void use_staff( Character *ch, Object *staff, const DLString &argument )
{
    int sn;
    Skill::Pointer skill;
    Spell::Pointer spell;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->pecho(_("Палками махать?! Ты же вои{Smн{Sfтельница{Sx клана Ярости, а не презренный МАГ!"));
        return;
    }

    if (!magic_item_skill_gate( ch, *gsn_staff ))
        return;

    sn = staff->value3();
    skill = SkillManager::getThis( )->find( sn );
    
    if (!skill || !( spell = skill->getSpell( ) )) { 
        bug( "staff: bad sn %d.", sn );
        return;
    }

    int level = staff->value0() + skill_level_bonus(*gsn_staff, ch);
    int t = spell->getTarget( );

    if (!IS_SET( t, TAR_IGNORE|TAR_CREATE_MOB|TAR_CREATE_OBJ|TAR_ROOM|TAR_PEOPLE )) {
        if (!zap_cast( ch, staff, spell, level, *gsn_staff, argument, staff->value2() > 0 ))
            return;
    }
    else if (staff->value2() > 0) {
        const char *terrain = terrains[ch->in_room->getSectorType()].hit;
        
        oldact(_("$c1 ударяет $o5 $T."), ch, staff, terrain, TO_ROOM );
        oldact(_("Ты ударяешь $o5 $T."), ch, staff, terrain, TO_CHAR );

        if ( number_percent( ) >= gsn_staff->getEffective( ch ))
        {
            oldact(_("Ты не смо$gгло|г|гла активировать $o4."),ch,staff,0,TO_CHAR);
            oldact(_("...и ничего не происходит."),ch,0,0,TO_ROOM);
            gsn_staff->improve( ch, false );
        }
        else {
            try {
                if (IS_SET( t, TAR_IGNORE|TAR_CREATE_MOB|TAR_CREATE_OBJ ))
                    ::spell_nocatch(spell, level, ch, SpellTarget::Pointer(NEW, DLString::emptyString), 0);
                else
                    ::spell_nocatch(spell, level, ch, SpellTarget::Pointer(NEW, ch->in_room), 0);
            } catch (const VictimDeathException &e) {
            }

            gsn_staff->improve( ch, true );
        }
    }

    if (staff->extracted)
        return;

    staff->value2(staff->value2() - 1);
    if (staff->value2() <= 0) {
        ch->recho( _("%1$^O1 %2$C2 темне%1$nет|ют и исчеза%1$nет|ют."), staff, ch );
        ch->pecho( _("Тво%1$Gе|й|я|и %1$O1 темне%1$nет|ют и исчеза%1$nет|ют."), staff );
        extract_obj( staff );
    }
}


/*
 * The wand cast, shared by legacy wands and charged items. Returns false when no
 * target was found: then no charge is spent. A failed skill roll still spends it.
 */
static bool zap_cast( Character *ch, Object *wand, Spell::Pointer &spell, int slevel,
                      Skill &skill, const DLString &argument, bool hasCharge )
{
    std::basic_ostringstream<char> buf;
    DLString args = argument;
    SpellTarget::Pointer target = spell->locateTargets( ch, args, buf );

    if (target->error != 0) {
        switch (target->error) {
        case TARGET_ERR_CAST_ON_WHAT:
            ch->pecho(_("Взмахнуть жезлом на что?"));
            break;
        case TARGET_ERR_CAST_ON_WHOM:
            ch->pecho(_("Взмахнуть жезлом на кого именно?"));
            break;
        default:
            ch->pecho(buf.str());
            break;
        }

        return false;
    }
        
    if (target->castFar && target->door != -1) {
        ch->pecho(_("Твой жезл не дотягивается до соседней комнаты."));
        return false;
    }
    
    if (!hasCharge)
        return true;

    Character *victim = target->victim;

    if (victim && victim->in_room == ch->in_room) {
        if (ch != victim) {
            oldact(_("$c1 взмахивает $o5 на тебя!"), ch, wand, victim, TO_VICT );
            oldact(_("$c1 взмахивает $o5 на $C4."), ch, wand, victim, TO_NOTVICT );
            oldact(_("Ты взмахиваешь $o5 на $C4."), ch, wand, victim, TO_CHAR );
        }
        else {
            oldact(_("$c1 взмахивает $o5 на себя."), ch, wand, 0, TO_ROOM );
            oldact(_("Ты взмахиваешь $o5 на себя."), ch, wand, 0, TO_CHAR );
        }
    }
    else if (target->obj && target->obj->getRoom( ) == ch->in_room) {
        oldact(_("$c1 взмахивает $o5 на $O4."), ch, wand, target->obj, TO_ROOM );
        oldact(_("Ты взмахиваешь $o5 на $O4."), ch, wand, target->obj, TO_CHAR );
    }
    else {
        oldact(_("$c1 взмахивает $o5."), ch, wand, 0, TO_ROOM );
        oldact(_("Ты взмахиваешь $o5."), ch, wand, 0, TO_CHAR );
    }

    if (number_percent() >= skill.getEffective( ch )) {
        oldact_p(_("Твои усиленные манипуляции с $o5 приводят лишь к дыму и искрам."),
                ch,wand,0,TO_CHAR,POS_RESTING);
        oldact_p(_("Усиленные манипуляции $c2 с $o5 приводят лишь к дыму и искрам."),
                ch,wand,0,TO_ROOM,POS_RESTING);
        skill.improve( ch, false, victim );
    }
    else {
        try {
            bitstring_t zap_flags = FSPELL_CHECK_SAFE|FSPELL_BANE|FSPELL_ATTACK_CASTER;
            
            if (::spell_nocatch(spell, slevel, ch, target, zap_flags))
                if (spell->getSpellType( ) == SPELL_OFFENSIVE)
                    yell_panic( ch, victim,
                                "Помогите! Кто-то размахивает около меня волшебным жезлом!",
                                "Помогите! %1$^C1 пугает меня своим жезлом!" );

        } catch (const VictimDeathException &e) {
        }

        skill.improve( ch, true, victim );
    }

    return true;
}

void use_wand( Character *ch, Object *wand, const DLString &argument )
{
    int sn;
    Skill::Pointer skill;
    Spell::Pointer spell;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->pecho(_("Ты долж%1$Gно|ен|на уничтожать магию, а не использовать её!"), ch);
        return;
    }

    // Wands work from the inventory now, so the wear-level check no longer covers them.
    if (get_wear_level( ch, wand ) > ch->getRealLevel( )) {
        ch->pecho(_("%1$^O1 чересчур слож%1$Gно|ен|на|ны для твоего понимания."), wand);
        return;
    }

    if (!magic_item_skill_gate( ch, *gsn_wands ))
        return;

    sn = wand->value3();
    skill = SkillManager::getThis( )->find( sn );

    if (!skill || !( spell = skill->getSpell( ) )) {
        bug( "zap: bad sn %d", sn );
        return;
    }

    int slevel = wand->value0() + skill_level_bonus(*gsn_wands, ch);

    if (!zap_cast( ch, wand, spell, slevel, *gsn_wands, argument, wand->value2() > 0 ))
        return;

    wand->value2(wand->value2() - 1);
    if (wand->value2() <= 0) {
        ch->recho( _("%1$^O1 %2$C2 развалива%1$nется|ются на куски."), wand, ch );
        ch->pecho( _("Тво%1$Gе|й|я|и %1$O1 развалива%1$nется|ются на куски."), wand );
        extract_obj( wand );
    }
}

/*
 * Charged items (props "charged", see chargedprops.h): a wand that can be any item.
 * "carried" charges work from the inventory or any wear slot and roll 'wands';
 * "wielded" ones (staff weapons) need the item wielded and roll 'staff'.
 */
void use_charged( Character *ch, Object *obj, const DLString &argument )
{
    const Json::Value *c = charged_props( obj );
    bool wielded = charged_wielded( *c );
    Skill &skill = wielded ? *gsn_staff : *gsn_wands;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->pecho(_("Ты долж%1$Gно|ен|на уничтожать магию, а не использовать её!"), ch);
        return;
    }

    if (wielded && obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield) {
        ch->pecho(_("%1$^O4 сперва нужно вооружиться."), obj);
        return;
    }

    if (get_wear_level( ch, obj ) > ch->getRealLevel( )) {
        ch->pecho(_("%1$^O1 чересчур слож%1$Gно|ен|на|ны для твоего понимания."), obj);
        return;
    }

    if (charged_charges( *c ) <= 0) {
        ch->pecho(_("В %1$O6 не осталось ни одного заряда."), obj);
        return;
    }

    DLString spellName = (*c).isMember( "spell" ) && (*c)["spell"].isString( ) ? (*c)["spell"].asString( ) : "";
    Skill *spellSkill = skillManager->findExisting( spellName );
    Spell::Pointer spell;
    if (!spellSkill || !( spell = spellSkill->getSpell( ) )) {
        LogStream::sendWarning( ) << "Charged item [" << obj->pIndexData->vnum << "] has bad spell "
                                  << spellName << endl;
        return;
    }

    if (!magic_item_skill_gate( ch, skill ))
        return;

    int level = charged_int( *c, "level", obj->level );
    int slevel = level + skill_level_bonus( skill, ch );
    bool crumble = charged_flag( *c, "crumble" );

    if (!zap_cast( ch, obj, spell, slevel, skill, argument, true ))
        return;

    // The cast may have killed the user or destroyed the item.
    if (obj->extracted)
        return;

    Json::Value &own = charged_own( obj );
    int left = charged_charges( own ) - 1;
    own["charges"] = left;

    if (left > 0)
        return;

    if (crumble) {
        ch->recho( _("%1$^O1 %2$C2 развалива%1$nется|ются на куски."), obj, ch );
        ch->pecho( _("Тво%1$Gе|й|я|и %1$O1 развалива%1$nется|ются на куски."), obj );
        extract_obj( obj );
        if (!ch->is_npc( ))
            achievement( ch, "last_drop" );
    }
    else {
        ch->pecho( _("%1$^O1 тускне%1$nет|ют: заряды кончились."), obj );
    }
}
