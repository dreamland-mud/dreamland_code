/* $Id: hunter.cpp,v 1.1.6.11.6.22 2010-09-01 21:20:44 rufina Exp $
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

#include "hunter.h"
#include "cclantalk.h"

#include "commonattributes.h"
#include "char.h"
#include "util/regexp.h"

#include "commandtemplate.h"
#include "summoncreaturespell.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "skill_utils.h"
#include "fight.h"
#include "magic.h"
#include "save.h"
#include "material.h"
#include "weapongenerator.h"
#include "damage.h"
#include "merc.h"

#include "comm.h"
#include "mercdb.h"
#include "handler.h"
#include "interp.h"
#include "vnum.h"
#include "wiznet.h"
#include "act.h"
#include "act_move.h"
#include "roomtraverse.h"
#include "def.h"

#define OBJ_VNUM_HUNTER_PIT           110

bool obj_index_has_name( OBJ_INDEX_DATA *pObj, const DLString &arg );

GSN(hunter_pit);
GSN(hunter_beacon);
GSN(hunter_snare);
GSN(prevent);
GSN(detect_trap);
GSN(dispel_affects);
GSN(acid_arrow);
GSN(acid_blast);
GSN(caustic_font);
GSN(hunt);
GSN(poison);
GSN(shield_block);
GSN(shield_cleave);
GSN(spellbane);
GSN(weapon_cleave);
GSN(world_find);

CLAN(none);
CLAN(hunter);
CLAN(lion);
CLAN(flowers);

/*--------------------------------------------------------------------------
 * Hunter's Cleric 
 *-------------------------------------------------------------------------*/
void ClanHealerHunter::tell( Character *ach, const char *msg )
{
    speech(ach, msg);
}

void ClanHealerHunter::speech( Character *ach, const char *speech )
{
    PCharacter *wch;
    Character *carrier;
    Object *obj;
    int vnum = 0;
    ClanAreaHunter::Weapons::iterator i;
    ClanAreaHunter::Pointer clanArea;
    RegExp trouble("^(trouble|потеряла?) *(.*)$");
    
    if (!( wch = ach->getPC( ) ))
        return;
    if (!getClanArea( ))
        return;
    if (!( clanArea = getClanArea( ).getDynamicPointer<ClanAreaHunter>( ) ))
        return;

    RegExp::MatchVector matches = trouble.subexpr(speech);
    if (matches.empty())
        return;

    DLString itemName = matches.at(1);
    if (!( vnum = clanArea->vnumByString( itemName ) )) {
        do_say(ch, "Я не понимаю, что именно ты хочешь вернуть?");
        return;
    }
   
    if (wch->getClan( ) != clanArea->getClan( )) {
        do_say(ch, "Тебе придется сильно постараться!");
        return;
    }

    if (!wch->getAttributes( ).isAvailable( "hunterarmor" )) {
        do_say(ch, "Что ты имеешь в виду?");
        return;
    }
    
    obj = get_obj_world_unique( vnum, wch );
    
    if (!obj) {
        do_say( ch, "Ты уже не сможешь найти свое оружие!" );
    
        if (vnum == clanArea->armorVnum) 
            obj = clanArea->createArmor( wch );
        else
            obj = clanArea->createWeapon( wch, vnum );

        obj_to_char( obj, wch );
        
        ch->recho("%^C1 создает %O4.", ch, obj);        
        say_fmt("Я дам тебе друг%3$Gое|ой|ую %3$#O4.", ch, wch, obj);        
        oldact("$C1 дает $o4 $c3.", wch, obj, ch, TO_ROOM );
        oldact("$C1 дает тебе $o4.", wch, obj, ch, TO_CHAR );
        do_say( ch, "Будь внимательней! Не потеряй снова!" );
        return;
    }

    if (( carrier = obj->getCarrier( ) )) {
        if (carrier == wch) {
            do_say( ch, "Это шутка такая? Давай посмеемся вместе!" );
        }
        else {
            interpret_raw( ch, "say", "{1%s{2 находится у %s!",
                            obj->getShortDescr( '1' ).c_str( ),
                            wch->sees( carrier, '2' ).c_str( ) );
            interpret_raw( ch, "say", "%s находится в зоне %s около %s!",
                            wch->sees( carrier, '1' ).c_str( ),
                            carrier->in_room->areaName().c_str(),
                            carrier->in_room->getName() );
        }
    }
    else {
        interpret_raw( ch, "say", "{1%s{2 находится в зоне %s около %s!",
                        obj->getShortDescr( '1' ).c_str( ),
                        obj->getRoom( )->areaName().c_str(), 
                        obj->getRoom()->getName() );
    }
}

/*--------------------------------------------------------------------------
 * Hunter's Clan Guard 
 *-------------------------------------------------------------------------*/
void ClanGuardHunter::actPush( PCharacter *wch )
{
    oldact("$C1 вытягивает такой страшненький ножичек и слегка щекочет тебя.\n\r...Ты с диким воплем подпрыгиваешь и уносишься не видя ничего перед собой.", wch, 0, ch, TO_CHAR );
    oldact("$C1 вытягивает такой страшненький ножичек и слегка щекочет $c4\n\r... $c1 с диким воплем уносится не видя ничего перед собой.", wch, 0, ch, TO_ROOM );
}

void ClanGuardHunter::actGreet( PCharacter *wch )
{
    do_say( ch, "Добро пожаловать, доблестный охотник." );
    createEquipment( wch );
}

int ClanGuardHunter::getCast( Character *victim )
{
    int sn = -1;

    switch ( dice(1,16) )
    {
    case  0: 
    case  1:
            if (!victim->isAffected( gsn_spellbane ))
                sn = gsn_dispel_affects;
            break;
    case  2:
    case  3:
            sn = gsn_acid_arrow;
            break;
    case  4: 
    case  5:
            sn = gsn_caustic_font;
            break; 
    case  6:
    case  7:
    case  8:
    case  9:
            sn = gsn_acid_blast;
            break;
    default:
            sn = -1;
            break;
    }

    return sn;
}

void ClanGuardHunter::createEquipment( PCharacter *wch )
{
    Object *armor;
    ClanAreaHunter::Pointer clanArea;

    if (wch->getAttributes( ).isAvailable( "hunterarmor" ))
        return;
    if (!getClanArea( ))
        return;
    if (!( clanArea = getClanArea( ).getDynamicPointer<ClanAreaHunter>( ) ))
        return;
    
    wch->getAttributes( ).getAttr<XMLEmptyAttribute>( "hunterarmor" );
    armor = clanArea->createEquipment( wch ); 
    
    do_say( ch, "Я дарю тебе именное оружие охотника." );
    interpret( ch, "emote создает комплект оружия Охотников." );

    oldact("Ты передаешь $o4 $C3.", ch, armor, wch, TO_CHAR);
    oldact("$c1 передает тебе $o4.", ch, armor, wch, TO_VICT);
    oldact("$c1 передает $o4 $C3.", ch, armor, wch, TO_NOTVICT);
    obj_to_char( armor, wch );

    do_say( ch, "Помни! Если оружие будет утеряно, то найти его поможет клановый лекарь!" );
    do_say( ch, "Просто скажи ему '{lRпотерял{Sfа{Sx{lEtrouble{lx{g' и имя вещи. Например, '{lRпотерял{Sfа{Sx жакет{lEtrouble armor{x'." );
}

/*--------------------------------------------------------------------------
 * Hunter's Equipment (base) 
 *-------------------------------------------------------------------------*/
HunterEquip::HunterEquip( )
{
    clan.assign( clan_hunter );
}

void HunterEquip::config( PCharacter *wch )
{
    obj->fmtShortDescr( obj->getShortDescr( ), wch->getNameP('2').c_str() );
    obj->setOwner( wch->getNameC() );
    obj->from = str_dup( wch->getNameC() );
    obj->level = wch->getRealLevel( );
    obj->cost = 0;
    
    if (obj->pIndexData->extra_descr) {
        char buf[MAX_STRING_LENGTH];

        sprintf( buf, obj->pIndexData->extra_descr->description, wch->getNameP('1').c_str() );
        obj->addExtraDescr( obj->pIndexData->extra_descr->keyword, buf );
    }
}   

void HunterEquip::get( Character *ch )
{
    canEquip( ch );
}

bool HunterEquip::canEquip( Character *ch )
{
    if (ch->is_immortal( ))
        return true;
    
    if (obj->hasOwner( ch ) && ch->getClan( ) == clan)
    {
        ch->pecho( "{C%1$^O1 начина%1$nет|ют светиться.{x", obj );
        return true;
    }
    else {
        ch->pecho( "Ты не можешь владеть %1$O5 и бросаешь %1$P2.", obj );
        ch->recho("%2$^C1 не может владеть %1$O5 и бросает %1$P2.", obj, ch);
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }
}

bool HunterEquip::mayFloat( ) 
{
    return true;
}

/*---------------------------------------------------------------------------
 * Hunter's Armor 
 *-------------------------------------------------------------------------*/
void HunterArmor::wear( Character *ch )
{
    obj->level = ch->getRealLevel( );

    if (!obj->affected.empty()) {
        for (auto &paf: obj->affected)
            addAffect( ch, paf );
    }
    else {
        Affect af;

        af.type  = -1;
        af.duration = -1;

        af.location = APPLY_AC;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );

        af.location = APPLY_DAMROLL;
        addAffect( ch, &af );
        affect_to_obj( obj, &af );
    }
}

void HunterArmor::addAffect( Character *ch, Affect *paf ) 
{
    int level = ch->getModifyLevel( );

    switch (paf->location) {
    case APPLY_DAMROLL:
        paf->level = level;
        paf->modifier = level / 7;
        return;
    case APPLY_AC:
        paf->level = level;
        paf->modifier = -level;
        return;
    }
}

bool HunterArmor::canLock( Character *ch ) 
{ 
    return obj->hasOwner( ch );
}

void HunterArmor::delete_( Character *ch ) 
{
    if (obj->hasOwner( ch ))
        extract_obj( obj );
}

/*---------------------------------------------------------------------------
 * Hunter's Weapon 
 *-------------------------------------------------------------------------*/
void HunterWeapon::wear( Character *ch )
{
    obj->level = ch->getModifyLevel();
    WeaponGenerator()
        .item(obj)
        .valueTier(2)
        .hitrollTier(IS_GOOD(ch) ? 2 : 3)
        .damrollTier(IS_EVIL(ch) ? 2 : 3)
        .assignValues()
        .assignHitroll()
        .assignDamroll();
}

void HunterWeapon::fight( Character *ch )
{
    if (obj->wear_loc != wear_wield)
        return;
    
    if (number_percent( ) >= 25)
        return;

    switch (obj->value0()) {
    case WEAPON_SWORD:        fight_sword( ch );  return;
    case WEAPON_MACE:        fight_mace( ch );   return;
    case WEAPON_AXE:        fight_axe( ch );    return;
    }
}

/* shield cleave and may be destroy victim's equipment */
void HunterWeapon::fight_axe( Character *ch )
{
    Character *victim;
    int chance,ch_weapon,vict_shield;
    Object *shield;

    if ( ( victim = ch->fighting ) == 0 )
        return;
        
    chance=25;

    if ( ( shield = get_eq_char( victim, wear_shield )) == 0 )
        return;

    if (material_is_flagged( shield, MAT_INDESTR ) || shield->pIndexData->limit != -1)
        return;

    /* find weapon skills */
    ch_weapon = ch->getSkill(get_weapon_sn(ch, obj->wear_loc == wear_second_wield));
    vict_shield = std::max(1, gsn_shield_block->getEffective( ch ));
    /* modifiers */

    /* skill */
   chance = chance * ch_weapon / 200;
   chance = chance * 100 / vict_shield;

    /* dex vs. strength */
    chance += ch->getCurrStat(STAT_DEX);
    chance -= 2 * victim->getCurrStat(STAT_STR);

    /* level */
    chance += ch->getRealLevel( ) - skill_level(*gsn_shield_block, victim);
    chance += obj->level - shield->level;

    /* and now the attack */

    if (number_percent() < chance){
            //ch->setWait( gsn_shield_cleave->getBeats(ch)  );
        oldact_p("$o1 раскалывает пополам щит $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
        oldact_p("$o1 раскалывает пополам твой щит.",ch,obj,victim,TO_VICT,POS_DEAD);
        oldact_p("$o1 раскалывает пополам щит $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
        extract_obj( get_eq_char(victim,wear_shield) );
    }else if(::chance(10)){
        oldact_p("$o1 с грохотом отскакивает от щита $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
        oldact_p("$o1 с грохотом отскакивает от твоего щита.",ch,obj,victim,TO_VICT,POS_DEAD);
        oldact_p("$o1 с грохотом отскакивает от щита $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
            //ch->setWait( gsn_shield_cleave->getBeats(ch)  );
    }
}

/* stun */
void HunterWeapon::fight_mace( Character *ch )
{
    Character *victim;
    int chance;

    if ( ( victim = ch->fighting ) == 0 )
        return;
        
    chance=25;

    if (number_percent() < chance){
        oldact_p("$o1 оглушает $C4.",ch,obj,victim,TO_CHAR,POS_DEAD);
        oldact_p("$o1 оглушает тебя.",ch,obj,victim,TO_VICT,POS_DEAD);
        oldact_p("$o1 оглушает $C4.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
        SET_BIT(victim->affected_by,AFF_WEAK_STUN);
        victim->setWaitViolence( 2 );
    }
}

/* weapon destroy */
void HunterWeapon::fight_sword( Character *ch )
{
    Character *victim;
    Object *wield;
    int chance,ch_weapon,vict_weapon;

    if ( ( victim = ch->fighting ) == 0 )
        return;

    chance=25;

    if ( (wield = get_eq_char( victim, wear_wield )) == 0 )
        return;

    if (material_is_flagged( wield, MAT_INDESTR ) || wield->pIndexData->limit != -1 )
        return;

    /* find weapon skills */
    ch_weapon = ch->getSkill(get_weapon_sn(ch, obj->wear_loc == wear_second_wield));
    vict_weapon = std::max(1, victim->getSkill(get_weapon_sn(victim, false)));
    /* modifiers */

    /* skill */
    chance = chance * ch_weapon / 200;
    chance = chance * 100 / vict_weapon;

    /* dex vs. strength */
    chance += ch->getCurrStat(STAT_DEX) + ch->getCurrStat(STAT_STR);
    chance -= victim->getCurrStat(STAT_STR) +
                        2 * victim->getCurrStat(STAT_DEX);

    chance += ch->getRealLevel( ) - victim->getRealLevel( );
    chance += obj->level - wield->level;

    if (number_percent() < chance){
            //ch->setWait( gsn_weapon_cleave->getBeats(ch)  );
        oldact_p("$o1 уничтожает оружие $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
        oldact_p("$o1 уничтожает твое оружие.",ch,obj,victim,TO_VICT,POS_DEAD);
        oldact_p("$o1 уничтожает  оружие $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
        extract_obj( get_eq_char(victim,wear_wield) );
    }else if(::chance(10)){
        oldact_p("$o1 со звоном отскакивает от оружия $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
        oldact_p("$o1 со звоном отскакивает от твоего оружия.",ch,obj,victim,TO_VICT,POS_DEAD);
        oldact_p("$o1 со звоном отскакивает от оружия $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
            //ch->setWait( gsn_weapon_cleave->getBeats(ch)  );
    }
}

/*--------------------------------------------------------------------------
 * Hunter's Clan Area 
 *-------------------------------------------------------------------------*/
Object * ClanAreaHunter::createEquipment( PCharacter *wch )
{
    Object *armor;
    Weapons::iterator i;

    armor = createArmor( wch );

    for (i = weapons.begin( ); i != weapons.end( ); i++) 
        obj_to_obj( createWeapon( wch, i->second ), armor );

    return armor;
}

Object * ClanAreaHunter::createArmor( PCharacter *wch )
{
    Object *armor;

    armor = create_object( get_obj_index( armorVnum ), 0 );
    armor->behavior.getDynamicPointer<HunterArmor>( )->config( wch );
    return armor;
}

Object * ClanAreaHunter::createWeapon( PCharacter *wch, int vnum )
{
    Object *weapon;
    
    weapon = create_object( get_obj_index( vnum ), 0 );
    weapon->behavior.getDynamicPointer<HunterWeapon>( )->config( wch );
    return weapon;
}

int ClanAreaHunter::vnumByString( const DLString& msg )
{
    list<OBJ_INDEX_DATA *> items;
    items.push_back(get_obj_index(armorVnum));
    for (auto &w: weapons) 
        items.push_back(get_obj_index(w.second));

    for (auto *pObj: items) {
        if (pObj && obj_index_has_name(pObj, msg))
            return pObj->vnum;
    }

    return 0;
}


/*
 * 'hunt' command
 */
SKILL_RUNP( hunt )
{
    char arg[MAX_STRING_LENGTH];
    Character *victim;
    Road road;
    bool fArea;
    
    if (!gsn_hunt->available( ch )) {
        ch->pecho("Ты не умеешь охотиться.");
        return;
    }
    if (!gsn_hunt->usable( ch ))
        return;

    one_argument( argument, arg );

    if( arg[0] == '\0' ) {
        ch->pecho("Кого выслеживаем?");
        return;
    }

    fArea = !(ch->is_immortal());

    if (fArea && gsn_world_find->available( ch )) {
        if (number_percent() < gsn_world_find->getEffective( ch )) {
            fArea = false;
            gsn_world_find->improve( ch, true );
        }
        else {
            gsn_world_find->improve( ch, false );
            ch->pecho("Твоих знаний недостаточно, чтобы искать по всему миру!");
        }
    }

    victim = get_char_world( ch, arg, FFIND_SAME_AREA );

    if (!fArea && victim == 0)
        victim = get_char_world( ch, arg);

    if (victim == 0) {
        ch->pecho("Нет никого здесь с таким именем.");
        return;
    }

    if (victim->in_room == 0) {
        ch->pecho("Ты не можешь точно определить, где находится цель.");
        return;
    }

    if( ch->in_room == victim->in_room ) {
        oldact("$C1 прямо здесь!", ch, 0, victim, TO_CHAR);
        return;
    }

    /*
     * Deduct some movement.
     */
    if (!ch->is_immortal()) {
        if (ch->endur > 2)
            ch->endur -= 3;
        else {
            ch->pecho("Твои силы истощились и ты не можешь охотиться!");
            return;
        }
    }

    oldact("$c1 сосредоточенно осматривает местность и следы на земле.", ch, 0, 0, TO_ROOM );

    ch->setWait( gsn_hunt->getBeats(ch)  );
    
    road = room_first_step( 
                    ch,
                    ch->in_room, 
                    victim->in_room, 
                    true, false, false );
    
    if (road.type == Road::DOOR)
        oldact("$C1 $t отсюда.", ch, dirs[road.value.door].leave, victim, TO_CHAR );
    else
        oldact("Тебе не удается понять, как пройти к $C3.", ch, 0, victim, TO_CHAR );
}

SPELL_DECL(FindObject);
VOID_SPELL(FindObject)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found;
    int number = 0, max_found;
    DLString args = arg_unquote(target_name);
    
    found = false;
    number = 0;
    max_found = ch->is_immortal() ? 200 : 2 * level;

    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if ( !ch->can_see( obj ) 
                || !obj_has_name(obj, args, ch)
                || number_percent() > 2 * level
                || ch->getModifyLevel() < obj->level
                || IS_OBJ_STAT(obj, ITEM_NOFIND) )
            continue;

        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by))
        {
            sprintf( buf, "имеется у %s\n\r",
                ch->sees(in_obj->carried_by,'2').c_str() );
        }
        else
        {
            if (ch->is_immortal() && in_obj->in_room != 0)
                sprintf( buf, "находится в %s [Комната %d]\n\r",
                    in_obj->in_room->getName(), in_obj->in_room->vnum);
            else
                sprintf( buf, "находится в %s\n\r",
                    in_obj->in_room == 0
                        ? "somewhere" : in_obj->in_room->getName() );
        }

        buf[0] = Char::upper(buf[0]);
        buffer << buf;

        if (number >= max_found)
            break;
    }

    if ( !found )
        ch->pecho("В Dream Land нет ничего похожего на это.");
    else
        page_to_char( buffer.str( ).c_str( ), ch );
}




SPELL_DECL(TakeRevenge);
VOID_SPELL(TakeRevenge)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *obj;
    Room *room;

    if (!IS_DEATH_TIME( ch ))
    {
        ch->pecho("Слишком поздно мстить в твоем положении.");
        return;
    }
    
    obj = get_obj_world_unique( OBJ_VNUM_CORPSE_PC, ch );
    room = (obj ? obj->getRoom( ) : 0);

    if (room == 0)
        ch->pecho("Увы, похоже твой труп разделали на мясо.");
    else if ( IS_SET(room->affected_by,AFF_ROOM_PREVENT) )
        ch->pecho("Извини, но тебе не удается добраться туда.");
    else
        transfer_char( ch, ch, room );
}



/*--------------------------------------------------------------------------
 * trap skills
 *-------------------------------------------------------------------------*/
/*
 * base object for hunters trap
 */
HunterTrapObject::HunterTrapObject( ) 
            : activated( false )
{
}

bool HunterTrapObject::checkPrevent( Character *victim )
{
    if (victim->can_see( obj ))
        return true;

    if (!victim->is_npc( ) && victim->getClan( ) == clan_flowers)
        return true;

    if (!victim->isAffected( gsn_prevent ))
        return false;

    if (!saves_spell( ownerLevel, victim, DAM_NONE ))
        return false;

    oldact("Сила твоего клана защищает тебя от ловушек Охотников.", victim, 0, 0, TO_CHAR);
    oldact("Сила клана защищает $c4 от ловушек Охотников.", victim, 0, 0, TO_ROOM);
    return true;
}

bool HunterTrapObject::checkRoom( Room *r )
{
    if (r->pIndexData->clan != clan_none && r->pIndexData->clan != clan_hunter)
        return false;
    
    if (IS_SET(r->room_flags, ROOM_SAFE | ROOM_LAW | ROOM_NO_DAMAGE))
        return false;
    
    return true;
}

bool HunterTrapObject::checkTrapConditions( Character *ch, Skill &skill )
{
    if (obj->carried_by != ch) {
        ch->pecho( "Подними это с земли." );
        return false;
    }
    
    if (!skill.usable( ch ))
        return false;

    if (skill.getLearned( ch ) <= 1) {
        ch->pecho( "Попрактикуйся сначала." );
        return false;
    }
    
    if (ch->mana < skill.getMana(ch)) {
        ch->pecho( "У тебя недостаточно энергии для этого." );
        return false;
    }

    if (ch->position != POS_STANDING) {
        ch->pecho( "Это гораздо удобней делать стоя." );
        return false;
    }
    
    if (IS_SET(ch->in_room->affected_by, AFF_ROOM_PREVENT)) {
        ch->pecho( "Львы защитили эту местность от ловушек Охотников." );
        return false;
    }

    return true;
} 

bool HunterTrapObject::visible( const Character *ch ) 
{
    if (ch->is_immortal( ))
        return true;

    if (!activated)
        return true;

    if (ch->is_npc( ))
        return false;
    
    if (ch->getName( ) == ownerName.getValue( ))
        return true;

    if (ch->isAffected(gsn_detect_trap))
        return true;

    return false;
}

void HunterTrapObject::log( Character *ch, const char *verb )
{
    wiznet( WIZ_FLAGS, 0, 110, 
            "Охотничьи ловушки: %^C1 %s %O4 в [%d] '%s'",
            ch, verb, obj, ch->in_room->vnum, ch->in_room->getName() );
}

/*
 * beacon trap
 */
bool HunterBeaconTrap::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterBeaconTrap::use( Character *ch, const char *cArgs ) 
{
    PCharacter *victim;
    DLString args = cArgs;
    
    if (!gsn_hunter_beacon->available( ch ))
        return false;

    if (!checkTrapConditions( ch, *gsn_hunter_beacon ))
        return true;
    
    if (!checkRoom( ch->in_room )) {
        ch->pecho( "Здесь нельзя устанавливать маяки." );        
        return true;
    }
    
    if (ch->isAffected( gsn_hunter_beacon )) {
        ch->pecho( "С момента установки предыдущего маяка прошло слишком мало времени." );
        return true;
    }
    
    args.colourstrip( );
    args.stripWhiteSpace( );
    if (args.empty( )) {
        ch->pecho( "На кого именно должен реагировать маяк?" );
        return true;
    }

    victim = get_player_world( ch, args.c_str( ) );
    if (victim == NULL) {
        ch->pecho( "Жертва с таким именем не найдена." );
        return true;
    }

    if (is_safe_nomessage( ch, victim )) {
        ch->pecho( "Жертва не находится в твоем ПК." );
        return true;
    }
    
    if (!chance( gsn_hunter_beacon->getEffective( ch ) )) {
        oldact("Твоя попытка установить $o4 окончилась неудачей.", ch, obj, 0, TO_CHAR );
        ch->mana -= gsn_hunter_beacon->getMana(ch) / 2;
        ch->setWait( gsn_hunter_beacon->getBeats(ch) / 2 );
        gsn_hunter_beacon->improve( ch, false );

        if (!chance( ch->getPC( )->getClanLevel( ) * 10 )) {
            oldact("Из-за неумелого обращения ты уничтожаешь $o4.", ch, obj, 0, TO_CHAR );
            oldact("$c1 своим неумелым обращением уничтожает $o4.", ch, obj, 0, TO_ROOM );
            extract_obj( obj );
        }
        
        return true;
    } 
    
    oldact("Ты устанавливаешь $o4 и настраиваешь реакцию на появление $C2.", ch, obj, victim, TO_CHAR );
    oldact("$c1 устанавливает и настраивает $o4.", ch, obj, 0, TO_ROOM );
    
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    REMOVE_BIT( obj->wear_flags, ITEM_TAKE );
    obj->timer = ch->getPC( )->getClanLevel( ) * 5;
    obj->setDescription( activeDescription.getValue( ).c_str( ) );
    
    activated = true;
    victimName = victim->getName( );
    quality = gsn_hunter_beacon->getEffective( ch );
    ownerName = ch->getName( );
    ownerLevel = ch->getModifyLevel( );
    charges = number_range( 1, ch->getPC( )->getClanLevel( ) );
    
    postaffect_to_char( ch, gsn_hunter_beacon, number_range( 0, 1 ) );
    ch->setWait( gsn_hunter_beacon->getBeats(ch) );
    ch->mana -= gsn_hunter_beacon->getMana(ch);
    gsn_hunter_beacon->improve( ch, true );
    
    log( ch, "устанавливает" );
    return true; 
}

void HunterBeaconTrap::greet( Character *victim )
{
    if (!activated || !obj->in_room)
        return;
    
    if (victim->is_npc( ) || victim->is_immortal( ))
        return;
    
    if (victimName.getValue( ) != victim->getName( ))
        return;
        
    if (checkPrevent( victim ))
        return;

    if (!chance( quality + 10 ))
        return;

//    oldact("Рядом с тобой раздается щелчок.", victim, 0, 0, TO_ALL );

    clantalk( *clan_hunter, 
              "Внимание! Сработал маяк, установленный в '%s' и настроенный на появление %s.",
              obj->in_room->getName(), victim->getNameP( '2' ).c_str( ) );
    
    log( victim, "активизирует" );

    if (( charges = charges - 1 ) <= 0)
        extract_obj( obj );
}

    
/*
 * damage from getting captured into the snare
 */
struct HunterSnareDamage : public Damage {
    HunterSnareDamage( Character *ch, HunterSnareTrap::Pointer snare, bool fMovement ) 
                      : Damage( ch, ch, DAM_SLASH, 0, DAMF_WEAPON ) 
    {
        this->snare = snare;
        this->fMovement = fMovement;
    }
    
    virtual ~HunterSnareDamage( ) {
    }
        
    virtual void message( ) {
        if (fMovement) {
            msgChar( "%2$^O1\6твою ногу при ходьбе", dam, snare->getObj( ) );
            msgRoom( "%3$^C1 морщится от боли, наступив на зажатую в %2$O4 ногу", dam, snare->getObj( ), ch );
        }
        else {
            msgRoom( "%2$^O1\6%3$C4", dam, snare->getObj( ), ch );
            msgChar( "%2$^O1\6тебя", dam, snare->getObj( ) );
        }
    }

    virtual void calcDamage( ) {
        int level = snare->getObj( )->level;

        if (fMovement) 
            dam = number_range( level / 2, level * 2 );        
        else 
            dam = number_range( level * 5, level * 8 );

        if (ch->getClan( ) == clan_lion)
            dam += dam / 5;

        dam = dam * snare->getQuality( ) / 100;

        protectSanctuary( );
        protectImmune( );
        protectRazer( );
        protectMaterial( snare->getObj( ) );
    }

protected:
    HunterSnareTrap::Pointer snare;
    bool fMovement;
};

    

/*
 * snare trap
 */
bool HunterSnareTrap::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterSnareTrap::use( Character *ch, const char *cArgs ) 
{
    if (!gsn_hunter_snare->available( ch ))
        return false;

    if (!checkTrapConditions( ch, *gsn_hunter_snare ))
        return true;

    if (!checkRoom( ch->in_room )) {
        ch->pecho( "Здесь невозможно установить и замаскировать капкан." );        
        return true;
    }
    
    if (ch->isAffected( gsn_hunter_snare )) {
        ch->pecho( "Предыдущий капкан был установлен тобой совсем недавно." );
        return true;
    }

    if (obj->level > ch->getModifyLevel( )) {
        ch->pecho( "Устройство этого капкана слишком сложно для твоего понимания." );
        return true;
    }

    if (!ownerName.getValue( ).empty( )) {
        ch->pecho( "В этом капкане уже кто-то побывал." );
        return true;
    }
    
    if (!chance( gsn_hunter_snare->getEffective( ch ))) {
        if (!chance( ch->getPC( )->getClanLevel( ) * 10 )) {
            ch->pecho( "Ты пытаешься зарядить %1$O4, но зажимаешь в %1$P4 собственную руку. Это больно!", obj );
            ch->recho( "%2$^C1 пытается зарядить %1$O4, но зажимает в %1$P4 собственную руку.", obj, ch );
            rawdamage( ch, ch, DAM_PIERCE, ch->hit / 10, true, "room" );
        }
        else {
            ch->pecho( "Ты пытаешься установить %1$O4, но только ломаешь %1$P2.", obj );
            ch->recho( "%2$^C1 пытается установить %1$O4, но только ломает %1$P2.", obj, ch );
        }

        ch->setWait( gsn_hunter_snare->getBeats(ch) / 2 );
        gsn_hunter_snare->improve( ch, false );
        extract_obj( obj );
        return true;
    }
    
    oldact("Ты устанавливаешь и маскируешь $o4.", ch, obj, 0, TO_CHAR );
    oldact("$c1 устанавливает и маскирует $o4.", ch, obj, 0, TO_ROOM );

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    obj->timer = 24 * 60;
    REMOVE_BIT( obj->wear_flags, ITEM_TAKE );
    obj->setDescription( activeDescription.getValue( ).c_str( ) );
    
    activated = true;
    ownerName = ch->getName( );
    ownerLevel = ch->getModifyLevel( );
    quality = gsn_hunter_snare->getEffective( ch );

    postaffect_to_char( ch, gsn_hunter_snare, number_range( 1, 3 ) );
    ch->setWait( gsn_hunter_snare->getBeats(ch) );
    ch->mana -= gsn_hunter_snare->getMana(ch);
    gsn_hunter_snare->improve( ch, true );

    log( ch, "маскирует" );
    return true;
}

void HunterSnareTrap::greet( Character *victim )
{
    if (!activated || !obj->in_room)
        return;

    if (is_safe_rspell( ownerLevel, victim, false ))
        return;

    if (get_eq_char( victim, wear_hold_leg ))
        return;

    if (is_flying( victim ))
        return;

    if (!chance( quality ))
        return;

    if (checkPrevent( victim ))
        return;

    obj_from_room( obj );
    obj_to_char( obj, victim );
    equip_char( victim, obj, wear_hold_leg );
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    obj->fmtDescription( "Разломанный %s лежит тут.", obj->getShortDescr( '1' ).c_str( ) );
    obj->timer = 24;
    activated = false;
    
    oldact("Твоя нога попала в $o4!", victim, obj, 0, TO_CHAR );
    oldact("$c1 угоди$gло|л|ла в $o4!", victim, obj, 0, TO_ROOM );

    try {
        HunterSnareDamage( victim, this, false ).hit( true );
        victim->setWait( gsn_hunter_snare->getBeats(victim) );
    } catch (const VictimDeathException &) {
    }

    log( victim, "попадает в" );
}


bool HunterSnareTrap::checkRoom( Room *r )
{
    if (!HunterTrapObject::checkRoom( r ))
        return false;

    switch (r->getSectorType()) {
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_FIELD:
    case SECT_MOUNTAIN:
        return true;
    default:
        return false;
    }
}

void HunterSnareTrap::fight( Character *ch )
{
    if (obj->wear_loc != wear_hold_leg)
        return;

    ch->move -= move_dec( ch );
}

void HunterSnareTrap::entry( )
{
    Character *ch = obj->carried_by;
    
    if (obj->wear_loc != wear_hold_leg)
        return;

    if (is_flying( ch ))
        return;
    
    try {
        HunterSnareDamage( ch, this, true ).hit( true );
    } catch (const VictimDeathException &) {
    }
}

int HunterSnareTrap::getQuality( ) const
{
    return quality.getValue( );
}

/*
 * shovel for pit trap
 */
bool HunterShovel::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterShovel::use( Character *ch, const char *cArgs ) 
{
    Object *pit;
    HunterPitTrap::Pointer bhv;
    int moveCost, chance;
    
    if (!gsn_hunter_pit->available( ch ))
        return false;
    
    if (obj->wear_loc == wear_none) {
        oldact("Ты не держишь $o4 в руках.", ch, obj, 0, TO_CHAR );
        return true;
    }
    
    if (!checkTrapConditions( ch, *gsn_hunter_pit ))
        return true;

    if (!checkRoom( ch->in_room )) {
        ch->pecho( "Здешняя почва непригодна для копания ямы." );        
        return true;
    }
    
    moveCost = ch->max_move / 4;

    if (ch->move < moveCost) {
        oldact("Ты слишком уста$gло|л|ла.", ch, 0, 0, TO_CHAR );
        return true;
    }
    
    if (obj->condition < 10) {
        ch->pecho( "%1$^O1 слишком затупил%1$Gось|ся|ась|ись.", obj );
        return true;
    }
    
    pit = get_obj_room_vnum( ch->in_room, OBJ_VNUM_HUNTER_PIT );

    if (!pit) {
        pit = create_object( get_obj_index( OBJ_VNUM_HUNTER_PIT ), 0 );
        obj_to_room( pit, ch->in_room );
    }
    
    if (!pit->behavior || !(bhv = pit->behavior.getDynamicPointer<HunterPitTrap>( ))) {
        ch->pecho( "Что-то не так.." );
        return true;
    }
    
    if (!bhv->isFresh( ) && !bhv->isOwner( ch )) {
        ch->pecho( "Другой Охотник уже начал копать здесь яму, не стоит ему мешать." );
        return true;
    }
    
    if (bhv->getSteaks( )) {
        ch->pecho( "Эта яма уже замаскирована и ждет гостей." );
        return true;
    }

    bhv->setOwner( ch );
    chance = gsn_hunter_pit->getEffective( ch );

    if (bhv->getDepth( ) == 0) {
        oldact("Ты начинаешь копать $o4.", ch, pit, 0, TO_CHAR );
        oldact("$c1 начинает копать $o4.", ch, pit, 0, TO_ROOM );
        bhv->setDepth( 1 );
    }
    else {
        if (number_percent( ) < number_fuzzy( chance )) {
            oldact("Ты орудуешь $O5, еще больше углубляя $o4.", ch, pit, obj, TO_CHAR );
            oldact("$c1 орудует $O5, углубляя $o4.", ch, pit, obj, TO_ROOM );
            bhv->setDepth( bhv->getDepth( ) + 1 );
            gsn_hunter_pit->improve( ch, true );
        }
        else {
            oldact("Ты втыкаешь $o4 в почву, но натыкаешься на камень.", ch, obj, 0, TO_CHAR );
            oldact("$c1 втыкает $o4 в почву, но натыкается на камень.", ch, obj, 0, TO_ROOM );
            gsn_hunter_pit->improve( ch, false );
        }
    }
    
    bhv->setDescription( );

    if (number_percent( ) < 10) {
        ch->pecho( "%1$^O1 слегка туп%1$nится|ятся.", obj );
        obj->condition = max( 1, obj->condition - 10 );
    }
    
    ch->setWait( gsn_hunter_pit->getBeats(ch) );
    ch->move -= moveCost;
    ch->mana -= gsn_hunter_pit->getMana(ch);
    
    save_items( ch->in_room );
    return true;
}

bool HunterShovel::checkRoom( Room *r )
{
    if (!HunterTrapObject::checkRoom( r ))
        return false;

    switch (r->getSectorType()) {
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_FIELD:
        return true;
    default:
        return false;
    }
}
   
/*
 * steaks for pit trap
 */
bool HunterPitSteaks::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterPitSteaks::use( Character *ch, const char * cArgs )
{
    DLString args = cArgs;
    HunterPitTrap::Pointer bhv;
    Object *pit;

    if (!gsn_hunter_pit->available( ch ))
        return false;

    if (!checkTrapConditions( ch, *gsn_hunter_pit ))
        return true;

    if (obj->level > ch->getModifyLevel( )) {
        oldact("Ты недостаточно опыт$gно|ен|на, чтобы использовать $o4.", ch, obj, 0, TO_CHAR );
        return true;
    }

    pit = get_obj_room_vnum( ch->in_room, OBJ_VNUM_HUNTER_PIT );
    if (!pit) {
        oldact("Здесь некуда засунуть $o4.", ch, obj, 0, TO_CHAR );
        oldact("$c1 тычет повсюду $o5, ища, куда бы это засунуть.", ch, obj, 0, TO_ROOM );
        return true;
    }

    if (!pit->behavior || !(bhv = pit->behavior.getDynamicPointer<HunterPitTrap>( ))) {
        ch->pecho( "С этой ямой что-то не так.." );
        return true;
    }
    
    if (bhv->getSteaks( )) {
        ch->pecho( "Эта яма уже замаскирована и ждет гостей." );
        return true;
    }

    if (!bhv->isOwner( ch )) {
        ch->pecho( "Эту яму выкопал другой Охотник." );
        return true;
    }
    
    oldact("Ты устанавливаешь на дне $O2 $o4 и тщательно маскируешь яму.", ch, obj, pit, TO_CHAR); 
    oldact("$c1 устанавливает на дне $O2 $o4 и тщательно маскирует яму.", ch, obj, pit, TO_ROOM); 
    bhv->setReady( ch );
    obj_from_char( obj );
    obj_to_obj( obj, pit );
    ch->setWait( gsn_hunter_pit->getBeats(ch) );

    log( ch, "устанавливает" );
    return true;
}


/*
 * damage from falling into the pit
 */
struct HunterPitDamage : public Damage {
    HunterPitDamage( Character *ch, HunterPitTrap::Pointer pit ) 
                     : Damage( ch, ch, DAM_BASH, 0, DAMF_WEAPON ) 
    {
        this->pit = pit;
    }

    virtual ~HunterPitDamage( ) {
    }
    
    virtual void message( ) {
        msgRoom( "%2$^O1 в %3$O6\6 %4$C4", dam, pit->getSteaks( ), pit->getObj( ), ch );
        msgChar( "%2$^O1 в %3$O6\6 тебя", dam, pit->getSteaks( ), pit->getObj( ) );
    }
    
    virtual void calcDamage( ) {
        dam = pit->getSteaks( )->level * number_range( 30, 40 );

        if (ch->getClan( ) == clan_lion)
            dam += dam / 5;

        dam += dam * 10 * max(0, pit->getSize( ) - victim->size) / 100;
        dam = dam * pit->getQuality( ) / 100;

        protectSanctuary( );
        protectImmune( );
        protectRazer( );
        protectMaterial( pit->getSteaks( ) );
    }

    virtual void postDamageEffects( ) {
        Object *obj = pit->getSteaks( );

        if (obj->item_type != ITEM_WEAPON)
            return;

        if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
            if (!saves_spell( obj->level, ch, DAM_POISON )) {   
                Affect af;

                oldact("Ты чувствуешь, как яд распространяется по твоим венам.", ch, 0, 0, TO_CHAR);
                oldact("$c1 отравле$gно|н|на ядом от $o2.", ch, obj, 0, TO_ROOM);

                af.bitvector.setTable(&affect_flags);
                af.type      = gsn_poison;
                af.level     = obj->level;
                af.duration  = obj->level / 4;
                af.location = APPLY_STR;
                af.modifier  = max( 1, obj->level / 20 );
                af.bitvector.setValue(AFF_POISON);
                affect_join( ch, &af );
            }
        }
    }

protected:
    HunterPitTrap::Pointer pit;
};


/*
 * pit trap 
 */
void HunterPitTrap::greet( Character *victim ) 
{
    if (!activated || !obj->in_room || !getSteaks( ))
        return;

    if (is_safe_rspell( ownerLevel, victim, false )) 
        return;

    if (is_flying( victim )) 
        return;

    if (victim->size > getSize( )) 
        return;
    
    if (!chance( number_fuzzy( quality ) )) 
        return;
    
    if (checkPrevent( victim )) 
        return;

    activated = false;
    oldact("Ты проваливаешься в $o4 и падаешь прямо на $O4!", victim, obj, getSteaks( ), TO_CHAR);
    oldact("$c1 проваливается в $o4 и падает прямо на $O4!", victim, obj, getSteaks( ), TO_ROOM);
    
    try { 
        HunterPitDamage( victim, this ).hit( true );

        oldact("Ты теряешь сознание.", victim, 0, 0, TO_CHAR);
        oldact("$c1 теряет сознание.", victim, 0, 0, TO_ROOM);
        victim->position = POS_STUNNED;
        victim->setWait( gsn_hunter_pit->getBeats(victim) );
    } catch (const VictimDeathException &) {
    }

    log( victim, "падает в" );
    
    extract_obj( getSteaks( ) );
    unsetReady( );
}

bool HunterPitTrap::area( )
{
    if (getSteaks( ))
        return false;
    
    if (chance( 90 ))
        return false;

    if (getDepth( ) > 0)
        setDepth( getDepth( ) - 1 );

    if (getDepth( ) == 0 && chance( 10 )) {
        extract_obj( obj );
        return true;
    }

    return false;
}

void HunterPitTrap::setDepth( int depth )
{
    this->depth = depth;
}

int HunterPitTrap::getDepth( ) const
{
    return depth.getValue( );
}

int HunterPitTrap::getSize( ) const
{
    return getDepth( ) / 3;
}

void HunterPitTrap::unsetReady( )
{
    activated = false;
    setDescription( );
    obj->timer = 0;
}

void HunterPitTrap::setReady( Character *ch )
{
    activated = true;
    ownerName = ch->getName( );
    ownerLevel = ch->getModifyLevel( );
    quality = gsn_hunter_pit->getEffective( ch );
    obj->setDescription( activeDescription.getValue( ).c_str( ) );
    obj->timer = 60 * 24;
}

Object * HunterPitTrap::getSteaks( ) 
{
    return obj->contains;
}

bool HunterPitTrap::isOwner( Character *ch ) const
{
    return !ch->is_npc( ) && ownerName.getValue( ) == ch->getName( );
}

void HunterPitTrap::setOwner( Character *ch )
{
    ownerName = ch->getName( );
}

int HunterPitTrap::getQuality( ) const
{
    return quality.getValue( );
}

bool HunterPitTrap::isFresh( ) const
{
    return ownerName.getValue( ).empty( );
}

void HunterPitTrap::setDescription( )
{
    obj->fmtDescription( 
            "В земле вырыта яма %s размера.", 
            size_table.message(URANGE( SIZE_TINY, getSize( ), SIZE_GARGANTUAN ), '2' ).c_str( ) );
}

/*
 * 'detect trap' spell
 */
SPELL_DECL(DetectTrap);
VOID_SPELL(DetectTrap)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Affect af;

    if (ch->isAffected(sn)) {
        ch->pecho( "Ты и так в состоянии отличить бревно от капкана.");
        return;
    }

    af.type             = sn;
    af.level            = level;
    af.duration         = max( 6, ch->getPC( )->getClanLevel( ) * 2 );
    affect_to_char(ch,&af);

    oldact("Теперь ты будешь замечать чужие ловушки.", ch, 0, 0, TO_CHAR);
    oldact("Взгляд $c2 становится более внимательным.", ch, 0, 0, TO_ROOM);
}

