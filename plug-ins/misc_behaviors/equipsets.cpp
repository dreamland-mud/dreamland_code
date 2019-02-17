#include "equipsets.h"
#include <map>

#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "character.h"
#include "object.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "magic.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "handler.h"
#include "fight.h"
#include "damage_impl.h"
#include "def.h"
   
GSN(sidhe_armor);
GSN(travellers_joy);
GSN(myrvale_noriva);
GSN(shevale_reykaris);
PROF(cleric);
PROF(necromancer);

/*--------------------------------------------------------------------------
 * EquipSet
 *-------------------------------------------------------------------------*/
EquipSet::EquipSet(int size, bool noDoubleNeck_, bool noDoubleWrist_) 
                      : totalSetSize(size), noDoubleNeck(noDoubleNeck_), noDoubleWrist(noDoubleWrist_)
{
}
 
void EquipSet::equip( Character *victim )
{
    if (victim->is_npc())
        return;

    if (hasAffect(victim)) 
        return;

    if (!isComplete(victim)) 
        return;

    addAffect(victim);    
}

void EquipSet::remove( Character *victim )
{
    if (victim->is_npc())
        return;

    if (!hasAffect(victim))
        return;

    if (isComplete(victim))
        return;

    removeAffect(victim);
}

bool EquipSet::isComplete(Character *ch) const
{
    GlobalArray slots(wearlocationManager);
    int wornSetSize = 0;

    if (ch->is_npc())
        return false;
    
    if (totalSetSize <= 0)
        return false;

    for (Object *item = ch->carrying; item; item = item->next_content) {
        if (item->wear_loc == wear_none)
            continue;
        if (!item->behavior)
            continue;
        if (item->behavior->getType() != obj->behavior->getType())
            continue;

        slots[item->wear_loc] = item->pIndexData->vnum;
        wornSetSize++;
    }

    if (noDoubleNeck) {
        int neck1 = slots[wear_neck_1];
        int neck2 = slots[wear_neck_2];
        if (neck1 != 0 && neck1 == neck2)
            wornSetSize--;
    }

    if (noDoubleWrist) {
        int wrist1 = slots[wear_wrist_l];
        int wrist2 = slots[wear_wrist_r];
        if (wrist1 != 0 && wrist1 == wrist2)
            wornSetSize--;
    }
    
    if (ch->is_immortal())
        ch->pecho("Окончательный результат %d предмета из %d.", wornSetSize, totalSetSize);

    return wornSetSize >= totalSetSize;
}

/*--------------------------------------------------------------------------
 * SidheArmorSet
 *-------------------------------------------------------------------------*/
SidheArmorSet::SidheArmorSet()
                : EquipSet(5, true, true), sn(gsn_sidhe_armor)
{
}

bool SidheArmorSet::hasAffect(Character *ch) const
{
    return ch->isAffected(sn);
}

void SidheArmorSet::addAffect(Character *ch) const
{
    if (IS_NEUTRAL(ch)) {
        ch->pecho("Набор сидхийской брони на мгновение загорается тусклым светом и тут же гаснет.");
        return;
    }

    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;

    af.where     = TO_AFFECTS;
    af.bitvector = IS_EVIL(ch) ? AFF_PROTECT_GOOD : AFF_PROTECT_EVIL;

    af.modifier  =  20;
    af.location  = APPLY_HITROLL;
    affect_to_char(ch, &af);

    af.where     = TO_RESIST;
    af.bitvector = ch->getRace()->getVuln().isSet(VULN_IRON) ? RES_IRON : 0;
    af.modifier  =  30;
    af.location  = APPLY_DAMROLL;
    affect_to_char(ch, &af);

    af.where     = TO_VULN;
    af.bitvector = IS_EVIL(ch) ? VULN_HOLY : VULN_NEGATIVE;
    af.modifier  =  200;
    af.location  = APPLY_HIT;
    affect_to_char(ch, &af);

    ch->pecho("{WНа{wб{Wо{wр{W си{wдх{Wийск{wо{Wй б{wр{Wони на{wч{Wина{wет{W светиться ровным се{wр{Wе{wбр{Wист{wы{Wм светом.{x");
}

void SidheArmorSet::removeAffect(Character *ch) const
{
    ch->pecho("Свет, окружавший твою сидхийскую броню, мерцает и гаснет.");
    affect_strip(ch, sn);
}

/*--------------------------------------------------------------------------
 * TravellersJoySet 
 *-------------------------------------------------------------------------*/
TravellersJoySet::TravellersJoySet()
                : EquipSet(4, true, true), sn(gsn_travellers_joy)
{
}

bool TravellersJoySet::hasAffect(Character *ch) const
{
    return ch->isAffected(sn);
}

void TravellersJoySet::addAffect(Character *ch) const
{
    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;
    af.where     = TO_AFFECTS;
    af.bitvector = 0;

    af.modifier  =  8;
    af.location  = APPLY_HITROLL;
    affect_to_char(ch, &af);

    af.modifier  =  12;
    af.location  = APPLY_DAMROLL;
    affect_to_char(ch, &af);

    af.modifier  =  35;
    af.location  = APPLY_HIT;
    affect_to_char(ch, &af);

    af.modifier  =  2;
    af.location  = APPLY_DEX;
    affect_to_char(ch, &af);

    af.modifier  =  2;
    af.location  = APPLY_STR;
    affect_to_char(ch, &af);

    af.modifier  =  2;
    af.location  = APPLY_CON;
    affect_to_char(ch, &af);

    ch->pecho("{CКомплект одежд путешественника начитает светиться ровным голубоватым светом.{x");
}

void TravellersJoySet::removeAffect(Character *ch) const
{
    ch->pecho("Свет, окружавший комплект одежд путешественника, мерцает и гаснет.");
    affect_strip(ch, sn);
}

void TravellersJoySet::fight( Character *ch )
{
    if (!ch->isAffected(sn))
        return;

    if (chance(4) && HEALTH(ch) < 75) {
        ch->pecho("{CКомплект одежд путешественника на мгновение вспыхивает ярким голубым светом.{x");
        spell( gsn_cure_critical, ch->getModifyLevel(), ch, ch );
    }
}

/*--------------------------------------------------------------------------
 * NorivaMyrvaleSet 
 *-------------------------------------------------------------------------*/
NorivaMyrvaleSet::NorivaMyrvaleSet()
                : EquipSet(11, false, false), sn(gsn_myrvale_noriva)
{
}

bool NorivaMyrvaleSet::hasAffect(Character *ch) const
{
    return ch->isAffected(sn);
}

void NorivaMyrvaleSet::addAffect(Character *ch) const
{
    if (ch->getProfession() != prof_cleric) {
        ch->pecho("Твое обмундирование на мгновение вспыхивает, но тут же гаснет.");
        return;
    }

    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;
    af.where     = TO_RESIST;
    af.bitvector = RES_WEAPON | RES_SPELL;
    af.modifier  = 3;
    af.location  = APPLY_LEVEL;
    affect_to_char(ch, &af);
    ch->pecho("{gПеред твоими глазами на мгновение возникает изображение ладони над пылающим кольцом.{x");
    ch->pecho("{gСила мир'вейл Норива пронизывает тебя.{x");
}

void NorivaMyrvaleSet::removeAffect(Character *ch) const
{
    ch->pecho("{gСила дома Норива ускользает от тебя.{x");
    affect_strip(ch, sn);
}

/*--------------------------------------------------------------------------
 * ReykarisShevaleSet 
 *-------------------------------------------------------------------------*/
ReykarisShevaleSet::ReykarisShevaleSet()
                : EquipSet(12, false, false), sn(gsn_shevale_reykaris)
{
}

bool ReykarisShevaleSet::hasAffect(Character *ch) const
{
    return ch->isAffected(sn);
}

void ReykarisShevaleSet::addAffect(Character *ch) const
{
    if (ch->getProfession() != prof_necromancer) {
        ch->pecho("Зловещая аура на мгновение окружает твое обмундирование.");
        return;
    }

    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;
    af.where     = TO_RESIST;
    af.bitvector = RES_WEAPON | RES_SPELL;
    af.modifier  = 3;
    af.location  = APPLY_LEVEL;
    affect_to_char(ch, &af);
    ch->pecho("{DПеред тобой возникает ухмыляющийся череп. В его пустых глазницах пылает жуткое {rкрасноватое{D пламя.{x");
    ch->pecho("{DСила ши'вейл Рейкарис пронизывает тебя.{x");
}

void ReykarisShevaleSet::removeAffect(Character *ch) const
{
    ch->pecho("{DСила дома Рейкарис ускользает от тебя.{x");
    affect_strip(ch, sn);
}

