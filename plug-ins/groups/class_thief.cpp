/* $Id: class_thief.cpp,v 1.1.2.27.6.25 2010-09-01 21:20:44 rufina Exp $
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

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "grammar_entities_impl.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "affecthandlertemplate.h"

#include "objectbehavior.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "bonus.h"
#include "dreamland.h"


#include "drink_utils.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "arg_utils.h"
#include "chance.h"
#include "exitsmovement.h"
#include "keyhole.h"
#include "string_utils.h"
#include "save.h"

#include "magic.h"
#include "fight.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "occupations.h"
#include "skill_utils.h"
#include "player_utils.h"
#include "loadsave.h"
#include "act.h"
#include "interp.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

GSN(backguard);
GSN(backstab);
GSN(blackjack);
GSN(circle);
GSN(dual_backstab);
GSN(envenom);
GSN(key_forgery);
GSN(pick_lock);
GSN(poison);
GSN(push);
GSN(rear_kick);
GSN(steal);

BONUS(thief_skills);

/*----------------------------------------------------------------------------
 * Circle
 *---------------------------------------------------------------------------*/
class CircleOneHit: public SkillWeaponOneHit {
public:
    CircleOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

CircleOneHit::CircleOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_circle )
{
}

void CircleOneHit::calcDamage( )
{
    damBase( );
    damapply_class(ch, dam);
    damApplyDamroll( );

    int slevel = skill_level(*gsn_circle, ch);    
    dam = ( slevel / 40 + 1 ) * dam + slevel;

    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}

/*
 * 'pick lock' skill command
 */

SKILL_RUNP( pick )
{
    DLString args = argument, arg1, arg2;
    Keyhole::Pointer keyhole;


    if (MOUNTED(ch)) {
        ch->pecho( _("Ты не можешь взломать что-либо, пока ты в седле.") );
        return;
    }

    arg1 = args.getOneArgument( );
    arg2 = args.getOneArgument( );

    if (arg1.empty( )) {
        ch->pecho( _("Взломать что?") );
        return;
    }

    if (arg2.empty( )) {
        ch->pecho( _("Взломать чем?") );
        return;
    }

    if (!( keyhole = Keyhole::create( ch, arg1 ) )) {
        ch->pecho( _("Ты не видишь здесь такого замка.") );
        return;
    }

    keyhole->doPick( arg2 );
}


/*
 * 'circle' skill command
 */

SKILL_RUNP( circle )
{
    Character *victim;
    Character *person;
    Object *obj;

    if ( MOUNTED(ch) )
    {
            ch->pecho(_("Только не верхом!"));
            return;
    }

    if ( ( victim = ch->fighting ) == 0 )
    {
            ch->pecho(_("Сейчас ты не сражаешься."));
            return;
    }

    if ( (obj = get_eq_char(ch,wear_wield)) == 0
            || (attack_table[get_weapon_attack(obj)].damage != DAM_PIERCE 
            && get_weapon_class(obj) != WEAPON_DAGGER))
    {
            ch->pecho(_("Вооружись для этого кинжалом или другим колющим оружием."));
            return;
    }

    if (is_safe_attack(ch,victim))
            return;

    for ( person = ch->in_room->people; person != 0; person = person->next_in_room )
    {
            if (person->fighting == ch)
            {
                    ch->pecho(_("Ты не можешь сделать это, защищаясь от ударов."));
                    return;
            }
    }
 
    CircleOneHit circ( ch, victim );
    int circleBonus = 0;
    circleBonus += skill_level_bonus(*gsn_circle, ch);

    if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
        ostringstream ostr;
        bonus_thief_skills->reportAction(ch->getPC(), ostr);
        ch->send_to(ostr);
        circleBonus+= number_range(20,25);
    }


    try {
        if (ch->is_npc() || number_percent( ) < min(100,circleBonus + gsn_circle->getEffective( ch )))
        {
                circ.hit( );
                gsn_circle->improve( ch, true, victim );
        }
        else
        {
                circ.miss( );
                gsn_circle->improve( ch, false, victim );
        }
    }
    catch (const VictimDeathException& e) {
    }
}


/*
 * 'key forgery' skill command
 */

SKILL_RUNP( forge )
{
    DLString args = argument, arg;
    Keyhole::Pointer keyhole;
    Object *key, *blank;

    if (( arg = args.getOneArgument( ) ).empty( )) {
        ch->pecho( _("Подделать что?") );
        return;
    }

    for (blank = get_obj_carry_type( ch, ITEM_LOCKPICK );
         blank && blank->value0() != Keyhole::LOCK_VALUE_BLANK;
         blank = get_obj_list_type( ch, ITEM_LOCKPICK, blank->next_content ))
        ;

    if (!blank) {
        ch->pecho( _("Тебе понадобится заготовка, чтобы создать дубликат или отмычку.") );
        return;
    }

    /*
     * create duplicate of a key
     */
    if (( key = get_obj_list_type( ch, arg, ITEM_KEY, ch->carrying ))) {
        Object *dup;

        if (!( keyhole = Keyhole::locate( ch, key ) )) {
            ch->pecho( _("Непонятно, что же открывает этот ключ.") );
            return;
        }

        if (!keyhole->isLockable( )) {
            ch->pecho( _("Это ключ от сломанного замка.") );
            return;
        }

        if (keyhole->isPickProof( )) {
            ch->pecho( _("Это ключ от замка, который невозможно взломать. Увы..") );
            return;
        }

        int chance;
        chance = min(100, gsn_key_forgery->getEffective( ch ) + skill_level_bonus(*gsn_key_forgery, ch));    
        
        if (number_percent( ) >= chance) {
            oldact(_("Тебе не удалось точно передать рисунок бороздок $o2."), ch, key, 0, TO_CHAR );
            gsn_key_forgery->improve( ch, false );
            return;
        }

        dup = create_object( key->pIndexData, 0 );
        dup->gram_gender = Grammar::MultiGender::MASCULINE;

        // A forged duplicate is named after the key it copies, and all three
        // strings were written to LANG_DEFAULT only, so EN and UA thieves carried
        // a Russian item for the rest of its life. The keyword went through the
        // single-argument setKeyword(), which re-splits a flattened list by
        // alphabet: Latin words to EN, Cyrillic to RU, nothing to UA. Write each
        // language from the source key's own name in that language instead. Every
        // token the old flattened list held still lives in one slot or another,
        // and matchesUnstrict scans them all, so targeting only widens.
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            lang_t lang = (lang_t)l;
            DLString keyName = key->getShortDescr( '2', lang );

            // getShortDescr(case, lang) drops back to LANG_DEFAULT on its own;
            // getKeyword(lang) does not, so an old key with no UA keyword would
            // otherwise leave the duplicate with a bare "дублікат" to aim at.
            DLString keyWords = key->getKeyword( lang );
            if (keyWords.empty())
                keyWords = key->getKeyword( LANG_DEFAULT );

            dup->setKeyword( fmtLang(lang, _("дубликат %s"),
                                     keyWords.c_str()), lang );
            dup->setShortDescr( fmtLang(lang, _("дубликат||а|у||ом|е %s"),
                                        keyName.c_str()), lang );
            dup->setDescription( fmtLang(lang, _("Дубликат %s лежит тут."),
                                         keyName.c_str()), lang );
        }
        dup->setMaterial( blank->getMaterial( ) );
        dup->wear_flags  = blank->wear_flags;
        dup->extra_flags = blank->extra_flags;
        dup->condition   = blank->condition;
        dup->weight      = blank->weight;
        // Worth what the blank was, not what the copied key was: otherwise a
        // cheap blank turns into an expensive key a shop will buy.
        dup->cost        = blank->cost;
        dup->value0(1);
        dup->value1(1);
        obj_to_char( dup, ch );

        oldact(_("Ты изготавливаешь $o4 из $O2."), ch, dup, blank, TO_CHAR );
        oldact(_("$c1 изготавливает $o4."), ch, key, 0, TO_ROOM );

        gsn_key_forgery->improve( ch, true );
        extract_obj( blank );
        return;
    }

    /*
     * create lockpick for a keyhole
     */
    if (( keyhole = Keyhole::create( ch, arg ) )) {
        if (!keyhole->isLockable( )) {
            ch->pecho( _("Здесь нет замочной скважины.") );
            return;
        }

        if (keyhole->isPickProof( )) {
            ch->pecho( _("Этот замок защищен от взлома.") );
            return;
        }

    int chance;
    chance = min(100, gsn_key_forgery->getEffective( ch ) + skill_level_bonus(*gsn_key_forgery, ch)); 

        if (number_percent( ) >= chance) {
            oldact(_("Твои попытки превратить $o4 в отмычку к этому замку ни к чему не привели."), ch, blank, 0, TO_CHAR );
            gsn_key_forgery->improve( ch, false );
            return;
        }

        oldact(_("$o1 в твоих умелых руках постепенно превращается в отмычку для $N2."), ch, blank, keyhole->getDescription( ).c_str( ), TO_CHAR );
        oldact(_("$c1 проделывает манипуляции с $o5."), ch, blank, 0, TO_ROOM );

        // A branded lockpick keeps its owner's name for good, so the old
        // LANG_DEFAULT-only write meant an EN or UA thief carried a Russian item
        // and read a Russian extra description every time they looked at it.
        // setKeyword() with one argument also splits the flattened list by
        // alphabet and never fills UA, so "відмичка" was untypeable.
        blank->gram_gender = Grammar::MultiGender::FEMININE;

        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            lang_t lang = (lang_t)l;

            blank->setKeyword( lmsg(lang, "lockpick", "отмычка", "відмичка"), lang );
        }

        // The keyword has to be complete before this: addProperDescription()
        // stamps the object's whole keyword list onto the new extra description,
        // and that is what `look отмычка` matches against later.
        ExtraDescription *ed = blank->addProperDescription();

        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            lang_t lang = (lang_t)l;

            // ItemKeyhole and ExtraExitKeyhole name the lock through
            // viewerLang(), so force the scope to get the lock's name in the
            // same language as the sentence around it. (DoorKeyhole builds its
            // text from Russian literals and stays Russian either way.)
            ForcedViewerLang scope(lang);

            blank->setShortDescr( fmtLang( lang,
                _("фирменн|ая|ой|ой|ую|ой|ой отмычк|а|и|е|у|ой|е %1$#^C2"), ch ), lang );
            blank->setDescription( fmtLang( lang,
                _("Фирменная отмычка (lockpick) %1$#^C2 оставлена тут хозя%1$#Gином|ином|йкой."), ch ), lang );
            ed->description[lang] = fmtLang( lang,
                _("Эта отмычка из 'Фирменного набора %1$#^C2' подходит для замка:\n%2$N2\n"),
                ch, keyhole->getDescription().c_str() );
        }

        blank->value0(keyhole->getLockType( ));
        blank->value1(50 + gsn_key_forgery->getEffective( ch ) / 2);

        gsn_key_forgery->improve( ch, true );
        return;
    }

    ch->pecho( _("У тебя нет такого ключа, и здесь нет такой замочной скважины.") );
}

