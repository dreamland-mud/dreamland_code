#include "equipsets.h"
#include <map>

#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "character.h"
#include "object.h"
#include "skill.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "magic.h"
#include "interp.h"
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
GSN(morris_dancer);
PROF(cleric);
PROF(necromancer);

/*--------------------------------------------------------------------------
 * EquipSet
 *-------------------------------------------------------------------------*/
EquipSet::EquipSet(int sn_, int size, bool noDoubleNeck_, bool noDoubleWrist_) 
                      : sn(sn_), totalSetSize(size), noDoubleNeck(noDoubleNeck_), noDoubleWrist(noDoubleWrist_)
{
}
 
void EquipSet::equip( Character *victim )
{

    if (hasAffect(victim)) 
        return;

    if (!isComplete(victim, true)) 
        return;

    addAffect(victim);    
}

void EquipSet::remove( Character *victim )
{

    if (!hasAffect(victim))
        return;

    if (isComplete(victim, false))
        return;

    removeAffect(victim);
}

bool EquipSet::isComplete(Character *ch, bool verbose) const
{
    GlobalArray slots(wearlocationManager);
    int wornSetSize = 0;
    
    if (totalSetSize <= 0)
        return false;

    for (Object *item = ch->carrying; item; item = item->next_content) {
        if (item->wear_loc == wear_none)
            continue;
        if (!item->behavior)
            continue;
        if (item->behavior->getType() != obj->behavior->getType())
            continue;
        if (item->getRealShortDescr())
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
    
    bool complete = wornSetSize >= totalSetSize;

    if (verbose && !complete)
        ch->pecho("Ты надел%Gо||а уже {c%d{x из {C%d{x предметов {hh67комплекта{x '{C%N1{x'.",
                   ch, wornSetSize, totalSetSize, skillManager->find(sn)->getRussianName().c_str());

    return complete;
}

/*--------------------------------------------------------------------------
 * SidheArmorSet
 *-------------------------------------------------------------------------*/
SidheArmorSet::SidheArmorSet()
                : EquipSet(gsn_sidhe_armor, 5, true, true)
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
        ch->pecho("Лишь те, кто следуют по пути Добра или Зла, смогут завершить комплект.");       
        return;
    }

    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;

    af.bitvector.setTable(&affect_flags);
    af.bitvector.setValue(IS_EVIL(ch) ? AFF_PROTECT_GOOD : AFF_PROTECT_EVIL);

    af.modifier  =  10;
    af.location = APPLY_HITROLL;
    affect_to_char(ch, &af);

    af.bitvector.setTable(&res_flags);
    af.bitvector.setValue(ch->getRace()->getVuln().isSet(VULN_IRON) ? RES_IRON : 0);
    af.modifier  =  20;
    af.location = APPLY_DAMROLL;
    affect_to_char(ch, &af);

    af.bitvector.setTable(&vuln_flags);
    af.bitvector.setValue(IS_EVIL(ch) ? VULN_HOLY : VULN_NEGATIVE);
    af.modifier  =  150;
    af.location = APPLY_HIT;
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
                : EquipSet(gsn_travellers_joy, 4, true, true)
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

    af.modifier  =  8;
    af.location = APPLY_HITROLL;
    affect_to_char(ch, &af);

    af.modifier  =  12;
    af.location = APPLY_DAMROLL;
    affect_to_char(ch, &af);

    af.modifier  =  35;
    af.location = APPLY_HIT;
    affect_to_char(ch, &af);

    af.modifier  =  2;
    af.location = APPLY_DEX;
    affect_to_char(ch, &af);

    af.modifier  =  2;
    af.location = APPLY_STR;
    affect_to_char(ch, &af);

    af.modifier  =  2;
    af.location = APPLY_CON;
    affect_to_char(ch, &af);

    ch->pecho("{CКомплект одежд путешественника начинает светиться ровным голубоватым светом.{x");
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
                : EquipSet(gsn_myrvale_noriva, 12, false, false)
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
        ch->pecho("Этот комплект предназначен только для клериков.");       
        return;
    }

    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;
    af.bitvector.setTable(&res_flags);
    af.bitvector.setValue(RES_WEAPON | RES_SPELL);
    af.modifier  = 3;
    af.location = APPLY_LEVEL;
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
                : EquipSet(gsn_shevale_reykaris, 9, false, false)
{
}

bool ReykarisShevaleSet::hasAffect(Character *ch) const
{
    return ch->isAffected(sn);
}

void ReykarisShevaleSet::addAffect(Character *ch) const
{
    if (ch->getProfession() != prof_necromancer) {
        ch->pecho("Зловещая аура на мгновение окружает твое обмундирование, но тут же гаснет.");
        ch->pecho("Этот комплект предназначен только для некромантов.");       
        return;
    }

    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;
    af.bitvector.setTable(&res_flags);
    af.bitvector.setValue(RES_WEAPON | RES_SPELL);
    af.modifier  = 3;
    af.location = APPLY_LEVEL;
    affect_to_char(ch, &af);
    ch->pecho("{DПеред тобой возникает ухмыляющийся череп. В его пустых глазницах пылает жуткое {rкрасноватое{D пламя.{x");
    ch->pecho("{DСила ши'вейл Рейкарис пронизывает тебя.{x");
}

void ReykarisShevaleSet::removeAffect(Character *ch) const
{
    ch->pecho("{DСила дома Рейкарис ускользает от тебя.{x");
    affect_strip(ch, sn);
}

/*--------------------------------------------------------------------------
 * MorrisDancerSet 
 *-------------------------------------------------------------------------*/
MorrisDancerSet::MorrisDancerSet()
                : EquipSet(gsn_morris_dancer, 4, true, true)
{
}

bool MorrisDancerSet::hasAffect(Character *ch) const
{
    return ch->isAffected(sn);
}

void MorrisDancerSet::addAffect(Character *ch) const
{
    Affect af;

    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = -2;

    af.modifier  =  5;
    af.location = APPLY_HITROLL;
    affect_to_char(ch, &af);

    af.modifier  =  7;
    af.location = APPLY_DAMROLL;
    affect_to_char(ch, &af);

    af.modifier  =  50;
    af.location = APPLY_HIT;
    affect_to_char(ch, &af);

    af.modifier  =  50;
    af.location = APPLY_MANA;
    affect_to_char(ch, &af);

    af.modifier  =  5;
    af.location = APPLY_CHA;
    affect_to_char(ch, &af);

    ch->pecho("{WТвои ноги неудержимо пускаются в пляс!{x");
}

void MorrisDancerSet::removeAffect(Character *ch) const
{
    ch->pecho("Желание плясать покидает тебя.");
    affect_strip(ch, sn);
}

bool MorrisDancerSet::area( )
{
    Character *ch = obj->carried_by;
    if (!ch)
        return false;
    if (!ch->isAffected(sn))
        return false;
    if (ch->position != POS_STANDING)
        return false;
    if (chance(99))
        return false;
    
    // Dance with a 'random' person in the room.
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch != ch && chance(50)) {
            ch->pecho("{WЖелание непременно с кем-то потанцевать охватывает тебя!{x");
            switch(number_range(0, 1)) {
            case 0:
                interpret_cmd(ch, "dance", rch->getName().c_str());
                break;
            case 1:
                interpret_cmd(ch, "discodance", rch->getName().c_str());
                break;
            }
            break;
        }

    return false;
}

void MorrisDancerSet::fight( Character *ch )
{
    if (!ch->isAffected(sn))
        return;

    if (number_range(0, 3) != 0)
        return;

    if (!IS_AFFECTED(ch, AFF_HASTE) && chance(50)) {
        spell(gsn_haste, ch->getModifyLevel(), ch, ch );
        ch->println("{WТы внезапно ощущаешь повышенную активность!{x");
        return;
    }

    switch(number_range(0, 200)) {
    case 0:
        do_yell(ch, "Mamma mia, mamma mia, gia' la luna e' in mezzo al mare!");
        break;
    case 1:
        do_yell(ch, "Нет времени на медленные танцы!");
        break;
    case 2:
        do_yell(ch, "Dance me to the end of love!");
        break;
    case 3:
        do_yell(ch, "Time on the ground is time wasted!");
        break;
    case 4:
        do_yell(ch, "С птицей и полотенцем потанцуй у стекла!");
        break;                  
    }
}
