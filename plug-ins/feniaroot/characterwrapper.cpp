/* $Id: characterwrapper.cpp,v 1.1.4.50.4.40 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <jsoncpp/json/json.h>

#include "logstream.h"
#include "mobilebehaviormanager.h"
#include "basicmobilebehavior.h"

#include "skill.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "spelltarget.h"
#include "clan.h"
#include "behavior.h"
#include "setbehavior.h"
#include "affect.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "string_utils.h"
#include "morphology.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "raceflags.h"
#include "object.h"
#include "room.h"

#include "xmlattributestatistic.h"
#include "xmlattributetrust.h"
#include "xmlkillingattribute.h"
#include "fight_exception.h"
#include "subprofession.h"
#include "screenreader.h"
#include "profflags.h"
#include "occupations.h"
#include "follow_utils.h"
#include "interp.h"
#include "comm.h"
#include "save.h"
#include "player_exp.h"
#include "fight.h"
#include "weapons.h"
#include "damage.h"
#include "itemvalue.h"
#include "itemmodel.h"
#include "armorgenerator.h"
#include "skill_utils.h"
#include "stats_apply.h"
#include "rageoath.h"
#include "areaquestutils.h"
#include "material.h"
#include "feniaquest.h"
#include "immunity.h"
#include "magic.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "transfermovement.h"
#include "doors.h"
#include "merc.h"
#include "mobbody.h"
#include "mobtiers.h"
#include "loadsave.h"
#include "alignment.h"
#include "wiznet.h"
#include "fight_extract.h"
#include "xmlattributecoder.h"
#include "xmlattributerestring.h"
#include "commonattributes.h"
#include "player_menu.h"
#include "pet.h"
#include "recipeflags.h"
#include "damageflags.h"
#include "act.h"
#include "player_utils.h"
#include "religionutils.h"
#include "religion.h"
#include "websocketrpc.h"

#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "clantreasury.h"
#include "chatframe.h"
#include "context.h"
#include "nodes.h"
#include "codesource.h"
#include "wrappermanager.h"
#include "mobindexwrapper.h"
#include "objindexwrapper.h"
#include "structwrappers.h"
#include "affectwrapper.h"
#include "areaquestwrapper.h"
#include "xmlattributequestdata.h"
#include "xmleditorinputhandler.h"
#include "reglist.h"
#include "regcontainer.h"
#include "nativeext.h"
#include "idcontainer.h"
#include "fenia_utils.h"
#include "wrapperbase.h"
#include "stringset.h"
#include "../loadsave/behavior_utils.h"
#include "wrap_utils.h"
#include "subr.h"
#include "def.h"
#include "l10n.h"

// For reportSkills(): the pet-report skill gathering, migrated out of comm/report.cpp.
#include "spell.h"
#include "skillcommand.h"
#include "command.h"
#include "commandbase.h"
#include "commandflags.h"
#include "wearlocation.h"
#include "wearloc_utils.h"
#include "char_weight.h"

// gearAdvice v2.1: area-quest reward map + path-cost difficulty (traverse plugin).
#include "areaquest.h"
#include "roomtraverse.h"
#include "accountmanager.h"

RELIG(none);   // god_none sentinel for the gear-advisor tattoo-slot filter

GSN(dark_shroud);
GSN(manacles);
GSN(spellbane);
GSN(charm_person);
// Weapon/utility skills excluded from a weaponless pet's report, and the
// combat-invalid set -- kept in sync with comm/report.cpp's own gsn list.
GSN(lash);
GSN(bash);
GSN(hand_to_hand);
GSN(sword);
GSN(polearm);
GSN(dagger);
GSN(whip);
GSN(grip);
GSN(axe);
GSN(mace);
GSN(shield_block);
GSN(flail);
GSN(slice);
GSN(second_weapon);
GSN(pick_lock);
GSN(recall);
GSN(concentrate);
GSN(hide);
GSN(sneak);
GSN(detect_hide);
GSN(bash_door);
DESIRE(hunger);
DESIRE(bloodlust);
DESIRE(thirst);
DESIRE(drunk);

void password_set( PCMemoryInterface *pci, const DLString &plainText );
const char *ttype_name( int ttype );
DLString regfmt(Character *to, const RegisterList &argv);
list< ::Object *> get_objs_list_type( Character *ch, int type, ::Object *list );
void obj_from_anywhere( ::Object *obj );
void do_visible( Character * );

using namespace std;
using namespace Scripting;
using Scripting::NativeTraits;

NMI_INIT(CharacterWrapper, "персонаж (моб или игрок)")

CharacterWrapper::CharacterWrapper( ) : target( NULL )
{
}

void CharacterWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );
    
    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void CharacterWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        if (Scripting::gc)
            LogStream::sendError() << "Character wrapper: extract without target" << endl;
    }
    
    GutsContainer::extract( count );
}

bool CharacterWrapper::targetExists() const
{
    return true;
    
    if (!target)
        return false;
        
    if (zombie)
        return false;
    
    if (target->isDead())
        return false;

    if (target->getID() == 0)
        return false;

    return true;
}

void CharacterWrapper::setTarget( ::Character *target )
{
    this->target = target;
    id = target->getID( );
}

void CharacterWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "Character is dead" );

    if (target == NULL) 
        throw Scripting::Exception( "Character is offline" );
}

Character * CharacterWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

/*
 * FIELDS
 */
NMI_GET( CharacterWrapper, id, "уникальный идентификатор персонажа" )
{
    return Register( DLString(id) );
}

NMI_GET( CharacterWrapper, tier, "тир моба числом 1..10 (1 сильнейший), 0 для игроков; тир экземпляра, если его задали, иначе прототипа" )
{
    checkTarget();
    return Register( target->is_npc() ? target->getNPC()->getTier() : 0 );
}

NMI_SET( CharacterWrapper, tier, "задать тир экземпляру моба (1..10), 0 вернет тир прототипа; для квестовых спавнов, не для призванных игроками" )
{
    checkTarget();
    if (!target->is_npc())
        throw Scripting::Exception( "NPC field requested on PC" );
    int t = arg.toNumber( );
    if (t != 0 && (t < MobTiers::TIER_BEST || t > MobTiers::TIER_WORST))
        throw Scripting::Exception( "tier must be 0 or 1..10" );
    target->getNPC()->tier = t;
}

NMI_GET( CharacterWrapper, normalHit, "здоровье обычного (normal) моба этого уровня по fight/mob_tiers.json, 0 для игроков и без файла тиров" )
{
    checkTarget();
    const MobTiers::Config &tc = MobBody::tiers();
    if (!target->is_npc() || !tc.loaded)
        return Register( 0 );
    // Breath and other hp-scaled attacks read this instead of the mob's own hp,
    // so a tier makes a mob tougher without multiplying its burst damage too.
    int lvl = std::max(1, (int)target->getRealLevel());
    return Register( std::max(1, (int)lround(tc.baseHp.at(lvl) * tc.get(MobTiers::TIER_NORMAL).hp)) );
}

NMI_GET( CharacterWrapper, bloodless, "true, если у тела нет крови: скелет, конструкция, туман или нет ни сердца, ни холодной крови" )
{
    checkTarget();
    return Register( IS_BLOODLESS(target) );
}

NMI_GET( CharacterWrapper, edible, "true, если тело годится в пищу (не конструкция, скелет, туман, мгновенный распад или магическое)" )
{
    checkTarget();
    return Register( IS_EDIBLE_FORM(target->form) );
}

NMI_GET( CharacterWrapper, canHoldCards, "true, если персонаж разумен и у него есть кисти рук: может держать колоду или радугу" )
{
    checkTarget();
    return Register( CAN_HOLD_CARDS(target) );
}

NMI_GET( CharacterWrapper, online, "true, если персонаж в мире" )
{
    return Register( target != NULL );
}

NMI_GET( CharacterWrapper, dying, "true, пока из персонажа делают труп: вывод act/recho/rvecho/ptc ему и от него глушится" )
{
    checkTarget();
    return Register( target->dying );
}

NMI_SET( CharacterWrapper, dying, "true, пока из персонажа делают труп: вывод act/recho/rvecho/ptc ему и от него глушится" )
{
    checkTarget();
    target->dying = arg.toBoolean();
}

NMI_GET( CharacterWrapper, dead, "true, если персонажа уничтожили или моб только что умер" )
{
    if (zombie)
        return true;

    if (!target)
        return true;        

    checkTarget();
    return target->isDead();
}

NMI_SET( CharacterWrapper, dead, "true, если персонажа уничтожили или моб только что умер" )
{
    checkTarget( );
    target->setDead();
}

NMI_GET( CharacterWrapper, displayLang, "язык вывода для этого зрителя (0=en, 1=ru, 2=ua) с учётом 'config lang'; для мобов -- язык переключённого имма или дефолт. Передавай в .tables.X.messages(bits, gcase, ch.displayLang), чтобы флаги отображались на языке игрока." )
{
    checkTarget();
    return Register( (int)Player::displayLang(target) );
}


#define CHK_PC \
    if (!target->is_npc()) \
        throw Scripting::Exception( "NPC field requested on PC" ); 
#define CHK_NPC \
    if (target->is_npc()) \
        throw Scripting::Exception( "PC field requested on NPC" ); 

#define GETWRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    return wrap(target->x); \
}
#define GET_NPC_WRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    CHK_PC \
    return wrap(target->getNPC()->x); \
}
#define GET_PC_WRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    CHK_NPC \
    return wrap(target->getPC()->x); \
}

GET_NPC_WRAP( pIndexData, "структура с прототипом для всех мобов с данным vnum"
                          "(mob index data, т.е. то, редактируется с помощью OLC)")
GETWRAP( reply, "чар, который последний говорил с нами. по команде reply реплика отправится именно ему" )
GETWRAP( next, "следующий чар в глобальном списке всех чаров, .char_list" )
GETWRAP( next_in_room, "следующий чар в этой комнате, в списке people у комнаты" )
GETWRAP( master, "тот, за кем следуем" )
GETWRAP( leader, "лидер группы или тот, кто очаровал" )
GETWRAP( fighting, "тот, с кем сражаемся" )
GETWRAP( last_fought, "чар, с которым сражались последний раз" )
GET_PC_WRAP( pet, "моб, домашнее животное" )
GET_PC_WRAP( switchedTo, "в какого моба вселились" )
GETWRAP( doppel, "игрок, которому подражаем с помощью doppelganger. "
                 "для зеркал - игрок, который их создал" )
GET_PC_WRAP( guarding, "игрок, которого охраняем с помощью умения guard" )
GET_PC_WRAP( guarded_by, "игрок, который нас охраняет" )

GETWRAP( carrying, "первый объект в списке инвентаря/экипировки")
NMI_GET( CharacterWrapper, inventory, "список всех предметов в инвентаре" )
{
    RegList::Pointer rc( NEW );
    checkTarget( );
    
    for (::Object *obj = target->carrying; obj != 0; obj = obj->next_content)  
    if (obj->wear_loc == wear_none)
        rc->push_back(wrap(obj));

    return wrap(rc);
}

NMI_GET( CharacterWrapper, equipment, "список всех предметов в экипировке" )
{
    RegList::Pointer rc( NEW );
    checkTarget( );
    
    for (::Object *obj = target->carrying; obj != 0; obj = obj->next_content)  
    if (obj->wear_loc != wear_none)
            rc->push_back(wrap(obj));

    return wrap(rc);
}

NMI_GET( CharacterWrapper, items, "список всех предметов в инвентаре или экипировке" )
{
    RegList::Pointer rc( NEW );
    checkTarget( );
    
    for (::Object *obj = target->carrying; obj != 0; obj = obj->next_content)  
        rc->push_back(wrap(obj));

    return wrap(rc);
}


GETWRAP( on, "объект, мебель, на которой сидим" )

GETWRAP( in_room, "комната, в которой сейчас находимся" ) 
GETWRAP( was_in_room, "комната, в которой находились перед закапыванием в могилу")
GETWRAP( mount, "на ком мы верхом или кто верхом на нас" )
    
NMI_SET( CharacterWrapper, mount, "лидер группы или тот, кто очаровал" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
        target->mount = NULL;
    else
        target->mount = arg2character( arg );
}

NMI_SET( CharacterWrapper, leader, "лидер группы или тот, кто очаровал" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
        target->leader = NULL;
    else
        target->leader = arg2character( arg );
}
NMI_SET( CharacterWrapper, last_fought, "чар, с которым сражались последний раз" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
        target->last_fought = NULL;
    else
        target->last_fought = arg2character( arg );
}


#define ARMOR(x) \
NMI_GET( CharacterWrapper, armor##x, "класс брони" ) \
{ \
    checkTarget(); \
    return target->armor[x]; \
} \
NMI_SET( CharacterWrapper, armor##x, "класс брони" ) \
{ \
    checkTarget(); \
    target->armor[x] = arg.toNumber(); \
}

ARMOR(0)
ARMOR(1)
ARMOR(2)
ARMOR(3)
#undef ARMOR

NMI_GET( CharacterWrapper, pc, "экземпляр игрока" )
{
    checkTarget( );
    return wrap(target->getPC( ));
}

NMI_GET( CharacterWrapper, logon, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    return (int)target->getPC( )->age.getLogon( ).getTime( );
}
NMI_SET( CharacterWrapper, logon, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->age.setLogon( arg.toNumber( ) );
}
NMI_GET( CharacterWrapper, ageYears, "возраст персонажа в годах (текущий, с учётом магического старения APPLY_AGE)" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->age.getYears( );
}
NMI_GET( CharacterWrapper, ageTrueYears, "настоящий возраст персонажа в годах (без учёта магического старения)" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->age.getTrueYears( );
}
NMI_GET( CharacterWrapper, ageHours, "сколько реальных часов сыграно персонажем (истинное время, без APPLY_AGE)" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->age.getTrueHours( );
}
NMI_GET( CharacterWrapper, terminal_type, "тип терминала у mud-клиента" )
{
    checkTarget( );
    CHK_NPC
    if (!target->desc)
        return "";
    return ttype_name( target->desc->telnet.ttype );
}

NMI_GET( CharacterWrapper, webclient, "true если использует вебклиент" )
{
    checkTarget( );
    CHK_NPC
    if (!target->desc)
        return false;
    return is_websock(target);
}

NMI_GET( CharacterWrapper, attack_name, "англ название типа атаки (таблица в коде attack_table)")
{
    checkTarget();
    return attack_table[target->dam_type].name;
}

NMI_GET( CharacterWrapper, attack_noun, "русск название типа атаки (таблица в коде attack_table)")
{
    checkTarget();
    return attack_table[target->dam_type].noun;
}

NMI_GET( CharacterWrapper, attack_damage, "название типа повреждения (таблица .tables.damage_table)")
{
    checkTarget();
    
    return damage_table.name(attack_table[target->dam_type].damage);
}

NMI_SET( CharacterWrapper, damage_number, "повреждения моба: сколько раз кидать кубик" )
{
    checkTarget( );
    CHK_PC
    target->getNPC( )->damage[DICE_NUMBER] = arg.toNumber();        
}
NMI_SET( CharacterWrapper, damage_type, "повреждения моба: кол-во граней кубика" )
{
    checkTarget( );
    CHK_PC
    target->getNPC( )->damage[DICE_TYPE] = arg.toNumber();        
}
NMI_GET( CharacterWrapper, damage_number, "повреждения моба: сколько раз кидать кубик" )
{
    checkTarget( );
    CHK_PC
    return target->getNPC( )->damage[DICE_NUMBER];
}
NMI_GET( CharacterWrapper, damage_type, "повреждения моба: кол-во граней кубика" )
{
    checkTarget( );
    CHK_PC
    return target->getNPC( )->damage[DICE_TYPE];
}

NMI_INVOKE( CharacterWrapper, setLevel, "(level): установить уровень мобу" )
{
    checkTarget();
    CHK_PC

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    target->setLevel( args.front( ).toNumber( ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, applyTier, "(level, tier): числа моба как у свежего моба этого уровня и тира по fight/mob_tiers.json (hp, урон, хитролл, AC, мана, сейвы, кап статов); false без файла тиров" )
{
    checkTarget();
    CHK_PC

    if (args.size( ) < 2)
        throw Scripting::NotEnoughArgumentsException( );

    return Register( apply_mob_tier( target->getNPC( ), argnum2number( args, 1 ), argnum2number( args, 2 ) ) );
}

NMI_GET( CharacterWrapper, short_descr, "короткое описание моба" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getShortDescr(LANG_DEFAULT) );
}

NMI_SET( CharacterWrapper, short_descr, "короткое описание моба" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setShortDescr( arg.toString( ), LANG_DEFAULT );
}

NMI_INVOKE( CharacterWrapper, getShort, "(lang): короткое описание моба на языке lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getShortDescr( argnum2lang(args, 1) ) );
}

// Per-language getters, twins of getShort and of the setLong/setDescr dressers.
// A script that copies or appends to a mob's long/description per viewer (imp
// doppel, golem creator note) needs to read the slot it is about to rewrite;
// the plain long_descr/description getters answer LANG_DEFAULT (RU) only.
NMI_INVOKE( CharacterWrapper, getLong, "(lang): длинное описание моба на языке lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getLongDescr( argnum2lang(args, 1) ) );
}

NMI_INVOKE( CharacterWrapper, getDescr, "(lang): описание (look mob) на языке lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    return Register( target->getDescription( argnum2lang(args, 1) ) );
}

NMI_GET( CharacterWrapper, long_descr, "длинное описание моба" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getLongDescr(LANG_DEFAULT) );
}

NMI_SET( CharacterWrapper, long_descr, "длинное описание моба" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setLongDescr( arg.toString( ), LANG_DEFAULT );
}

NMI_GET( CharacterWrapper, keyword, "ключевые слова моба" )
{
    checkTarget( );
    CHK_PC
    return Register( String::toString(target->getNPC()->getKeyword()) );
}

NMI_SET( CharacterWrapper, keyword, "ключевые слова моба" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setKeyword( arg.toString( ) );
}

/* Per-language dressers, mirroring ObjectWrapper::setShort/setDescr, so a Fenia
 * script that creates a mob from a prototype can name and describe it in all
 * three languages (the plain short_descr/long_descr/keyword/description setters
 * write LANG_DEFAULT only). Used by autoquest scenario mobs. */
NMI_INVOKE( CharacterWrapper, setShort, "(text, lang): установить короткое описание моба для языка lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setShortDescr( argnum2string( args, 1 ), argnum2lang( args, 2 ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, setLong, "(text, lang): установить длинное описание моба для языка lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setLongDescr( argnum2string( args, 1 ), argnum2lang( args, 2 ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, setDescr, "(text, lang): установить описание (look mob) для языка lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setDescription( argnum2string( args, 1 ), argnum2lang( args, 2 ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, setKeyword, "(text, lang): установить ключевые слова моба для языка lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setKeyword( argnum2string( args, 1 ), argnum2lang( args, 2 ) );
    return Register( );
}

NMI_GET( CharacterWrapper, description, "то что видно по look mob" )
{
    checkTarget( );
    return Register( target->getDescription(LANG_DEFAULT) );
}

NMI_SET( CharacterWrapper, description, "то что видно по look mob" )
{
    checkTarget( );
    target->setDescription( arg.toString( ), LANG_DEFAULT );
}

NMI_GET( CharacterWrapper, spec_fun, "спец-процедура") 
{
    checkTarget( ); 
    CHK_PC
    if (target->getNPC()->spec_fun.func)
        return Register( spec_name(target->getNPC()->spec_fun.func) );
    else
        return Register( );
}

NMI_SET( CharacterWrapper, spec_fun, "спец-процедура") 
{
    checkTarget( ); 
    CHK_PC

    SPEC_FUN *spec;

    if (arg.type == Register::NONE)
        target->getNPC()->spec_fun.clear();
    else if ((spec = spec_lookup(arg.toString().c_str())) == 0)
        throw Scripting::Exception("Unknown spec function name");
    else
        target->getNPC()->spec_fun = spec;
}

NMI_GET(CharacterWrapper, questpoints, "qp")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getQuestPoints();
}
NMI_SET(CharacterWrapper, questpoints, "qp")
{
    checkTarget();
    CHK_NPC
    target->getPC()->setQuestPoints(arg.toNumber());
}

// Shared account bank. The account id is resolved from the char's own pfile
// link on every call, so a detach/attach takes effect at once. NONE means the
// char has no (existing) account: the caller falls back to its pfile bank.
static DLString bank_account_of( PCharacter *pch )
{
    DLString acct = AccountManager::accountOf( pch->getName( ) );
    if (acct.empty( ) || !AccountManager::exists( acct ))
        return DLString::emptyString;
    return acct;
}

NMI_INVOKE( CharacterWrapper, acctBank, "(currency): баланс общего банка аккаунта: gold, silver или qp; NONE без аккаунта" )
{
    checkTarget( );
    CHK_NPC
    DLString cur = args2string( args );
    if (cur != "gold" && cur != "silver" && cur != "qp")
        throw Scripting::Exception( "currency must be gold, silver or qp" );

    DLString acct = bank_account_of( target->getPC( ) );
    if (acct.empty( ))
        return Register( );
    return Register( AccountManager::bankBalance( acct, cur ) );
}

NMI_INVOKE( CharacterWrapper, acctBankAdd, "(gold, silver, qp): изменить общий банк аккаунта одной записью; false если без аккаунта, баланс ушел бы в минус или запись не удалась" )
{
    checkTarget( );
    CHK_NPC
    if (args.size( ) < 3)
        throw Scripting::NotEnoughArgumentsException( );

    DLString acct = bank_account_of( target->getPC( ) );
    if (acct.empty( ))
        return Register( false );
    return Register( AccountManager::bankAdd( acct,
        argnum2number( args, 1 ), argnum2number( args, 2 ), argnum2number( args, 3 ) ) );
}
NMI_INVOKE( CharacterWrapper, acctBankCap, "(): сколько золота можно держать в общем банке аккаунта; NONE без аккаунта" )
{
    checkTarget( );
    CHK_NPC

    DLString acct = bank_account_of( target->getPC( ) );
    if (acct.empty( ))
        return Register( );
    return Register( AccountManager::bankCapGold( acct ) );
}

NMI_GET( CharacterWrapper, trust, "уровень привилегий" )
{
    PCMemoryInterface *pci;
    
    checkTarget( );
    
    if (!target->is_npc( ) && target->getLevel( ) == 0) { // may be not loaded yet
        if (( pci = PCharacterManager::find( target->getPC()->getName( ) ) ))
            return pci->get_trust( );
        else
            return 0;
    }

    return target->get_trust( );
}

NMI_GET( CharacterWrapper, pretitle, "претитул" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getPretitle( ) );
}

NMI_SET( CharacterWrapper, pretitle, "претитул" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setPretitle( arg.toString( ) );
}

NMI_GET( CharacterWrapper, title, "титул" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getTitle( ) );
}

NMI_SET( CharacterWrapper, title, "титул" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setTitle( arg.toString( ) );
}

NMI_GET( CharacterWrapper, password, "пароль: deprecated" )
{
    PCMemoryInterface *pci;

    checkTarget( );
    CHK_NPC

    if (( pci = PCharacterManager::find( target->getPC()->getName( ) ) ))
        return pci->getPassword( );
    else
        return target->getPC( )->getPassword( );
}

NMI_SET( CharacterWrapper, password, "пароль" )
{
    checkTarget( );
    CHK_NPC
    password_set( target->getPC( ), arg.toString( ) );
}

NMI_GET( CharacterWrapper, remort_count, "кол-во ремортов" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getRemorts( ).size( ) );
}

// --- Remort service bindings ----------------------------------------------------
// Expose the remort economy so a Fenia behavior (mobs 'koschei' and 'baba yaga')
// can drive the player-facing dialogue, while the canonical remort data model --
// its caps, currencies and the hp/mana recompute -- stays here in C++. These are
// now the only home of that logic: the old C++ service-trade framework
// (RemortBonus / LifePrice / VictoryPrice in plug-ins/remort) was removed once
// both NPCs migrated to Fenia.

// Quest victories needed to earn one bonus life (was VictoryPrice::COUNT_PER_LIFE).
static const int VICTORY_COUNT_PER_LIFE = 500;

// Map a stat alias ("str".."con") to its stat_table index, or -1 if not a stat.
static int remort_stat_index( const DLString &kind )
{
    for (int i = 0; i < stat_table.size; i++)
        if (kind == stat_table.fields[i].name)
            return i;
    return -1;
}

// Remaining headroom for a remort bonus kind, mirroring RemortBonus::bonusMaximum.
static int remort_bonus_max( PCharacter *pch, const DLString &kind )
{
    if (kind == "hp" || kind == "mana")
        return 0xffff;
    if (kind == "level")
        return 3 - pch->getRemorts( ).level;
    if (kind == "pretitle")
        return pch->getRemorts( ).pretitle ? 0 : 1;

    int stat = remort_stat_index( kind );
    if (stat >= 0)
        return MAX_STAT_REMORT - pch->getMaxStat( stat );

    return 0;
}

// How much of a remort bonus kind the player currently holds -- the sellable
// amount -- mirroring RemortBonus::bonusBought / IntegerRemortBonus::bonusField.
static int remort_bonus_have( PCharacter *pch, const DLString &kind )
{
    Remorts &rem = pch->getRemorts( );

    if (kind == "hp")
        return rem.hp.getValue( );
    if (kind == "mana")
        return rem.mana.getValue( );
    if (kind == "level")
        return rem.level.getValue( );
    if (kind == "pretitle")
        return rem.pretitle ? 1 : 0;

    int stat = remort_stat_index( kind );
    if (stat >= 0)
        return rem.stats[stat].getValue( );

    return 0;
}

NMI_GET( CharacterWrapper, victoriesSpendable, "квестовые победы, доступные для обмена на ремортные бонусы" )
{
    checkTarget( );
    CHK_NPC
    XMLAttributeStatistic::Pointer attr
        = target->getPC( )->getAttributes( ).findAttr<XMLAttributeStatistic>( "questdata" );
    if (!attr)
        return Register( 0 );

    // Cap so that bonuses from victories plus from remorts never exceed MAX_BONUS_LIFES.
    int bonusLifesLeft = Remorts::MAX_BONUS_LIFES - target->getPC( )->getRemorts( ).countBonusLifes( );
    int avail = bonusLifesLeft * VICTORY_COUNT_PER_LIFE;
    int questVictories = attr->getBonusVictoriesCount( );
    if (questVictories < avail)
        avail = questVictories;

    int spendable = avail - attr->getWasted( );
    return Register( spendable < 0 ? 0 : spendable );
}

NMI_INVOKE( CharacterWrapper, spendVictories, "(n): списать n квестовых побед (необратимо), вернуть true при успехе" )
{
    checkTarget( );
    CHK_NPC
    int n = argnum2number( args, 1 );
    if (n <= 0)
        return Register( false );

    XMLAttributeStatistic::Pointer attr
        = target->getPC( )->getAttributes( ).getAttr<XMLAttributeStatistic>( "questdata" );
    attr->setWasted( attr->getWasted( ) + n );
    target->getPC( )->save( );
    return Register( true );
}

NMI_INVOKE( CharacterWrapper, remortBonusMax, "(kind): сколько ещё бонуса kind (str..con|hp|mana|level|pretitle) можно получить" )
{
    checkTarget( );
    CHK_NPC
    return Register( remort_bonus_max( target->getPC( ), argnum2string( args, 1 ) ) );
}

NMI_INVOKE( CharacterWrapper, remortBonusApply, "(kind[, amount]): выдать перманентный ремортный бонус kind, вернуть фактически выданное кол-во" )
{
    checkTarget( );
    CHK_NPC
    PCharacter *pch = target->getPC( );
    DLString kind = argnum2string( args, 1 );
    int amount = args.size( ) > 1 ? argnum2number( args, 2 ) : 1;

    int room = remort_bonus_max( pch, kind );
    if (room <= 0 || amount <= 0)
        return Register( 0 );
    if (amount > room)
        amount = room;

    Remorts &rem = pch->getRemorts( );

    if (kind == "pretitle") {
        rem.pretitle = true;
        amount = 1;
    }
    else if (kind == "level") {
        rem.level += amount;
    }
    else if (kind == "hp" || kind == "mana") {
        // Mirror AppliedRemortBonus: drop the current per-level contribution, bump the
        // stored bonus, re-apply -- per-level hp/mana depends on the cumulative total.
        bool isHp = (kind == "hp");
        for (int i = 1; i <= pch->getRealLevel( ); i++)
            if (isHp) {
                pch->max_hit  -= rem.getHitPerLevel( i );
                pch->perm_hit -= rem.getHitPerLevel( i );
            } else {
                pch->max_mana  -= rem.getManaPerLevel( i );
                pch->perm_mana -= rem.getManaPerLevel( i );
            }

        if (isHp)
            rem.hp += amount;
        else
            rem.mana += amount;

        for (int i = 1; i <= pch->getRealLevel( ); i++)
            if (isHp) {
                pch->max_hit  += rem.getHitPerLevel( i );
                pch->perm_hit += rem.getHitPerLevel( i );
            } else {
                pch->max_mana  += rem.getManaPerLevel( i );
                pch->perm_mana += rem.getManaPerLevel( i );
            }
    }
    else {
        int stat = remort_stat_index( kind );
        if (stat < 0)
            return Register( 0 );
        rem.stats[stat] += amount;
    }

    pch->save( );
    return Register( amount );
}

// Remort life points -- the currency Baba Yaga (Fenia 'baba yaga' behavior)
// trades in. Mirror the former LifePrice over getRemorts().points: a getter,
// spend (deduct on buy) and refund (induct on sell-back).
NMI_GET( CharacterWrapper, lifePoints, "ремортные очки жизни, доступные для обмена у Бабы Яги" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getRemorts( ).points.getValue( ) );
}

NMI_INVOKE( CharacterWrapper, spendLifePoints, "(n): списать n очков жизни (покупка у Бабы Яги), вернуть true при успехе" )
{
    checkTarget( );
    CHK_NPC
    int n = argnum2number( args, 1 );
    if (n <= 0)
        return Register( false );

    Remorts &rem = target->getPC( )->getRemorts( );
    if (rem.points.getValue( ) < n)
        return Register( false );

    rem.points -= n;
    target->getPC( )->save( );
    return Register( true );
}

NMI_INVOKE( CharacterWrapper, refundLifePoints, "(n): вернуть n очков жизни (продажа бонуса Бабе Яге)" )
{
    checkTarget( );
    CHK_NPC
    int n = argnum2number( args, 1 );
    if (n <= 0)
        return Register( false );

    target->getPC( )->getRemorts( ).points += n;
    target->getPC( )->save( );
    return Register( true );
}

NMI_INVOKE( CharacterWrapper, remortBonusRemove, "(kind[, amount]): снять ранее купленный ремортный бонус kind, вернуть фактически снятое кол-во" )
{
    checkTarget( );
    CHK_NPC
    PCharacter *pch = target->getPC( );
    DLString kind = argnum2string( args, 1 );
    int amount = args.size( ) > 1 ? argnum2number( args, 2 ) : 1;

    int have = remort_bonus_have( pch, kind );
    if (have <= 0 || amount <= 0)
        return Register( 0 );
    if (amount > have)
        amount = have;

    Remorts &rem = pch->getRemorts( );

    if (kind == "pretitle") {
        rem.pretitle = false;
        amount = 1;
    }
    else if (kind == "level") {
        rem.level -= amount;
    }
    else if (kind == "hp" || kind == "mana") {
        // Mirror AppliedRemortBonus::bonusSell: drop the current per-level
        // contribution, decrement the stored bonus, re-apply -- per-level hp/mana
        // depends on the cumulative total.
        bool isHp = (kind == "hp");
        for (int i = 1; i <= pch->getRealLevel( ); i++)
            if (isHp) {
                pch->max_hit  -= rem.getHitPerLevel( i );
                pch->perm_hit -= rem.getHitPerLevel( i );
            } else {
                pch->max_mana  -= rem.getManaPerLevel( i );
                pch->perm_mana -= rem.getManaPerLevel( i );
            }

        if (isHp)
            rem.hp -= amount;
        else
            rem.mana -= amount;

        for (int i = 1; i <= pch->getRealLevel( ); i++)
            if (isHp) {
                pch->max_hit  += rem.getHitPerLevel( i );
                pch->perm_hit += rem.getHitPerLevel( i );
            } else {
                pch->max_mana  += rem.getManaPerLevel( i );
                pch->perm_mana += rem.getManaPerLevel( i );
            }
    }
    else {
        int stat = remort_stat_index( kind );
        if (stat < 0)
            return Register( 0 );
        rem.stats[stat] -= amount;
    }

    pch->save( );
    return Register( amount );
}

NMI_GET( CharacterWrapper, altar, "vnum комнаты-алтаря в родном городе персонажа" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getHometown( )->getAltar( ) );
}


NMI_GET( CharacterWrapper, craftProfessions, "map из названия->уровень мастерства для дополнительных профессий" )
{
    ::Pointer<RegContainer> rc(NEW);
    checkTarget( );
    CHK_NPC
    list<CraftProfession::Pointer>::const_iterator p;
    list<CraftProfession::Pointer> profs = craftProfessionManager->getProfessions();
     
    for (p = profs.begin(); p != profs.end(); p++)
        (*rc)->map[(*p)->getName()] = Register((*p)->getLevel(target->getPC()));

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_GET(CharacterWrapper, perm_stat, "массив постоянных параметров персонажа")
{
    checkTarget();
    Register statsReg = Register::handler<RegContainer>();
    RegContainer *stats = statsReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto i = 0; i < stat_table.size; i++)
        stats->setField(i, target->perm_stat[i]);
        
    return statsReg;
}

NMI_GET(CharacterWrapper, curr_stat, "массив параметров с учетом вещей")
{
    checkTarget();
    Register statsReg = Register::handler<RegContainer>();
    RegContainer *stats = statsReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto i = 0; i < stat_table.size; i++)
        stats->setField(i, target->getCurrStat(i));
        
    return statsReg;
}

NMI_GET(CharacterWrapper, max_train, "массив максимально возможных значений параметров")
{
    checkTarget();
    Register statsReg = Register::handler<RegContainer>();
    RegContainer *stats = statsReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto i = 0; i < stat_table.size; i++)
        if (target->is_npc())
            stats->setField(i, MAX_STAT);
        else
            stats->setField(i, target->getPC()->getMaxTrain(i));
        
    return statsReg;
}


#define CONDITION(type, api) \
NMI_GET( CharacterWrapper, cond_##type, api ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    return Register( target->getPC( )->desires[desire_##type] ); \
} \
NMI_SET( CharacterWrapper, cond_##type, "изменить '" api "' на указанное число баллов" ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    desire_##type->gain( target->getPC( ), arg.toNumber( ) ); \
}

CONDITION(hunger,    "голод");
CONDITION(thirst,    "жажда");
CONDITION(bloodlust, "жажда крови");
CONDITION(drunk,     "опьянение");
#undef CONDITION


NMI_SET( CharacterWrapper, sex, "пол (таблица .tables.sex_table)")
{
    checkTarget( );
    target->setSex( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, sex, "пол (таблица .tables.sex_table)")
{
    checkTarget( );
    return target->getSex( );
}

NMI_GET( CharacterWrapper, wait, "wait state (в пульсах, 1 пульс = четверть секунды)")
{
    checkTarget( );
    return target->wait;
}

NMI_SET( CharacterWrapper, wait, "wait state (в пульсах, 1 пульс = четверть секунды); присваивается как есть, чтобы можно было и снять lag -- добавлять через .max(x.wait, n)")
{
    checkTarget( );

    // A plain assignment, so scripts can also shorten or clear lag. setWait only
    // ever raises it, which silently broke every restore and refund. Gods keep
    // setWait's 1-pulse rule.
    if (target->is_immortal( ))
        target->setWait( arg.toNumber( ) );
    else
        target->wait = max( 0, arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, boat, "объект лодки" )
{
    checkTarget( );
    return wrap( boat_object_find( target ) );
}

NMI_GET( CharacterWrapper, slow, "true если есть бит slow, нету хасты и (в случае мобов) бита fast")
{
    checkTarget();
    return IS_SLOW(target);
}

NMI_GET( CharacterWrapper, quick, "true если есть бит haste/fast и нету slow")
{
    checkTarget();
    return IS_QUICK(target);
}

NMI_GET( CharacterWrapper, flying, "true если мы GHOST, летаем или верхом на летающем скакуне" )
{
    checkTarget( );
    
    if (IS_GHOST(target))
        return true;
        
    if (is_flying( target ))
        return true;

    if (MOUNTED(target) && is_flying(MOUNTED(target)))
        return true;

    return false;
}

NMI_INVOKE( CharacterWrapper, flydown, "опуститься на землю без задержек, вернет true если до этого летали" )
{
    checkTarget();

    if (is_flying(target)) {
        target->posFlags.setBit( POS_FLY_DOWN );
        target->pecho( _("Ты перестаешь летать.") );
        target->recho( _("%1$^C1 переста%1$nет|ют летать."), target );
        return Register(true);
    }

    return Register(false);
}

NMI_GET( CharacterWrapper, ambushing, "строка, на кого сидим в засаде" )
{
    checkTarget( );
    return Register(target->ambushing);
}

NMI_SET( CharacterWrapper, ambushing, "строка, на кого сидим в засаде" )
{
    checkTarget();
    DLString str = arg2string(arg);
    target->ambushing = str;
}

NMI_GET( CharacterWrapper, neutral, "true если персонаж нейтральный" )
{
    checkTarget( );
    return IS_NEUTRAL(target);
}

NMI_GET( CharacterWrapper, evil, "true если персонаж злой" )
{
    checkTarget( );
    return IS_EVIL(target);
}

NMI_GET( CharacterWrapper, good, "true если персонаж добрый" )
{
    checkTarget( );
    return IS_GOOD(target);
}

NMI_GET( CharacterWrapper, alignName, "название натуры" )
{
    checkTarget( );
    return align_name( target );
}

NMI_GET( CharacterWrapper, mod_beats, "на сколько процентов увеличены или уменьшены задержки от умений" )
{
    checkTarget( );
    return target->mod_beats;
}

#define DEF_STAT(x, stat, help) \
NMI_GET( CharacterWrapper, cur_##x, "текущий параметр: " help ) \
{ \
    checkTarget( ); \
    return Register( target->getCurrStat(stat) ); \
} \
NMI_GET( CharacterWrapper, max_train_##x, "максимум тренировки для параметра: " help ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    return Register( target->getPC( )->getMaxTrain(stat) ); \
} \
NMI_GET( CharacterWrapper, perm_##x, "перманентный параметр: " help ) \
{ \
    checkTarget( ); \
    return Register( target->perm_stat[stat] ); \
} \
NMI_SET( CharacterWrapper, perm_##x, "перманентный параметр: " help ) \
{ \
    checkTarget( ); \
    int max_value = (target->is_npc( ) ? MAX_STAT : target->getPC( )->getMaxTrain(stat)); \
    target->perm_stat[stat] = URANGE(1, arg.toNumber( ), max_value); \
}

DEF_STAT(str, STAT_STR, "сила")
DEF_STAT(int, STAT_INT, "ум")
DEF_STAT(wis, STAT_WIS, "мудрость")
DEF_STAT(dex, STAT_DEX, "ловкость")
DEF_STAT(con, STAT_CON, "телосложение")
DEF_STAT(cha, STAT_CHA, "харизма")

#define STR_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x = arg.toString(); \
}

STR_FIELD(prompt, "строка состояния")
STR_FIELD(batle_prompt, "строка состояния в бою")

#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( (int) target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x = arg.toNumber(); \
}

#define FLAG_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( (int) target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x.setValue( arg.toNumber() ); \
}

INT_FIELD(ethos, "этос")
INT_FIELD(timer, "сколько минут прошло с последней команды")
INT_FIELD(daze, "daze state (в пульсах, 1 пульс = четверть секунды)")
INT_FIELD(hit, "текущее здоровье (hit points)")
INT_FIELD(max_hit, "максимальное здоровье")
INT_FIELD(mana, "текущая mana")
INT_FIELD(max_mana, "максимальная mana")
INT_FIELD(move, "текущие moves")
INT_FIELD(max_move, "максимальные moves")
INT_FIELD(gold, "золото")
INT_FIELD(silver, "серебро")
INT_FIELD(exp, "суммарный опыт")
INT_FIELD(invis_level, "уровень для wisinvis")
INT_FIELD(incog_level, "уровень для incognito")
INT_FIELD(lines, "кол-во строк в буфере вывода")
INT_FIELD(act, "act флаги для мобов и plr для игроков (таблицы .tables.act_flags и plr_flags)")
INT_FIELD(comm, "comm флаги (таблица .tables.comm_flags)")
INT_FIELD(add_comm, "расширение поля comm (таблица .tables.add_comm_flags)")
INT_FIELD(imm_flags, "флаги иммунитета (таблица .tables.imm_flags)")
INT_FIELD(res_flags, "флаги сопротивляемости (таблица .tables.res_flags)")
INT_FIELD(vuln_flags, "флаги уязвимости (таблица .tables.res_flags)")
INT_FIELD(affected_by, "флаги аффектов (таблица .tables.affect_flags)")
INT_FIELD(detection, "флаги детектов (таблица .tables.detect_flags)")
INT_FIELD(position, "позиция (таблица .tables.position_table)")
INT_FIELD(posFlags, "флаги позиции (таблица .tables.position_flags)")
INT_FIELD(carry_number, "количество вещей которое несет чар")
INT_FIELD(saving_throw, "савесы")
INT_FIELD(alignment, "натура, от -1000 до 1000")
INT_FIELD(hitroll, "точность")
INT_FIELD(damroll, "урон")
INT_FIELD(wimpy, "трусость. при скольки hp чар будет убегать автоматически")
INT_FIELD(dam_type, "тип повреждения (таблица .tables.weapon_flags)")
INT_FIELD(form, "форма тела (таблица .tables.form_flags)")
INT_FIELD(parts, "части тела (таблица .tables.part_flags)")
INT_FIELD(size, "размер (таблица .tables.size_table)")
INT_FIELD(death_ground_delay, "счетчик ловушки")
FLAG_FIELD(trap, "флаги ловушки (таблица .tables.trap_flags)")
INT_FIELD(riding, "если mount!=null: true - мы верхом, false - мы оседланы")

#undef INT_FIELD
#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    CHK_NPC \
    checkTarget( ); \
    return Register( (int) target->getPC()->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    CHK_NPC \
    checkTarget( ); \
    target->getPC()->x = arg.toNumber(); \
}

INT_FIELD(last_level, "какой был played, когда набили последний левел")
INT_FIELD(last_death_time, "когда последний раз был убит")
INT_FIELD(ghost_time, "сколько висит ghost")
INT_FIELD(PK_time_v, "сколько висит violent")
INT_FIELD(PK_time_sk, "сколько висит slain и killer")
INT_FIELD(PK_time_t, "сколько висит thief")
INT_FIELD(PK_flag, "KILLER, SLAIN, VIOLENT, GHOST, THIEF")
INT_FIELD(death, "сколько раз умирал")
INT_FIELD(perm_hit, "max hp без шмота")
INT_FIELD(perm_mana, "max mana без шмота")
INT_FIELD(perm_move, "max move без шмота")
INT_FIELD(practice, "сколько практик")
INT_FIELD(train, "сколько тренировок")
INT_FIELD(bank_s, "серебра в банке")
INT_FIELD(bank_g, "золота в банке")
INT_FIELD(config, "настройки чара (таблица .tables.config_flags)")
INT_FIELD(shadow, "сколько висеть тени (shadowlife) в секундах")
    
#undef INT_FIELD

#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    CHK_PC \
    checkTarget( ); \
    return Register( (int) target->getNPC()->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    CHK_PC \
    checkTarget( ); \
    target->getNPC()->x = arg.toNumber(); \
}
INT_FIELD(off_flags, "флаги поведения моба (таблица .tables.off_flags)")

NMI_GET(CharacterWrapper, last_fight_delay, "задержка после боя в секундах")
{
    checkTarget();
    return (int)target->getLastFightDelay();
}

NMI_GET(CharacterWrapper, adrenaline, "полна ли кровь адреналина")
{
    checkTarget();
    return target->is_adrenalined();
}

NMI_GET(CharacterWrapper, afterCharm, "очарован или недавно раз-очарован")
{
    checkTarget();
    if (target->is_npc())
        return target->getNPC()->behavior && target->getNPC()->behavior->isAfterCharm();
    else
        return IS_CHARMED(target);
}

NMI_GET(CharacterWrapper, start_room, "в какой комнате зашли в мир")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getStartRoom();
}

NMI_SET(CharacterWrapper, start_room, "в какой комнате зашли в мир")
{
    checkTarget();
    CHK_NPC
    target->getPC()->setStartRoom(arg2number(arg));
}

NMI_GET(CharacterWrapper, loyalty, "лояльность по отношению к закону")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getLoyalty();
}

NMI_SET(CharacterWrapper, loyalty, "лояльность по отношению к закону")
{
    checkTarget();
    CHK_NPC
    target->getPC()->setLoyalty(arg2number(arg));
}


NMI_SET( CharacterWrapper, wearloc, "названия всех слотов экипировки через пробел")
{
    checkTarget( );
    target->wearloc.fromString( arg.toString( ) );
}

NMI_GET( CharacterWrapper, wearloc, "названия всех слотов экипировки через пробел")
{
    checkTarget( );
    return target->wearloc.toString();
}

NMI_GET( CharacterWrapper, max_carry_weight, "макс вес, который может нести персонаж, 0 для петов, 100500 для богов")
{
    checkTarget( );
    return Char::canCarryWeight(target);
}

NMI_GET( CharacterWrapper, max_carry_number, "макс кол-во вещей, которое может нести персонаж, 0 для петов, 1000 для богов")
{
    checkTarget( );
    return Char::canCarryNumber(target);
}

NMI_GET(CharacterWrapper, carry_weight, "вес, который несет персонаж")
{
    checkTarget();
    return Char::getCarryWeight(target);
}


NMI_GET( CharacterWrapper, expToLevel, "сколько опыта осталось набрать до след уровня")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getExpToLevel( );
}

NMI_GET( CharacterWrapper, hostname, "IP-адрес соединения")
{
    checkTarget( );

    if (!target->desc)
        return "";
    else
        return target->desc->getRealHost( );
}

NMI_GET( CharacterWrapper, modifyLevel, "уровень с учетом бонусов от ремортов" )
{
    checkTarget( );
    return target->getModifyLevel( );
}

NMI_GET( CharacterWrapper, level, "настоящий уровень" )
{
    checkTarget( );
    return target->getRealLevel( );
}

NMI_SET( CharacterWrapper, level, "настоящий уровень" )
{
    checkTarget( );
    return target->setLevel( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, newbie, "true если нет ремортов, <50 квестов")
{
    checkTarget();
    CHK_NPC
    return Player::isNewbie(target->getPC());
}

NMI_GET( CharacterWrapper, lastAccessTime, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getLastAccessTime( ).getTimeAsString( );
}

NMI_GET( CharacterWrapper, profession, "класс (структура .Profession)" )
{
    checkTarget( );
    return Register::handler<ProfessionWrapper>(target->getProfession()->getName());
}

NMI_SET( CharacterWrapper, profession, "класс (структура .Profession)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setProfession( "none" );
    else
        target->getPC( )->setProfession( wrapper_cast<ProfessionWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, religion, "религия (структура .Religion)" )
{
    checkTarget( );
    CHK_NPC
    return Register::handler<ReligionWrapper>( target->getPC( )->getReligion( )->getName( ) );
}

NMI_GET( CharacterWrapper, godName, "название религии, случайный бог для неопределившихся или строка 'бог|и|ов...' для мобов" )
{
    checkTarget( );
    return ReligionUtils::godName(target);    
}

NMI_GET( CharacterWrapper, godReligion, "божество, которому молится персонаж (структура .Religion), или null -- в отличие от godName работает и для мобов, и отдаёт объект, а не русское имя" )
{
    checkTarget( );
    Religion *god = ReligionUtils::godReligion(target);
    if (!god)
        return Register( );

    // ReligionWrapper keeps only the name and re-resolves it with findExisting()
    // on every field read, so a handle built from a name that call cannot find is
    // non-null and throws on the FIRST read -- which is how a deityless healer's
    // armor spell took the game down after reboot #48. Resolve it here, through
    // the same call the wrapper will use, and hand back null when it fails, so a
    // plain null check is enough on the Fenia side.
    if (!religionManager->findExisting( god->getName( ) ))
        return Register( );

    return Register::handler<ReligionWrapper>( god->getName( ) );
}

NMI_SET( CharacterWrapper, religion, "религия (структура .Religion)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setReligion( "none" );
    else
        target->getPC( )->setReligion( wrapper_cast<ReligionWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, hometown, "родной город (структура .Hometown)" )
{
    checkTarget( );
    CHK_NPC
    return HometownWrapper::wrap( target->getPC( )->getHometown( )->getName( ) );
}

NMI_SET( CharacterWrapper, hometown, "родной город (структура .Hometown)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setHometown( "none" );
    else
        target->getPC( )->setHometown( wrapper_cast<HometownWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, clan, "клан (структура .Clan)" )
{
    checkTarget( );
    return ClanWrapper::wrap( target->getClan( )->getName( ) );
}

NMI_SET(CharacterWrapper, on, "объект, мебель, на которой сидим")
{
    checkTarget( );
    ::Object *obj = arg2item(arg);
    target->on = obj;
}

NMI_SET( CharacterWrapper, russianName, "русские имена с падежами" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setRussianName( arg.toString( ) );
}

NMI_GET( CharacterWrapper, russianName, "русские имена с падежами" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getRussianName( ).getFullForm( );
}

// The Ukrainian twin of the pair above. russianName has been reachable from
// Fenia for years while this slot was not, so the nanny had nowhere to put the
// name of a player who registers in Ukrainian and had to file it under Russian.
NMI_SET( CharacterWrapper, ukrainianName, "украинские имена с падежами" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setUkrainianName( arg.toString( ) );
}

NMI_GET( CharacterWrapper, ukrainianName, "украинские имена с падежами" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getUkrainianName( ).getFullForm( );
}

NMI_SET( CharacterWrapper, name, "имя" )
{
    checkTarget( );
    if (target->is_npc())
        target->getNPC()->setKeyword(arg.toString());
    else
        target->getPC()->setName(arg.toString());
}

NMI_GET( CharacterWrapper, name, "имя" )
{
    checkTarget( );
    return target->getNameC( );
}

NMI_GET( CharacterWrapper, race, "раса (структура .Race)" )
{
    checkTarget( );
    return RaceWrapper::wrap( target->getRace( )->getName( ) );
}

NMI_SET( CharacterWrapper, race, "раса (структура .Race)" )
{
    checkTarget( );
    if (arg.type == Register::NONE)
        target->setRace( "none" );
    else
        target->setRace( wrapper_cast<RaceWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, charmed, "true если очарован и есть хозяин" )
{
    checkTarget();
    return IS_CHARMED(target);
}

NMI_GET( CharacterWrapper, vampire, "true если персонаж в форме вампира или моб-вампир" )
{
    checkTarget();
    if (!target->is_npc())
        return target->is_vampire();
    else
        return IS_MOB_VAMPIRE(target);
}

NMI_GET(CharacterWrapper, followers, "список существ под очарованием, у которых персонаж master" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (Character *wch = char_list; wch; wch = wch->next) {
        if (IS_CHARMED(wch) && wch->master == target)
            rc->push_back(wrap(wch));
    }

    return wrap(rc);
}

NMI_GET( CharacterWrapper, connected, "true если есть связь" )
{
    Character *ch;
    
    checkTarget( );
    
    if (!target->is_npc( ) && target->getPC( )->switchedTo)
        ch = target->getPC( )->switchedTo;
    else
        ch = target;

    return (ch->desc != NULL);
}

NMI_GET( CharacterWrapper, isInInterpret, "true если игрок в состоянии ввода команд (не ed, не olc, не pager, etc)" )
{
    checkTarget();
    CHK_NPC
    return Register(target->desc && target->desc->handle_input.front( )->getType() == "InterpretHandler");
}

/*
 * METHODS
 */

NMI_INVOKE( CharacterWrapper, ptc, "(msg): print to char, печатает строку msg" )
{
    checkTarget( );
    if (target->dying)
        return Register();
    DLString d = args.front().toString();
    page_to_char(d.c_str(), target);
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret, "(msg): интерпретирует строку msg, как будто чар ее набрал сам" )
{
    checkTarget( );

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    DLString d = args.front().toString();
    return ::interpret( target, d.c_str() );
}

NMI_INVOKE( CharacterWrapper, interpret_raw, "(cmd, arg): выполняет команду с аргументами от имени чара, без предварительных проверок" )
{
    DLString cmdName, cmdArgs;
    RegisterList::const_iterator i;
    checkTarget( );

    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    cmdName = i->toString( );

    if (++i != args.end( ))
        cmdArgs = i->toString( );

    ::interpret_raw( target, cmdName.c_str( ), "%s", cmdArgs.c_str( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret_cmd, "(cmd, args): выполняет команду с аргументами от имени чара" )
{
    DLString cmdName, cmdArgs;
    RegisterList::const_iterator i;
    checkTarget( );

    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    cmdName = i->toString( );

    if (++i != args.end( ))
        cmdArgs = i->toString( );

    ::interpret_cmd( target, cmdName.c_str( ), "%s", cmdArgs.c_str( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, get_char_world, "(name[,flags]): найти персонажа в мире по имени name, с флагоми поиска (таблица .tables.find_flags)" )
{
    checkTarget( );
    DLString name = argnum2string(args, 1);
    bitstring_t flags = args.size() > 1 ? argnum2flag(args, 2, find_flags) : 0;

    return wrap(::get_char_world(target, name, flags));
}

NMI_INVOKE( CharacterWrapper, get_obj_here, "(name): видимый нам объект в комнате, инвентаре или equipment" )
{
    checkTarget( );
    return wrap( ::get_obj_here( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_carry_type, "(type): видимый нам объект в инвентаре или equipment с этим типом (таблица .tables.item_table)" )
{
    checkTarget( );
    return wrap( ::get_obj_carry_type( target, args2number( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_liquid_carry, "(liqname): вернет емкость в инвентаре с заданной жидкостью" )
{
    checkTarget( );

    DLString liqName = args2string(args);
    Liquid *liquid = liquidManager->find(liqName);
    if (!liquid)
        throw Scripting::Exception( "Invalid liquid name");

    list< ::Object *> drinks = ::get_objs_list_type(target, ITEM_DRINK_CON, target->carrying);
    for (list< ::Object *>::iterator o = drinks.begin(); o != drinks.end(); o++)
        if ((*o)->wear_loc == wear_none)
            if (liquidManager->find((*o)->value2()) == liquid)
                return wrap(*o);

    return Register();
}

NMI_INVOKE( CharacterWrapper, get_recipe_carry, "(flag): вернет рецепт в инвентаре с заданным флагом (таблица .tables.recipe_flags)" )
{
    checkTarget( );

    bitstring_t flag = args2number(args);
    list< ::Object *> recipes = ::get_objs_list_type(target, ITEM_RECIPE, target->carrying);
    for (list< ::Object *>::iterator o = recipes.begin(); o != recipes.end(); o++)
        if ((*o)->wear_loc == wear_none)
            if (IS_SET((*o)->value0(), flag))
                return wrap(*o);

    return Register();
}
NMI_INVOKE( CharacterWrapper, get_obj_room, "(name): поиск по имени видимого объекта в комнате" )
{
    checkTarget( );
    return wrap( ::get_obj_room( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_wear, "(name): поиск по имени видимого объекта в экипировке" )
{
    checkTarget( );
    return wrap( ::get_obj_wear( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_wear_vnum, "(vnum): поиск объекта в экипировке по внуму" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum && obj->wear_loc != wear_none)
            return wrap( obj );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, get_char_room, "(name[,room]): поиск по имени видимого персонажа, в той же комнате или в room" )
{
    checkTarget( );
    
    Room *room;
    DLString name = args2string( args );

    if (args.size( ) == 2)
        room = arg2room( args.back( ) );
    else
        room = target->in_room;
    
    return wrap( ::get_char_room( target, room, name ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_inventory, "(name): поиск объекта в инвентаре, по имени или ID" )
{
    checkTarget( );

    DLString objname = argnum2string(args, 1);
    
    return wrap( ::get_obj_carry( target, objname ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_carry, "(name[,looker]): поиск объекта в экипировке или видимого (себе или персонажу looker) объекта в инвентаре, по имени или ID" )
{
    checkTarget( );

    DLString objname = argnum2string(args, 1);
    Character *looker = args.size() <= 1 ? 0 : argnum2character(args, 2);
    
    return wrap( ::get_obj_wear_carry( target, objname, looker ) );
}


NMI_INVOKE( CharacterWrapper, transfer, "(room,actor,msgRoomLeave,msgSelfLeave,msgRoomEnter,msgSelfEnter): actor переносит нас в комнату room" )
{
    Room *room;
    Character *actor;
    RegisterList::const_iterator i = args.begin( );
    DLString m1, m2, m3, m4;
    
    checkTarget( );

    if (args.size( ) != 6)
        throw Scripting::NotEnoughArgumentsException( );
    
    room = arg2room( *i );
    actor = arg2character( *++i );
    m1 = (++i)->toString();
    m2 = (++i)->toString();
    m3 = (++i)->toString();
    m4 = (++i)->toString();
    transfer_char( target, actor, room, m1.c_str(), m2.c_str(), m3.c_str(), m4.c_str() );
    
    return Register( );
}

NMI_INVOKE(CharacterWrapper, transfer_silent, "(room): перенестись в комнату room, без сообщений и look")
{
    checkTarget();

    Room *room = argnum2room(args, 1);
    SilentTransferMovement(target, room).move();

    return Register();
}

NMI_INVOKE( CharacterWrapper, char_to_room, "(room): поместить в комнату room")
{
    checkTarget( );
    Room *room = arg2room( get_unique_arg( args ) ); 
    
    if (target->in_room) {
        undig( target );
        target->dismount( );
        ::char_from_room( target );
    }

    ::char_to_room( target, room );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, is_npc, "(): true для мобов, false для игроков" )
{
    checkTarget( );
    return Register( (int)target->is_npc( ) );
}

NMI_INVOKE( CharacterWrapper, getName, "(): имя игрока или список имен моба" )
{
    checkTarget( );
    return Register( target->getNameC() );
}

// Did this Flexer pad actually decline, i.e. does any case cell past the
// nominative carry a form? Both "cannot decline" answers come back with every
// case cell empty, but with a DIFFERENT cell count: the sidecar answers
// "word||||||" (a full 7-cell pad) when it does not recognise the word, while
// decline_sidecar's own fallback is "word|||||" for when the sidecar is
// unreachable. Testing the cells covers both; string-comparing one exact shape
// silently missed the other.
static bool pad_declined( const DLString &pad )
{
    DLString::size_type bar = pad.find( '|' );

    if (bar == DLString::npos)
        return false;

    return pad.find_first_not_of( '|', bar ) != DLString::npos;
}

// The STEM of a Flexer pad -- everything before the first '|', or the whole
// string when it carries no case forms at all. NOTE: this is the common prefix
// of the six declined forms, NOT the nominative. A pad is
// stem|nom-cell|gen-cell|dat-cell|acc-cell|instr-cell|loc-cell, so the
// nominative is stem + the first cell (see pad_case1). Олен|а|и|... has stem
// "Олен" but nominative "Олена". The two source-derivation call sites below feed
// this to declineUa; a stem that is a clean prefix of the name works, a
// stem-alternating one is a known limitation (fable review 2026-09-08).
static DLString pad_stem( const DLString &pad )
{
    DLString::size_type bar = pad.find( '|' );

    if (bar == DLString::npos)
        return pad;

    return pad.substr( 0, bar );
}

// The nominative a Flexer pad actually produces: stem + first case cell. For a
// bare pad ("Имя||||||") or an undivided string this is just the name.
static DLString pad_case1( const DLString &pad )
{
    DLString::size_type bar1 = pad.find( '|' );
    if (bar1 == DLString::npos)
        return pad;

    DLString::size_type bar2 = pad.find( '|', bar1 + 1 );
    if (bar2 == DLString::npos)
        bar2 = pad.size( );

    return pad.substr( 0, bar1 ) + pad.substr( bar1 + 1, bar2 - bar1 - 1 );
}

// The compiled half of the T8 name auto-fill: romanise / decline the login into
// whatever per-language form the player hasn't set. Kept in C++ for speed; the
// hot-reloadable global/onConnect trigger just calls autofillNameForms() on
// every character entering the game.
static void autofill_name_forms( PCharacter *pch )
{
    // English: romanise the login when no explicit form is set (a Latin login
    // romanises to itself).
    if (pch->getEnglishName( ).empty( )) {
        DLString en = String::translitToLatin( pch->getName( ) );
        if (!en.empty( ))
            pch->setEnglishName( en );
    }

    // Ukrainian: decline into a Flexer pad via the morphology sidecar. Feed it the
    // Russian nominative rather than the login -- Ukrainian morphology can do
    // nothing with a Latin "Telesyk" and hands back an empty pad, whereas the
    // Russian form is Cyrillic and declines properly ("Телесик" ->
    // "Телесик||а|ові|а|ом|ові"). Cyrillic logins with no Russian form yet still
    // fall back to the login itself.
    //
    // The condition covers an undeclined pad as well as a missing one: characters
    // that went through the earlier, broken version carry a stored "Login||||||",
    // which counts as "set", so a plain empty() test would never repair them.
    if (!pad_declined( pch->getUkrainianName( ).getFullForm( ) )) {
        DLString gender = "-";
        if (pch->getSex( ) == SEX_MALE)   gender = "masc";
        if (pch->getSex( ) == SEX_FEMALE) gender = "femn";

        // Prefer the Ukrainian slot's OWN nominative. A player who registered in
        // Ukrainian has the name they typed sitting right there, undeclined;
        // deriving it from the Russian form instead would round-trip it through
        // another alphabet for nothing. Only when that slot is empty does the
        // Russian form -- and finally the login -- become the source.
        DLString source = pad_stem( pch->getUkrainianName( ).getFullForm( ) );
        if (source.empty( ))
            source = pad_stem( pch->getRussianName( ).getFullForm( ) );
        if (source.empty( ))
            source = pch->getName( );

        // Ukrainian has no э/ы/ъ/ё, so a Russian name carrying one is a word the
        // Ukrainian dictionary simply cannot read: it declines nothing and cannot
        // even tell the name is animate. Rewriting those four letters first makes
        // ~60 more of the 602 affected names parse. и is deliberately left alone
        // -- it is a normal Ukrainian letter, so mapping it to і would be a
        // judgement about each name rather than transliteration.
        source = String::ruLettersToUa( source );

        // Names that do not decline must not be handed to the morphology
        // service: it has no notion of the class and will invent a paradigm --
        // it declined the indeclinable Кворо as a feminine noun
        // ("Квор|а|и|і|у|ою|і") and inflected Тайфоэн, a feminine name ending in
        // a consonant. Leaving the slot empty is right: the name map already
        // falls back to the Russian form, which carries the same single shape.
        if (!String::nameIsIndeclinable( source, pch->getSex( ) == SEX_FEMALE )) {
            // Store only a pad that really declined AND whose nominative
            // reproduces the input. nameIsIndeclinable is a front-guard against
            // classes morphology cannot read at all; this is the back-guard for
            // the ones it misreads with confidence. pymorphy hands back a full
            // paradigm for a name it has guessed wrong -- Диабол inflected as a
            // plural (nominative "Диаболи"), Лариена with its -а dropped
            // ("Лариен"), Сенька reshaped into "Сенько" -- and pad_declined() is
            // true for every one of them, because they DID inflect, just into the
            // wrong word. Compare the pad's actual nominative -- stem + first case
            // cell, NOT the bare stem (Олен|а|и| is nominative "Олена", stem
            // "Олен") -- back to the source: if it no longer equals the name we
            // fed in, the paradigm is invented. Leave the slot empty and let the
            // name map fall back to the Russian single form (correct), exactly as
            // for an indeclinable name.
            //
            // Note the guard cannot un-stick a bad pad already stored: a
            // non-empty field makes this autofill skip the character (line
            // above), and a plural-extension misread that this check let through
            // in an earlier build stays. Historical re-audit is a separate task.
            DLString pad = Morphology::declineUa( source, "NOUN", gender );
            if (pad_declined( pad ) && pad_case1( pad ) == source)
                pch->setUkrainianName( pad );
        }
    }
}

NMI_INVOKE( CharacterWrapper, autofillNameForms, "(): заполнить недостающие языковые формы имени игрока (T8)" )
{
    checkTarget( );
    PCharacter *pch = target->getPC( );
    if (pch != 0)
        autofill_name_forms( pch );
    return Register( );
}

NMI_GET( CharacterWrapper, baseLang, "язык регистрации (0=en,1=ru,2=ua); эта форма имени -- default, не редактируется" )
{
    checkTarget( );
    CHK_NPC
    return (int)target->getPC( )->getBaseLang( );
}
NMI_SET( CharacterWrapper, baseLang, "язык регистрации; ставится один раз в nanny" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setBaseLang( (lang_t)arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, created, "unix time the character was created (0 = unknown); survives remort" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getCreated( ) );
}
NMI_SET( CharacterWrapper, created, "unix creation time; stamped by the nanny (notifyCreated), backfilled for older characters" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setCreated( arg.toNumber( ) );
}

NMI_INVOKE( CharacterWrapper, seeName, "(ch[, case]): как мы видим имя и претитул ch в падеже case") 
{
    checkTarget( );
    int cse = 1;
    
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );

    Character *ch = arg2character( *i );

    i++;
    
    if(i != args.end())
        cse = i->toNumber();
        
    return Register( target->seeName(ch, '0' + cse ) );
}

NMI_INVOKE( CharacterWrapper, getParsedTitle, "DEPRECATED" )
{
    checkTarget();
    CHK_NPC
    return Player::title(target->getPC(), Player::displayLang(target));
}

NMI_GET( CharacterWrapper, parsedTitle, "титул персонажа как мы его видим" )
{
    checkTarget();
    CHK_NPC
    return Player::title(target->getPC(), Player::displayLang(target));
}

NMI_INVOKE( CharacterWrapper, can_see_mob, "(ch): видим ли персонажа ch" )
{
    checkTarget( );
    return target->can_see( arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_obj, "(obj): видим ли предмет obj" )
{
    checkTarget( );
    return target->can_see( arg2item( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_room, "(room): видим ли комнату room" )
{
    checkTarget( );
    return target->can_see( arg2room( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_exit, "(door): видим ли выход под номером door" )
{
    int door;
    EXIT_DATA *pExit;

    checkTarget( );
    door = args2number( args );
    if (door < 0 || door >= DIR_SOMEWHERE)
        throw Scripting::IllegalArgumentException( );

    if (!( pExit = target->in_room->exit[door] ))
        return false;

    return target->can_see( pExit );
}

NMI_INVOKE( CharacterWrapper, print, "(fmt, args): возвращает отформатированную строку (см. статью вики про ф-ии вывода)" )
{
    checkTarget();
    
    return Register( regfmt(target, args) );
}

NMI_INVOKE( CharacterWrapper, act, "(fmt, args): печатает нам отформатированную строку (с символом конца строки). " )
{
    checkTarget();

    // Death strips the body in silence (make_corpse sets dying): Remove
    // triggers still revoke skills and affects, but their fade lines would
    // only spam the dying char and the room.
    if (target->dying)
        return Register( );
    
    target->pecho( regfmt(target, args) );
    
    return Register( );
}

NMI_INVOKE( CharacterWrapper, echoMaster, "(fmt, args): выдать строку хозяину, если он есть и отдал этот приказ" )
{
    checkTarget();

    bool needsOutput = IS_CHARMED(target) 
            && target->master->getPC() 
            && target->master->getPC()->getAttributes().isAvailable("ordering");

    if (needsOutput) {
        DLString msg = fmt(target->master, _("{W%#^C1 {Wне может выполнить твой приказ, потому что видит следующее:{x\r\n  {W*{x "), target);
        target->master->pecho( msg + regfmt(target->master, args) );
        return true;
    }
    
    return false;
}

NMI_INVOKE( CharacterWrapper, recho, "(fmt, args): выводит отформатированную строку всем в комнате, кроме нас" )
{
    checkTarget();
    if (!target->in_room || target->dying)
        return Register();

    for (Character *to = target->in_room->people; to; to = to->next_in_room) {
        if (to == target)
            continue;
        if (!to->can_sense(target))
            continue;

        to->pecho(POS_RESTING, regfmt(to, args).c_str());
    }

    return Register();
}

NMI_INVOKE( CharacterWrapper, rvecho, "(vict, fmt, args...): выводит отформатированную строку всем в комнате, кроме нас и vict" )
{
    checkTarget();
    if (!target->in_room || target->dying)
        return Register();

    RegisterList myArgs(args);
    Character *vict = args2character(args);
    myArgs.pop_front();

    for (Character *to = target->in_room->people; to; to = to->next_in_room) {
        if (to == target || to == vict)
            continue;            
        if (!to->can_sense(target))
            continue;

        to->pecho(POS_RESTING, regfmt(to, myArgs).c_str());
    }

    return Register( );
}

/*-------------------------------------------------------------------------
 * Chat frames: who is speaking, and about what
 *------------------------------------------------------------------------*/

/** The codesource that is running right now, or an empty string when C++ is.
 *
 *  The runtime keeps the chain the exception backtrace prints ("in cs #317
 *  (areas/drow.are/mob/5107.q5100_step0_begin_postGreet) line 47"), so a mob
 *  line can be filed under the quest that produced it without any script being
 *  edited. A postponed trigger keeps the chain too -- its body is invoked
 *  through the ordinary path -- so post* speech is tagged like any other. */
static DLString chat_codesource( )
{
    using namespace Scripting;

    Context *ctx = Context::current;

    if (!ctx || !ctx->nodeTrace || !ctx->nodeTrace->node)
        return DLString::emptyString;

    // NEVER DEREFERENCE source.source. It can dangle: a mass free destroys the
    // CodeSource while a Closure still holds one of its functions, and reading
    // through the stale pointer is exactly the crash of 2026-08-08 ("manager at
    // 0x68", four crash-looping boots) that Function::finalize was rewritten to
    // avoid. Do what that fix does: look the source up by the stored id, and
    // only use it when the live entry is the SAME object -- ids wrap.
    const CodeSourceRef &ref = ctx->nodeTrace->node->source;
    if (!ref.csId || !CodeSource::manager)
        return DLString::emptyString;

    CodeSource::Manager::iterator it = CodeSource::manager->find(ref.csId);
    if (it == CodeSource::manager->end())
        return DLString::emptyString;

    if (&*it != ref.source.getPointer())
        return DLString::emptyString;

    return it->name;
}

/** q<number>_step<number>_ anywhere in the codesource name: the quest and the
 *  step this speech belongs to. Everything else is ordinary mob chatter. */
static bool chat_quest_tag( const DLString &name, int &quest, int &step )
{
    static const DLString STEP = "_step";

    for (size_t at = name.find('q'); at != DLString::npos; at = name.find('q', at + 1)) {
        size_t p = at + 1;
        size_t digits = p;

        while (p < name.size() && isdigit((unsigned char)name[p]))
            p++;

        if (p == digits)
            continue;

        if (name.compare(p, STEP.size(), STEP) != 0)
            continue;

        int q = atoi(name.substr(digits, p - digits).c_str());
        p += STEP.size();
        digits = p;

        while (p < name.size() && isdigit((unsigned char)name[p]))
            p++;

        if (p == digits || p >= name.size() || name[p] != '_')
            continue;

        quest = q;
        step = atoi(name.substr(digits, p - digits).c_str());
        return true;
    }

    return false;
}

/** One mob line, to one listener. Quest speech goes into its own thread, keyed
 *  by the quest: a dialogue read end to end is the thing a player comes back
 *  to, and mixing it with room chatter destroys exactly that. */
static void chat_emit_mob( Character *to, Character *mob, const DLString &line )
{
    if (!chat_subscribed( to ))
        return;

    int quest = -1, step = -1;
    DLString cs = chat_codesource( );

    if (!cs.empty( ) && chat_quest_tag( cs, quest, step )) {
        chat_emit( to, mob, false, "quest", "quest", line, DLString::emptyString, quest, step );
        return;
    }

    chat_emit( to, mob, false, "mob", "mob", line );
}

NMI_INVOKE( CharacterWrapper, say, "(format, args...): произносит вслух реплику, отформатированную как в методе act" )
{
    checkTarget( );

    for (Character *to = target->in_room->people; to; to = to->next_in_room) {
        if (to == target)
            continue;
        if (!to->can_sense(target))
            continue;

        DLString msg = regfmt(to, args);
        to->pecho(POS_RESTING, _("%^C1 произносит '{g%s{x'"), target, msg.c_str());

        // The same line again for the panel, and only when there is a panel:
        // the console path above is left exactly as it was, and a listener
        // without a subscription costs one boolean. The position test is
        // pecho's own -- a sleeping character is shown neither.
        if (chat_subscribed(to) && to->position >= POS_RESTING)
            chat_emit_mob(to, target, fmt(to, _("%^C1 произносит '{g%s{x'"), target, msg.c_str()));
    }

    return Register();
}

NMI_INVOKE( CharacterWrapper, psay, "(ch, format, args...): произносит вслух реплику, отформатированную как в методе act и видимую только для ch" )
{
    checkTarget( );
    RegisterList myArgs(args);
    Character *ch= args2character(args);
    myArgs.pop_front();

    DLString msg = regfmt(ch, myArgs);
    ch->pecho(_("%^C1 произносит '{g%s{x'"), target, msg.c_str());

    // One listener, one frame -- psay is a mob speaking to one person, and the
    // panel files it the same way as anything else the mob says.
    if (chat_subscribed(ch))
        chat_emit_mob(ch, target, fmt(ch, _("%^C1 произносит '{g%s{x'"), target, msg.c_str()));
    return Register();
}


NMI_INVOKE( CharacterWrapper, getModifyLevel, "(): уровень, с учетом плюшек от ремортов" )
{
    checkTarget();
    
    return target->getModifyLevel();
}

NMI_INVOKE( CharacterWrapper, getRealLevel, "(): настоящий уровень" )
{
    checkTarget();
    
    return target->getRealLevel();
}

NMI_INVOKE( CharacterWrapper, getSex, "(): номер пола (0 neutral, 1 male, 2 female, 3 random - только у прототипов)" )
{
    checkTarget();
    
    return target->getSex();
}

NMI_INVOKE( CharacterWrapper, is_immortal, "(): true, если this бессмертный или кодер" )
{
    checkTarget();
    
    return target->is_immortal();
}

NMI_INVOKE( CharacterWrapper, edit, "(): переводит this в режим редактирования" )
{
    checkTarget();
    
    PCharacter *pch = target->getPC();
    
    if(!pch)
        throw Scripting::Exception( "only for PCs" );
    
    DLString str;
    
    XMLEditorInputHandler::Pointer eih( NEW );
    
    if(!args.empty()) {
        eih->clear( );
        eih->setBuffer(args.front().toString());
    }

    eih->attach(pch);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, edReg, "([ndx[, txt]]): возвращает/устанавливает содержимое регистров редактора" )
{
    RegisterList::const_iterator i = args.begin( );

    checkTarget();
    
    PCharacter *pch = target->getPC();
    
    if(!pch)
        throw Scripting::Exception( "only for PCs" );
    
    unsigned char ndx = 0;

    if(i != args.end()) {
        ndx = i->toNumber();
        i++;
    }

    Editor::reg_t &reg = pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[ndx];

    DLString str;

    if(i == args.end())
        for(Editor::reg_t::const_iterator j = reg.begin(); j != reg.end(); j++)
            str.append(*j).append("\n");
    else 
        reg.split(str = i->toString());

    return Register(str);
}


NMI_INVOKE( CharacterWrapper, gainExp, "(exp): добавляет exp очков опыта" )
{
    checkTarget( );
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );
    
    CHK_NPC
    Player::gainExp(target->getPC(), i->toNumber());

    return Register();
}

NMI_INVOKE( CharacterWrapper, getClass, "(): строка с названием класса" )
{
    checkTarget();
    return Register( target->getProfession( )->getName( ).c_str( ) );
}
NMI_INVOKE( CharacterWrapper, getClan, "(): строка с названием клана" )
{
    checkTarget();
    return Register( target->getClan( )->getShortName( ) );
}
NMI_INVOKE( CharacterWrapper, setClan, "(name): устанавливает клан по строке с именем" )
{
    Clan *clan;
    
    checkTarget();

    if (args.empty())
        throw Scripting::NotEnoughArgumentsException( );
    
    clan = ClanManager::getThis( )->findUnstrict( args.front( ).toString( ) );

    if (!clan)
        throw Scripting::IllegalArgumentException( );
    else
        target->setClan( clan->getName( ) );
    
    return Register( );
}
NMI_GET( CharacterWrapper, rageDeflect, "шанс ауры ярости (spellbane) отразить прицельное заклинание, в процентах; 0 без ауры" )
{
    checkTarget();
    if (!target->isAffected(gsn_spellbane))
        return 0;
    return rage_deflect( target );
}

NMI_INVOKE( CharacterWrapper, rageAreaBane, "(caster[,retaliate[,skillName[,quiet]]]): колдовство на всю комнату или местность дошло до персонажа; true, если аура ярости его отвела (половинный шанс). skillName решает, молитва это или магия; quiet -- без сообщений" )
{
    checkTarget();
    Character *caster = argnum2character(args, 1);
    bool retaliate = args.size() >= 2 && argnum2number(args, 2);
    bool prayer = false;
    if (args.size() >= 3) {
        Skill *skill = argnum2skill(args, 3);
        prayer = skill->getSpell() && skill->getSpell()->isPrayer(caster);
    }
    bool quiet = args.size() >= 4 && argnum2number(args, 4);

    try {
        return rage_area_bane( caster, target, prayer, retaliate, quiet );
    } catch (const VictimDeathException &) {
        return true;
    }
}

NMI_GET( CharacterWrapper, clanPower, "сила клановых умений по рангу в клане после реформы, в процентах (100 вне реформы)" )
{
    checkTarget();
    CHK_NPC
    return clan_rank_power( *target->getClan( ), target->getPC( )->getClanLevel( ) );
}
NMI_INVOKE( CharacterWrapper, getClanLevel, "(): клановый уровень, число от 0 до 8" )
{
    checkTarget();
    CHK_NPC
    return Register( target->getPC()->getClanLevel() );
}
NMI_INVOKE( CharacterWrapper, setClanLevel, "(уровень): клановый уровень, число от 0 до 8" )
{
    checkTarget();
    CHK_NPC
    target->getPC()->setClanLevel(args2number(args));
    return Register();
}
NMI_INVOKE( CharacterWrapper, getRace, "(): строка с названием расы" )
{
    checkTarget();
    return Register( target->getRace( )->getName( ) );
}

NMI_INVOKE( CharacterWrapper, extract, "(bool): уничтожить полностью (suicide/remort игрока или смерть моба) или не полностью как при выходе из мира" )
{
    checkTarget( );
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );
    
    extract_char(target, i->toNumber());
    return Register();
}

NMI_INVOKE( CharacterWrapper, add_follower, "(master): делает нас последователем master-а" )
{
    checkTarget( );
    follower_add(target, arg2character( get_unique_arg( args ) ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, stop_follower, "([verbose]): прекращает следование, снимает очарование" )
{
    checkTarget( );

    bool verbose = true;
    if (args.size() > 0)
        verbose = args.front().toBoolean();

    follower_stop(target, verbose);
    return Register();
}

NMI_INVOKE( CharacterWrapper, is_same_group, "(gch): вернет true если мы с персонажем gch в одной группе" )
{
    checkTarget( );
    Character *gch = arg2character(get_unique_arg(args));
    return Register(is_same_group(target, gch));
}

NMI_GET(CharacterWrapper, groupHere, "список (List) всех согруппников в комнате")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (Character *rch = target->in_room->people; rch; rch = rch->next_in_room)
        if (is_same_group(target, rch))
            rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    return wrap(rc);
}

NMI_INVOKE( CharacterWrapper, clearBehavior, "(): сбросить поведение моба до обычного" )
{
    checkTarget( );
    CHK_PC
    MobileBehaviorManager::assignBasic( target->getNPC( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, rememberFought, "(ch): запомнить персонажа ch как будто с ним сражались" )
{
    checkTarget();
    CHK_PC

    if (!target->getNPC()->behavior)
        return Register(false);

    BasicMobileBehavior::Pointer ai = target->getNPC()->behavior.getDynamicPointer<BasicMobileBehavior>();
    if (!ai)
        return Register(false);

    Character *ch = args2character(args);
    ai->rememberFought(ch);
    return Register(true);
}

// Default C++ brain of a mob (BasicMobileBehavior and descendants), or null.
static BasicMobileBehavior::Pointer mob_ai( Character *ch )
{
    if (!ch->is_npc( ) || !ch->getNPC( )->behavior)
        return BasicMobileBehavior::Pointer( );

    return ch->getNPC( )->behavior.getDynamicPointer<BasicMobileBehavior>( );
}

// true if the optional argument 'num' asks for the 'attacked' memory instead of 'fought'
static bool ai_memory_attacked( const RegisterList &args, int num )
{
    return (int)args.size( ) >= num && argnum2boolean( args, num );
}

NMI_GET( CharacterWrapper, ai_lastFought, "имя игрока, с которым моб сражался последним и которого ищет (пустая строка, если никого)" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( "" );
    return Register( ai->getLastFoughtName( ) );
}

NMI_INVOKE( CharacterWrapper, ai_setLastFought, "(ch): запомнить игрока ch как последнего противника, которого моб будет выслеживать" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai || !target->in_room)
        return Register( false );
    ai->setLastFought( argnum2character( args, 1 ) );
    return Register( true );
}

NMI_INVOKE( CharacterWrapper, ai_clearLastFought, "(): забыть последнего противника и прекратить погоню" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    ai->clearLastFought( );
    return Register( true );
}

NMI_INVOKE( CharacterWrapper, ai_memorized, "(ch[, attacked]): помнит ли моб ch среди тех, с кем дрался (или на кого нападал, если attacked)" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->aiMemorized( argnum2character( args, 1 ), ai_memory_attacked( args, 2 ) ) );
}

NMI_INVOKE( CharacterWrapper, ai_remember, "(ch[, attacked]): запомнить ch среди тех, с кем дрался (или на кого нападал, если attacked)" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    ai->aiRemember( argnum2character( args, 1 ), ai_memory_attacked( args, 2 ) );
    return Register( true );
}

NMI_INVOKE( CharacterWrapper, ai_forget, "(ch[, attacked]): забыть ch в памяти о драках (или о нападениях, если attacked)" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->aiForget( argnum2character( args, 1 ), ai_memory_attacked( args, 2 ) ) );
}

NMI_GET( CharacterWrapper, ai_lostTrack, "моб потерял след последнего противника" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->getLostTrack( ) );
}

NMI_SET( CharacterWrapper, ai_lostTrack, "моб потерял след последнего противника" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (ai)
        ai->setLostTrack( arg.toBoolean( ) );
}

NMI_GET( CharacterWrapper, ai_homeVnum, "внум комнаты, где моб начал погоню и куда вернется (0, если не запомнена)" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( 0 );
    return Register( ai->getHomeVnum( ) );
}

NMI_INVOKE( CharacterWrapper, ai_goHome, "([always]): вернуться домой после погони; always - исчезнуть, если дороги домой нет (после этого проверять .tmp.mob.valid)" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai || !target->in_room)
        return Register( false );
    return Register( ai->goHome( !args.empty( ) && argnum2boolean( args, 1 ) ) );
}

NMI_INVOKE( CharacterWrapper, ai_attack, "(victim): напасть на victim так, как нападает моб (охранник жертвы, onAttackAI); false, если victim не здесь или под защитой богов. Очарованность не проверяет" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->aiAttack( argnum2character( args, 1 ) ) );
}

NMI_INVOKE( CharacterWrapper, ai_trackStep, "(quarry): один шаг погони за quarry - призвать его или пройти по следам; после этого проверять .tmp.mob.valid" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->aiTrackStep( argnum2character( args, 1 ) ) );
}

NMI_INVOKE( CharacterWrapper, ai_rangedAttack, "(): выстрелить или ударить заклинанием по запомненному врагу в соседних комнатах. Очарованность не проверяет" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->aiRangedAttack( ) );
}

NMI_INVOKE( CharacterWrapper, ai_assistGroup, "(fch, victim): помочь соратнику fch против victim - полечить, отойти для стрельбы или вступить в бой" )
{
    checkTarget( );
    CHK_PC
    BasicMobileBehavior::Pointer ai = mob_ai( target );
    if (!ai)
        return Register( false );
    return Register( ai->aiAssistGroup( argnum2character( args, 1 ), argnum2character( args, 2 ) ) );
}

NMI_INVOKE( CharacterWrapper, yellPanic, "(attacker, msgBlind, msg[, label]): крикнуть о помощи против attacker, как кричат жертвы нападения" )
{
    checkTarget( );
    Character *attacker = argnum2character( args, 1 );
    DLString msgBlind = argnum2string( args, 2 );
    DLString msg = argnum2string( args, 3 );
    DLString label = args.size( ) > 3 ? argnum2string( args, 4 ) : DLString::emptyString;

    yell_panic( attacker, target, msgBlind.c_str( ), msg.c_str( ), label.empty( ) ? 0 : label.c_str( ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, get_random_room, "(): случайная комната, куда можно зайти" )
{
    checkTarget( );
    
    RoomVector rooms;
    
    for (auto &r: roomInstances)
        if (target->canEnter(r) && !r->isPrivate())
            rooms.push_back(r);
    
    if (rooms.empty())
        return Register( );
    else {
        Room *r = rooms[::number_range(0, rooms.size() - 1)];
        return WrapperManager::getThis( )->getWrapper(r); 
    }
}

NMI_INVOKE( CharacterWrapper, is_safe, "(vict): защищают ли боги vict от нас" )
{
    checkTarget( );
    return ::is_safe_nomessage( target, 
                                arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, donatesTo, "(ch): раса этого персонажа отдает свои вещи расе ch по просьбе (кентавры и прочие)" )
{
    checkTarget( );
    Character *other = args2character( args );
    return getTarget( )->getRace( )->getAttitude( *other->getRace( ) ).isSet( RACE_DONATES );
}

NMI_INVOKE( CharacterWrapper, is_safe_msg, "(vict): то же, что is_safe, но с сообщением 'под защитой богов', как у C++ умений" )
{
    checkTarget( );
    return ::is_safe( target, arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, setLastFightTime, "(): отметить персонажа как недавно сражавшегося (адреналин)" )
{
    checkTarget( );
    target->setLastFightTime( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, is_safe_spell, "(vict): защищают ли боги vict от наших арийных заклинаний" )
{
    checkTarget();
    return ::is_safe_spell(target,
                           args2character(args),
                           true);
}

NMI_INVOKE( CharacterWrapper, is_safe_rspell, "(af): защищают ли боги от действия заклинания af на комнате" )
{
    checkTarget();
    Affect *paf = args2affect(args);
    return ::is_safe_rspell(paf, target, true);
}

NMI_INVOKE( CharacterWrapper, rawdamage, "(vict,dam,damtype[,label]): нанести vict повреждения в размере dam с типом damtype (таблица .tables.damage_table)" )
{
    RegisterList::const_iterator i;
    Character *victim;
    int dam;
    int dam_type = DAM_NONE;
    DLString label;

    checkTarget( );

    if (args.size() < 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    victim = argnum2character(args, 1);
    dam = argnum2number(args, 2);
    dam_type = argnum2flag(args, 3, damage_table);
    if (args.size() > 3)
        label = argnum2string(args, 4);

    ::rawdamage_nocatch(target, victim, dam_type, dam, true, label);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, setViolent, "(vict): установить VIOLENT за нападение на vict" )
{
    checkTarget();
    CHK_NPC
    Character *victim = args2character(args);
    ::set_violent( target, victim, true );
    return Register();
}

NMI_INVOKE( CharacterWrapper, damage, "(vict,dam,skillName,damtype,damflags[,show]): нанести vict повреждения в размере dam умением skillName и типом damtype (таблица .tables.damage_table)" )
{
    checkTarget( );
    Character *victim = argnum2character(args, 1);
    int dam = argnum2number(args, 2);
    Skill *skill = argnum2skill(args, 3);
    int dam_type = argnum2flag(args, 4, damage_table);
    bitstring_t damflags = argnum2flag(args, 5, damage_flags);
    bool show = args.size() > 5 ? argnum2boolean(args, 6) : true;

    return ::damage_nocatch(target, victim, dam, skill->getIndex( ), dam_type, show, damflags);
}

NMI_INVOKE( CharacterWrapper, one_hit, "(vict): нанести vict один удар оружием" )
{
    checkTarget();
    Character *victim = args2character(args);
    ::one_hit(target, victim);
    return Register();
}

NMI_INVOKE( CharacterWrapper, skill_one_hit, "(vict,skillName,damDivisor[,thacCoeff[,counter[,miss]]]): нанести vict удар оружием по формуле умения skillName. Урон умножается на (уровень_умения/damDivisor + 1), thacCoeff -- бонус к попаданию при 100% умения (вычитает thacCoeff*эффективность/100 из THAC0), counter добавляет бонус контратаки, miss печатает промах вместо удара" )
{
    checkTarget( );
    Character *victim = argnum2character(args, 1);
    Skill *skill = argnum2skill(args, 2);
    int damDivisor = argnum2number(args, 3);
    int thacCoeff = args.size( ) > 3 ? argnum2number(args, 4) : 0;
    bool applyCounter = args.size( ) > 4 ? argnum2boolean(args, 5) : false;
    bool miss = args.size( ) > 5 ? argnum2boolean(args, 6) : false;

    ::skill_one_hit_nocatch(target, victim, skill, damDivisor, thacCoeff, applyCounter, miss);
    return Register();
}

NMI_INVOKE( CharacterWrapper, saves_spell, "(caster,level,dam_type[,dam_flag[,verbose]]): спас-бросок против типа повреждения (.tables.damage_table) с флагом повреждения (.tables.damage_flags)")
{
    checkTarget();
    Character *caster = argnum2character(args, 1);
    int level = argnum2number(args, 2);
    int dam_type = argnum2flag(args, 3, damage_table);
    int dam_flag = DAMF_OTHER;
    if (args.size() > 3)
        dam_flag = argnum2flag(args, 4, damage_flags);

    bool verbose = true;
    if (args.size() > 4)
        verbose = argnum2boolean(args, 5);
        

    return Register(saves_spell(level, target, dam_type, caster, dam_flag, verbose));
}

NMI_INVOKE(CharacterWrapper, quaff, "(obj): получить эффекты от пилюли или зелья")
{
    checkTarget();
    ::Object *item = argnum2item(args, 1);

    if (item->item_type != ITEM_POTION && item->item_type != ITEM_PILL)
        throw Scripting::Exception("Object is not a pill or a potion");

    spell_by_item(target, item);
    return Register();
}

NMI_INVOKE( CharacterWrapper, spell, "(skillName,level[,vict|argument[,spellbane[,verbose]]]): скастовать заклинания на всю комнату, на vict или с аргументом")
{
    checkTarget( );

    Skill *skill = argnum2skill(args, 1);
    int level = argnum2number(args, 2);
    if (args.size() == 2) {
        // Room spell.
        spell( skill->getIndex( ), level, target, target->in_room );
        return Register();
    }

    Register arg3 = argnum(args, 3);
    if (arg3.type == Register::STRING) {
        // String argument spell.
        const char *arg = arg3.toString().c_str();
        spell( skill->getIndex( ), level, target, const_cast<char *>(arg) );
        return Register();
    } 

    // Character target spell.    
    Character *victim = argnum2character(args, 3);
    if (!victim)
        throw Scripting::IllegalArgumentException( );

    // Figure out the flags. Without an explicit choice a Battlerager's aura
    // (any spellbane on a player) still gets its roll.
    int flags = 0;
    if (args.size() >= 4) {
        if (argnum2number(args, 4))
            SET_BIT(flags, FSPELL_BANE);
    }
    else if (victim != target && !victim->is_npc() && victim->isAffected(gsn_spellbane))
        SET_BIT(flags, FSPELL_BANE);
    if (args.size() >= 5 && argnum2number(args, 5))
        SET_BIT(flags, FSPELL_VERBOSE);
 
    ::spell( skill->getIndex( ), level, target, victim, flags );
    return Register();
}

NMI_INVOKE( CharacterWrapper, multi_hit, "(vict): нанести один раунд повреждений жертве" )
{
    checkTarget( );
    ::multi_hit( target, arg2character( get_unique_arg( args ) ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, raw_kill, "([flags[,killer[,label[,damtype]]]]): убить. флаги из таблицы .tables.death_flags" )
{
    RegisterList::const_iterator i;
    Character *killer = NULL;
    DLString label;
    int damtype = -1; 
    bitstring_t flags = 0;

    checkTarget();
    
    if (args.size() > 0)
        flags = argnum2flag(args, 1, death_flags);
    flags = std::max(0LL, flags);
    if (args.size() > 1)
        killer = argnum2character(args, 2);
    if (args.size() > 2)
        label = argnum2string(args, 3);
    if (args.size() > 3)
        damtype = argnum2flag(args, 4, damage_table);
    
    raw_kill( target, flags, killer, label, damtype );
    throw VictimDeathException();
}

NMI_INVOKE( CharacterWrapper, affectAdd, "(.Affect): повесить новый аффект" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    affect_to_char( target, &(aw->getTarget()) );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectJoin, "(.Affect): повесить новый аффект или усилить существующий" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    affect_join( target, &(aw->getTarget()) );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectBitStrip, "(where,bit): снять все аффекты, устанавливающие в поле where (.tables.affwhere_flags) значение bit")
{
    int bits;
    
    checkTarget( );

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    // FIXME: change affectBitStrip in existing codesources.
    bits = args.back( ).toNumber( );
    affect_bit_strip( target, &affect_flags, bits );
    return Register( ); 
}

NMI_INVOKE( CharacterWrapper, isAffected, "(skillName): находится ли под воздействием аффекта с именем skillName" )
{
    Skill *skill;
    
    checkTarget( );

    if (args.size( ) != 1)
        throw Scripting::NotEnoughArgumentsException( );

    skill = skillManager->findExisting( args.front( ).toString( ) );

    if (skill)
        return target->isAffected( skill->getIndex( ) );
    else
        return false;
}

NMI_INVOKE( CharacterWrapper, affectStrip, "(skillName[,verbose]): снять все аффекты с именем skillName, показав сообщение о спадании (verbose)" )
{
    checkTarget( );
    Skill *skill = argnum2skill(args, 1);
    bool verbose = args.size() > 1 ? argnum2boolean(args, 2) : false;
        
    affect_strip( target, skill->getIndex( ), verbose );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectPermanent, "(skillName[,obj]): сделать все уже висящие аффекты этого типа постоянными (duration -2). Для вещей, дающих аффект при надевании: cast + affectPermanent(name, obj) -- аффект держится пока вещь надета и снимается движком автоматически при снятии/распаде/разрушении вещи, onRemove не нужен" )
{
    checkTarget( );
    Skill *skill = argnum2skill(args, 1);
    int sn = skill->getIndex( );
    ::Object *src = args.size( ) > 1 ? argnum2item(args, 2) : 0;

    for (auto &paf: target->affected)
        if (paf->type == sn) {
            paf->duration = -2;
            if (src)
                paf->sources.add( src );
        }
    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectReplace, "(.Affect): удалить все аффекты этого типа и повесить новый" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );        
    affect_strip(target, aw->getTarget().type);
    affect_to_char( target, &(aw->getTarget()) );
    return Register( );
}


NMI_INVOKE( CharacterWrapper, affectRemoveAll, "(): снять все аффекты" )
{
    checkTarget();

    for (auto &paf: target->affected.clone())
        affect_remove( target, paf );

    return Register();
}

NMI_INVOKE( CharacterWrapper, isVulnerable, "(damtype, damflag): есть ли уязвимость к типу повреждений из .tables.damage_table с флагом повреждений из .tables.damage_flags" )
{
    checkTarget();
    int damtype = argnum2flag(args, 1, damage_table);
    int damflag = argnum2flag(args, 2, damage_flags);
    return immune_check(target, damtype, damflag) == RESIST_VULNERABLE;
}

NMI_INVOKE( CharacterWrapper, isImmune, "(damtype, damflag): есть ли иммунитет к типу повреждений из .tables.damage_table с флагом повреждений из .tables.damage_flags" )
{
    checkTarget();
    int damtype = argnum2flag(args, 1, damage_table);
    int damflag = argnum2flag(args, 2, damage_flags);
    return immune_check(target, damtype, damflag) == RESIST_IMMUNE;
}

NMI_INVOKE( CharacterWrapper, stop_fighting, "(): прекратить битву" )
{
    checkTarget( );
    stop_fighting(target, get_unique_arg( args ).toBoolean( ));
    return Register( );
}

NMI_INVOKE( CharacterWrapper, move_char, "(door[,movetype]): переместить персонажа в дверь door, с типом движения movetype('running','crawl'). Вернет true если переместили.")
{
    int door, rc;
    DLString movetypeName;
    
    checkTarget( );

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    if (args.size( ) > 2)
        throw Scripting::TooManyArgumentsException( );
    
    door = args.front( ).toNumber( );
    if (door < 0 || door >= DIR_SOMEWHERE)
        return false;
    
    if (args.size( ) > 1)
        movetypeName = args.back( ).toString( );
    else 
        movetypeName = "normal";

    // "flee" is the scripted flee step: it may leave while still fighting.
    if (movetypeName == "flee")
        rc = ::move_char_flee( target, door );
    else
        rc = ::move_char( target, door, movetypeName.c_str( ) );

    return Register( rc == RC_MOVE_OK );
}

NMI_INVOKE( CharacterWrapper, dismount, "(): спешиться или сбросить всадника")
{
    checkTarget();
    target->dismount();
    return Register();
}

NMI_INVOKE( CharacterWrapper, addDarkShroud, "(): повесить темную ауру")
{
    Affect af;
    
    checkTarget( );

    af.type      = gsn_dark_shroud;
    af.level     = target->getRealLevel( );
    af.duration  = -1;
    affect_to_char( target, &af );

    return Register( );
}    

NMI_INVOKE( CharacterWrapper, isLawProtected, "(): охраняется ли моб законом" )
{
    NPCharacter *mob;
    
    checkTarget();
    CHK_PC
    mob = target->getNPC( );

    if (IS_SET(mob->pIndexData->area->area_flag, AREA_HOMETOWN))
        return true;

    return false;
}

NMI_INVOKE( CharacterWrapper, can_get_obj, "(obj): может ли поднять предмет obj с земли" )
{
    checkTarget( );

    ::Object *obj = arg2item( get_unique_arg( args ) );

    if (!obj->can_wear( ITEM_TAKE )) 
        return false;
    if (!obj->getOwner().empty())
        return false;
//     if (obj->behavior)
//         return false;
    if (!target->can_see( obj ))
        return false;
    if (obj->isAntiAligned( target ))
        return false;

    return true;
}

NMI_GET(CharacterWrapper, totems, "список всех тотемов, созданных персонажем" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (::Object *obj = object_list; obj != 0; obj = obj->next) {
        if (obj->item_type != ITEM_FURNITURE)
            continue;
        if (!IS_SET(obj->value2(), TOTEM))
            continue;
        if (!obj->hasOwner(target))
            continue;
        
        rc->push_back(WrapperManager::getThis( )->getWrapper(obj));
    }

    return wrap(rc);
}

NMI_INVOKE(CharacterWrapper, list_obj_world, "(arg): поиск по миру видимых персонажу предметов с уровнем не выше персонажа" )
{
    checkTarget();
    DLString arg = args2string(args);
    RegList::Pointer rc(NEW);

    for (::Object *obj = object_list; obj != 0; obj = obj->next) {
        // Name filter FIRST: it rejects almost every object with a cheap keyword
        // match, so the level check, visibility, and the two container-chain walks
        // (getCarrier/getRoom) below run only on the handful that match, not all
        // ~75k. Pure reorder of AND-ed conditions -- identical result set.
        if (!obj_has_name(obj, arg, target))
            continue;

        if (target->getRealLevel() < get_wear_level(target, obj))
            continue;

        if (!target->can_see(obj))
            continue;

        Character *carrier = obj->getCarrier();
        if (carrier && !target->can_see(carrier))
            continue;

        Room *location = obj->getRoom();
        if (location && !target->can_see(location))
            continue;

        rc->push_back(WrapperManager::getThis( )->getWrapper(obj));
    }

    return wrap(rc);
}

NMI_INVOKE(CharacterWrapper, get_obj_inventory_vnum, "(vnum): поиск по внуму предмета в инвентаре" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
        if (obj->wear_loc == wear_none && obj->pIndexData->vnum == vnum)
            return wrap( obj );

    return Register( );
}

NMI_INVOKE(CharacterWrapper, get_obj_carry_vnum, "(vnum): поиск по внуму предмета в инвентаре или экипировке" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            return wrap( obj );

    return Register( );
}

/*----------------------------------------------------------------------------
 * gearAdvice -- the sage's "service advice". Ranks the world's wearable gear
 * this character can ACTUALLY wear right now (native per-item-type wear level +
 * alignment) and scores it by a profile. Returns [pct, optimal[], best[]] as
 * prototype wrappers for Fenia to render: search stays in C++, output/tone in
 * Fenia. Weapons are scored by their dice scaled by the char's weapon skill
 * (Phase 1.5); AC and weapon flags (sharp/elementals) are not scored yet.
 *--------------------------------------------------------------------------*/

// Where the path to an item starts: Midgaard Market Square, the universal hub.
#define GA_START_ROOM 3014

// Sentinel slotFilter for a "service advice light" browse. Light items carry
// item_type LIGHT and NO wear_flags bit (their gearAdvice slot is 0), so a wear-bit
// mask cannot select them. Fenia's .tmp.advice.LIGHT_SLOT returns this same value.
#define GA_SLOT_LIGHT (1 << 30)

// How the sage tells you to get an item. GA_INPACK: the char already carries it
// unworn -- the sage says "put it on" instead of pointing at a route.
enum { GA_KILL = 0, GA_BUY = 1, GA_PICKUP = 2, GA_QUEST = 3, GA_UNKNOWN = 4, GA_INPACK = 5, GA_REQUEST = 6 };

// Mobs that make a room "guarded": aggressive, or any kind of assist.
#define GA_ASSIST_MASK (ASSIST_ALL|ASSIST_ALIGN|ASSIST_RACE|ASSIST_PLAYERS|ASSIST_GUARD|ASSIST_VNUM)

// The easiest way to obtain one object vnum.
// tier = the toughest mob tier you must fight on the route (1 legend .. 10), the
// worst tier when there is no fight (a shop, an unguarded floor).
struct GAAcq {
    int method, aux, room, cost, guard, tier;   // aux = holder / shop / quest vnum
    GAAcq( ) : method(GA_UNKNOWN), aux(0), room(0), cost(0), guard(0), tier(MobTiers::TIER_WORST) { }
};

// A guard's tier as extra levels: an elite fights like a normal mob ten levels up, a
// champion twenty, a boss or a legend forty. Ranks routes and sets the band, so the
// sage never calls a boss fight easy because the boss is low level (mob reform tiers:
// elite = a group or better gear, champion+ = near the edge of a solo fight).
static int ga_tierLevels( int tier )
{
    if (tier <= 2)
        return 40;
    if (tier <= 3)
        return 20;
    if (tier <= 5)
        return 10;
    return 0;
}

static int ga_effGuard( int guard, int tier )
{
    return guard + ga_tierLevels( tier );
}

// Keep the easiest source per object vnum by guard level plus tier (buy beats a kill,
// an easy room beats a hard one, a normal mob beats an elite of the same level). Ties
// keep the first seen.
static void ga_record( std::map<int,GAAcq> &acq, int vnum, int method, int aux, int room, int cost, int guard,
                       int tier = MobTiers::TIER_WORST )
{
    std::map<int,GAAcq>::iterator it = acq.find( vnum );
    if (it != acq.end( ) && ga_effGuard( it->second.guard, it->second.tier ) <= ga_effGuard( guard, tier ))
        return;
    GAAcq a; a.method = method; a.aux = aux; a.room = room; a.cost = cost; a.guard = guard; a.tier = tier;
    acq[vnum] = a;
}

// Profile weights: fight_core/itemmodel.h, one table for the sage and the generators.
typedef ItemWeights GAWeights;

struct GACand {
    obj_index_data *pObj;
    ::Object *inst = 0; // a rolled item the char carries: scored and shown as itself
    int    slot;      // wear_flags without ITEM_TAKE (0 = light)
    double score;
    double obtain;    // 0..1 obtainability
    double value;     // gap * obtain (filled for the 'optimal' ranking)
    GAAcq  acq;       // how the sage says to get it
    bool   present;   // the acquisition route is actionable now. Only a limited item
                      // (limit>0) can be false: unlimited gear always repops. false ->
                      // render says whereabouts unknown instead of a stale route.
};

// Boss cap: gear whose easiest route is killing or looting more than 10 levels
// above the char. The sage never advises it, so it must not raise the percentile's
// ceiling either -- otherwise a kit with nothing left to chase still reads below 100%.
static bool ga_overCap( const GAAcq &acq, int chLevel )
{
    return (acq.method == GA_KILL || acq.method == GA_PICKUP) && acq.guard > chLevel + 10;
}

// Weapon flags of a candidate: a rolled weapon keeps them on the instance.
static int ga_candValue4( const GACand &c )
{
    return c.inst ? c.inst->value4( ) : c.pObj->value[4];
}

// The body can fight with an off-hand weapon at all: the second-weapon skill is usable,
// the char has the off-hand wear location (some shapeshifts lose it), and has hands and
// both wrists -- second_weapon_hit (fight.cpp) skips the off-hand swing without them.
static bool ga_dualBody( Character *ch )
{
    Skill *sk = skillManager->findExisting( "second weapon" );
    if (sk == 0 || !sk->usable( ch, false ))
        return false;
    const char *locs[] = { "second_wield", "hands", "wrist_l", "wrist_r" };
    for (int i = 0; i < 4; i++) {
        Wearlocation *loc = wearlocationManager->findExisting( locs[i] );
        if (loc == 0 || !ch->getWearloc( ).isSet( loc ))
            return false;
    }
    return true;
}

// A two-handed weapon blocks the other hand unless the race is SIZE_HUGE: giants may wield
// a two-hander beside an off-hand weapon, or put one IN the off-hand. Mirrors
// SecondWieldWearloc::canWear and second_weapon_hit.
static bool ga_twoHandBlocks( Character *ch, bool twoHanded )
{
    return twoHanded && ch->getRace( )->getSize( ) < SIZE_HUGE;
}

// Can the char put a weapon in the off-hand right now? Mirrors SecondWieldWearloc's
// gate: ga_dualBody, no shield or held item in the left hand, and the primary weapon
// not two-handed (a giant excepted). Tests ACTUAL wear locations, not prototype flags --
// an arrow stuck in the char or a sheathed weapon carries the wield flag but holds no
// hand. A shield build keeps a single weapon position here; the shield-or-second-weapon
// verdict in gearAdvice weighs that choice separately. (Conservative gap: a sheathed
// weapon reads the off-hand as free -- only ever under-reports capacity.)
static bool ga_canDualWield( Character *ch )
{
    if (!ga_dualBody( ch ))
        return false;
    Wearlocation *shieldLoc = wearlocationManager->findExisting( "shield" );
    Wearlocation *holdLoc   = wearlocationManager->findExisting( "hold" );
    if ((shieldLoc != 0 && shieldLoc->find( ch ) != 0)
        || (holdLoc != 0 && holdLoc->find( ch ) != 0))
        return false;
    Wearlocation *wieldLoc = wearlocationManager->findExisting( "wield" );
    ::Object *primary = wieldLoc ? wieldLoc->find( ch ) : 0;
    if (primary != 0 && ga_twoHandBlocks( ch, IS_WEAPON_STAT( primary, WEAPON_TWO_HANDS ) ))
        return false;
    return true;
}

// Cap-aware stat value: only the points that actually land BELOW the character's
// stat cap are worth anything. base = the stat WITHOUT the item being scored.
static double ga_statgain( int base, int delta, int cap )
{
    return item_fit_stat( base, delta, cap );
}

// ga_score's stat[] weight order (str,int,wis,dex,con,cha) -> STAT_ index.
static const int ga_statMap[6] = { STAT_STR, STAT_INT, STAT_WIS, STAT_DEX, STAT_CON, STAT_CHA };

// A prototype carrying any Fenia trigger (onUse/onGet/onExamine/...) is almost
// always special, high-value gear, so ga_score gives it a flat boost.
static bool ga_hasFeniaTriggers( obj_index_data *pObj )
{
    if (pObj->wrapper == 0)
        return false;
    WrapperBase *w = get_wrapper( pObj->wrapper );
    if (w == 0)
        return false;
    StringSet triggers, misc;
    w->collectTriggers( triggers, misc );
    return !triggers.empty( );
}

// An item carrying the 'grantskills' behavior teaches its wearer skills -- the
// same "special, high-value" class as Fenia-triggered gear. The grantskills
// migration annulled these items' onEquip/onRemove prototype triggers, so
// ga_hasFeniaTriggers no longer sees them; credit the behavior here so a
// skill-teaching item keeps its boost.
static bool ga_grantsSkills( obj_index_data *pObj )
{
    Behavior *b = behaviorManager->findExisting( "grantskills" );
    return b != 0 && pObj->behaviors.isSet( b->getIndex( ) );
}

// Defined in feniaskillaction.cpp (skills_impl, linked into feniaroot): expected
// damage of a named spell's <tier> at the given level; 0 when the name is not a
// spell or the spell declares no damage tier.
double spell_proc_tier_value( const DLString &spellName, int level );

// Worth of what an item does in combat each round, in expected-damage units,
// mapped to gear-score currency by _global and scaled by item level. Two kinds,
// summed:
//  <props>combatcast</props> [{spell,chance,count}] -- offensive/heal/buff spells
//    cast in combat. A clean damage nuke derives its value from its <tier>
//    (spell_proc_tier_value, discounted by _save_factor); spells whose worth does
//    not follow their tier carry an explicit override in spell_combat_value.json.
//  <props>combathits</props> [{one_hit,multi_hit,chance}] -- EXTRA melee attacks
//    the item's onFight fires (ch.one_hit / ch.multi_hit). For a WIELDABLE weapon each
//    extra swing is worth one base swing of it (weaponWeight * dice*skill, passed in as
//    weaponSwing) -- already score units, so NOT _global- or level-scaled. A non-weapon
//    grant (a belt, a ring) is scored on the wearer's current weapon; with no usable
//    weapon it falls back to _hit_value (a reference hit at the ref level, _global- and
//    level-scaled like a spell). A multi_hit is a whole extra round, _round_attacks swings.
// Returns 0 for an item that declares neither -- ga_score then keeps the flat +50.
// COMBAT_PROC_SCORING.md.
static double ga_procScore( obj_index_data *pObj, double weaponSwing = -1.0, ::Object *inst = 0,
                            int modelLevel = -1, bool caster = false )
{
    // A random item carries its procs on the instance (armor generator); they win
    // over the prototype's, exactly as ocombatcast_fight fires them.
    const Json::Value &castSrc = (inst != 0 && inst->props.isMember( "combatcast" ))
                                    ? inst->props : pObj->props;
    // isMember guard FIRST on every prop read: pObj is non-const, so
    // pObj->props["x"] would INSERT a null member into the prototype on every
    // scored item (jsoncpp non-const operator[]), polluting props world-wide and
    // drifting to disk on the next autosave. Read only after isMember confirms it.
    // weaponSwing = the SCORE value of one full swing of THIS weapon (weaponWeight*eff),
    // supplied by the caller for a wieldable weapon; each extra combathits swing is valued as
    // one base swing (a heuristic, see the combathits loop), already in final score units and
    // NOT re-scaled by _global. -1 = no known usable weapon (a non-weapon grant, or an
    // unusable weapon) -> combathits fall back to the flat level-referenced hit. Spell casts
    // and the flat fallback are damage -> _global + level-scaled; a real swing is neither.
    int ref = (int)spell_combat_level_ref( );
    double rawCast = 0, rawFlatHits = 0, swingHits = 0;
    // Model (modelLevel >= 0): each cast is priced by the combat-effect model at the
    // item level, in final points (item_proc_points, decision 9), not via _global.
    double modelCast = 0;

    // Spells cast in combat.
    if (castSrc.isMember( "combatcast" )) {
        const Json::Value &casts = castSrc["combatcast"];
        if (casts.isArray( )) {
            for (auto i = casts.begin( ); i != casts.end( ); ++i) {
                const Json::Value &c = *i;
                DLString spellName = c["spell"].asString( );
                if (modelLevel >= 0) {
                    modelCast += item_proc_points( spellName, c["chance"].asDouble( ),
                                                   c.isMember( "count" ) ? c["count"].asDouble( ) : 1.0,
                                                   modelLevel, caster );
                    continue;
                }

                // Explicit override wins (spells whose worth does not follow their
                // tier); otherwise derive from the spell's damage tier at the
                // reference level, discounted for an average save. Neither -> skip
                // (the flat +50 fallback in ga_score still covers "it triggers").
                double v = spell_combat_value( spellName );
                if (v <= 0)
                    v = spell_proc_tier_value( spellName, ref ) * spell_combat_save_factor( );
                if (v <= 0)
                    continue;

                double chance = c["chance"].asDouble( );
                double count  = c.isMember( "count" ) ? c["count"].asDouble( ) : 1.0;
                if (count <= 0)
                    count = 1.0;
                if (count > 10)          // match the firing cap (ocombatcast_fight)
                    count = 10.0;
                rawCast += v * (chance / 100.0) * count;
            }
        }
    }

    // Extra melee attacks fired from the item's onFight.
    if (pObj->props.isMember( "combathits" )) {
        const Json::Value &hits = pObj->props["combathits"];
        if (hits.isArray( )) {
            for (auto i = hits.begin( ); i != hits.end( ); ++i) {
                const Json::Value &h = *i;
                double chance = h["chance"].asDouble( );
                if (chance <= 0)
                    continue;
                double oneHit   = h.isMember( "one_hit" )   ? h["one_hit"].asDouble( )   : 0.0;
                double multiHit = h.isMember( "multi_hit" ) ? h["multi_hit"].asDouble( ) : 0.0;
                double attacks  = oneHit + multiHit * spell_combat_round_attacks( );
                if (attacks <= 0)
                    continue;
                // Each extra swing is a real hit with the char's wielded weapon, valued as
                // one base swing of it -- weaponSwing (weaponWeight*eff), already a final
                // score value, skill-aware (exotic on a level+INT char swings for far more
                // than the flat reference). Heuristic, not exact: base dice are per-hit while
                // a proc is per-round, but each proc swing also carries the char's damroll
                // that the base term omits, so charging a full base swing roughly cancels
                // out. A non-weapon grant has no known swing -> flat level-referenced hit.
                if (weaponSwing >= 0)
                    swingHits += attacks * weaponSwing * (chance / 100.0);
                else
                    rawFlatHits += attacks * spell_combat_hit_value( ) * (chance / 100.0);
            }
        }
    }

    if (rawCast <= 0 && rawFlatHits <= 0 && swingHits <= 0 && modelCast <= 0)
        return 0;

    // rawCast and rawFlatHits are expected DAMAGE -> convert to score currency with
    // spell_combat_global (calibrated on fireball's true mean, ~2.68 on live) and level-scale
    // them. swingHits is ALREADY in score units (weaponWeight*eff) and already reflects
    // level+skill+dice, so it is neither level-scaled nor re-converted by _global -- doing so
    // would double-convert it (the bug the first cut shipped: everything x2.68).
    double levelScale = pObj->level / spell_combat_level_ref( );
    return spell_combat_global( ) * (rawCast + rawFlatHits) * levelScale + swingHits + modelCast;
}

// Clerics learn "compound" (lvl 37): it weights any weapon-class weapon into a
// mace they can wield, so a non-limited weapon a cleric can't natively use is
// still a real pick -- scored at their MACE skill, not the zero native skill.
// compound refuses maces (already one), arrows/daggers/bows, limited items,
// noenchant, and katana/spell weapons (dreamland_world/generic-skills/compound.xml,
// spell/compound/runObj). The owner check is per-instance so the prototype scorer
// can't see it; every other gate it can. Trello #2854.
// Can this level-37+ cleric compound this weapon into a mace (so it scores at the
// mace skill rather than the unskilled floor)? itemType/limit/extraFlags plus the
// weapon class and its type-2 flags are supplied by the caller so a prototype and a
// live instance can each pass their own -- a random weapon's real class and flags
// live on the instance; its prototype is the class-exotic, no-flag stub (vnum 104),
// which would wrongly pass the gate for every worn random weapon.
static bool ga_clericCanCompoundCore( Character *target, int itemType, int limit,
                                      int extraFlags, int wclass, int wflags )
{
    if (itemType != ITEM_WEAPON)
        return false;
    if (target->getModifyLevel( ) < 37)
        return false;
    if (target->getProfession( )->getName( ) != "cleric")
        return false;
    if (limit > 0)
        return false;
    if (IS_SET( extraFlags, ITEM_NOENCHANT ))
        return false;
    if (IS_SET( wflags, WEAPON_KATANA ) || IS_SET( wflags, WEAPON_SPELL ))
        return false;
    switch (wclass) {
    case WEAPON_MACE:      // already a mace: usable natively, no compound needed
    case WEAPON_DAGGER:
    case WEAPON_BOW:
    case WEAPON_ARROW:
        return false;
    }
    return true;
}

// Candidate prototype: class and flags read from the proto.
static bool ga_clericCanCompound( Character *target, obj_index_data *pObj )
{
    return ga_clericCanCompoundCore( target, pObj->item_type, pObj->limit,
                                     pObj->extra_flags, pObj->value[0], pObj->value[4] );
}

// Live worn item: weapon class from get_weapon_class (resolves instance-or-proto)
// and flags from the instance value, so a rolled random weapon is judged on what it
// actually is, not on the stub prototype it was built from.
static bool ga_clericCanCompound( Character *target, ::Object *o )
{
    // The compound prayer (spell/compound/runObj) also refuses a personalized/owned
    // weapon; an owner exists only on the instance, so it is checked only here.
    if (!o->getOwner( ).empty( ))
        return false;
    return ga_clericCanCompoundCore( target, o->item_type, o->pIndexData->limit,
                                     o->extra_flags, get_weapon_class( o ), o->value4( ) );
}

// Can the character produce this buff on themselves -- i.e. does their class get
// the spell/skill at their level? Then an item granting it is worth only
// convenience, not the full effect. Keyed on class availability (level + class),
// NOT on practiced percent: a level-35 paladin can cast sanctuary (a level-27
// prayer) whether or not she has practiced it yet, so the item is redundant for
// her; a level-15 paladin cannot reach it, so the grant keeps its full value.
// Unknown name (or no target) -> false, i.e. keep the full value: a wrong name
// never over-discounts.
static bool ga_canSelfCast( Character *target, const char *name )
{
    return item_fit_self_cast( target, name );
}

// Value of an affect_flags bitvector (sanctuary/haste/stealth; negatives are
// cursed-gear penalties). Profile-split where it matters. Values + rationale:
// GEAR_AFFECT_VALUES.md. Owner overrides folded in (imp_invis/camouflage/fade=100,
// stun=-100). Concentration is a Fenia onEquip skill, not a flag -- deferred to 3c.
// A positive self-buff the char's class can already cast scores at 10% of full
// (no mana/slot cost, undispellable, works when silenced -- but not a new
// capability). Curses and gear-only bits (no matching spell) never discount.
static double ga_affectFlagValue( bitstring_t b, bool caster, Character *target, bitstring_t heldFlags )
{
    return item_flag_points( b, caster, target, heldFlags );
}

static double ga_resValue( bitstring_t b, int kind )
{
    return item_res_points( b, kind );
}

// One affect's profile-weighted worth: flat pools (hp/mana/regen/dr/hr/saves) fold
// into s; the six primary stats accumulate into statDelta[] for the caller to
// resolve cap-aware; ac/slevel/level/move/beats and any flag/res bits fold in here
// too. Shared by ga_score (item affects) and ga_setValue (set bonus).
// Remort count -> how far past the practice adept (75) gear can still push a skill
// group toward 100. 0 for an NPC or a null target.
static int ga_remorts( Character *target )
{
    return (target != 0 && target->getPC( ) != 0)
        ? (int)target->getPC( )->getRemorts( ).size( ) : 0;
}

// Does the char actually have this skill group -- i.e. know at least one skill in
// it? A +level to a group the char has no skills in does nothing.
static bool ga_hasGroup( Character *target, int gi )
{
    if (target == 0)
        return false;
    for (int i = 0; i < skillManager->size( ); i++) {
        Skill *sk = skillManager->find( i );
        if (sk == 0)
            continue;
        for (int g: sk->getGroups( ).toArray( ))
            if (g == gi) {
                if (target->getSkill( i ) > 0)   // a KNOWN skill in this group -> char has it
                    return true;
                break;   // skill is in gi but unknown; no need to scan its other groups
            }
    }
    return false;
}

// How much slevel gear is worth to THIS char: full for a real caster, ~0 for a
// warrior with no spells. Ramp on the count of spells the char actually knows
// (getSpell() != 0 marks a skill as a spell); saturates at 10 known spells.
static double ga_spellFactor( Character *target )
{
    if (target == 0)
        return 1.0;
    int spells = 0;
    for (int i = 0; i < skillManager->size( ); i++) {
        Skill *sk = skillManager->find( i );
        if (sk == 0 || !sk->getSpell( ))
            continue;
        // Only CASTED spells read mod_level_spell (skill_utils.cpp skill_level_bonus).
        if (sk->getSpell( )->isCasted( ) && target->getSkill( i ) > 0)
            spells++;
    }
    double f = spells / 10.0;
    return f > 1.0 ? 1.0 : f;
}

// Score breakdown for the sage's "what changes" line (.itemScoreTerms). While
// ga_terms is set, the scorer also files every scored piece under a key with its
// points and raw amount, so the render can rank a candidate's terms against the worn
// item's by score impact. Set only around one item's final ga_score call.
struct GATerm {
    double pts = 0;
    int raw = 0;
};
typedef std::map<DLString, GATerm> GATerms;
static GATerms *ga_terms = 0;

static void ga_note( const DLString &key, double pts, int raw )
{
    if (ga_terms == 0 || pts == 0)
        return;
    GATerm &t = (*ga_terms)[key];
    t.pts += pts;
    t.raw += raw;
}

// Term key of an affect's location part: the apply name, the four saves folded into
// one, and a skill/group global appended ("learned:illusion", "level:combat").
static DLString ga_termKey( const Affect &af )
{
    switch (af.location) {
    case APPLY_SAVES:
    case APPLY_SAVING_ROD:
    case APPLY_SAVING_PETRI:
    case APPLY_SAVING_BREATH:
    case APPLY_SAVING_SPELL:
        return "saves";
    }
    DLString key = apply_flags.name( af.location );
    if (!af.global.empty( ))
        key = key + ":" + af.global.toString( ',' );
    return key;
}

static void ga_accumAffect( const Affect &af, const GAWeights &w, double &s, int statDelta[6],
                            Character *target, bool worn, int itemLevel = -1 )
{
    int m = af.modifier;
    double s0 = s;
    switch (af.location) {
    case APPLY_MANA_GAIN:
        // Diminishing worth: mana_gain is a PERCENT multiplier on base regen
        // (update_params.cpp), so once regen already outpaces in-combat spend the
        // next point buys nothing. Discount a positive modifier hyperbolically
        // against what the char already owns (target->mana_gain). Curses stay full
        // price; melee's 0.05 is a token, only casters get the curve.
        if (m > 0 && w.caster && target != 0) {
            double g0 = target->mana_gain < 0 ? 0.0 : target->mana_gain;
            s += w.manaGain * m * 100.0 / (100.0 + g0 + m / 2.0);
        }
        else
            s += item_apply_points( APPLY_MANA_GAIN, m, w );
        break;
    case APPLY_STR: statDelta[0] += m; break;
    case APPLY_INT: statDelta[1] += m; break;
    case APPLY_WIS: statDelta[2] += m; break;
    case APPLY_DEX: statDelta[3] += m; break;
    case APPLY_CON: statDelta[4] += m; break;
    case APPLY_CHA: statDelta[5] += m; break;
    // Plain applies: the shared base price (fight_core/itemmodel.cpp).
    case APPLY_HIT:
    case APPLY_MANA:
    case APPLY_HEAL_GAIN:
    case APPLY_DAMROLL:
    case APPLY_HITROLL:
    case APPLY_SAVES:
    case APPLY_SAVING_ROD:
    case APPLY_SAVING_PETRI:
    case APPLY_SAVING_BREATH:
    case APPLY_SAVING_SPELL:
    case APPLY_AC:            // w.ac is level-scaled
    case APPLY_SPELL_LEVEL:   // ~0 with no spells (w.spellFactor)
    case APPLY_MOVE:
    case APPLY_BEATS:
        s += item_apply_points( af.location, m, w );
        break;
    // +N levels: scope-split like APPLY_LEARNED -- one named skill < a group < all --
    // and capability-gated: +level to a skill the char can't use or a group it doesn't
    // have is worthless (0); only the unscoped all-skills bonus is unconditional.
    case APPLY_LEVEL: {
        bool isSkill = !af.global.empty( ) && af.global.getRegistry( ) == skillManager;
        bool isGroup = !af.global.empty( ) && af.global.getRegistry( ) == skillGroupManager;
        if (isSkill) {
            if (target != 0)
                for (int sn: af.global.toArray( ))
                    if (target->getSkill( sn ) > 0)
                        s += w.skillLevelSkill * m;
        }
        else if (isGroup) {
            for (int gi: af.global.toArray( ))
                if (ga_hasGroup( target, gi ))
                    s += w.skillLevel * m;
        }
        else
            s += item_apply_points( APPLY_LEVEL, m, w );
        break;
    }
    // +N% skill knowledge, cap-aware. APPLY_NONE with a skill/group global reaches the
    // same mod_skills path (loadsave/affects.cpp), so alias it in -- 9 live items used
    // it and scored 0. A point above a skill's effective 100 (getEffective clamps) or
    // above the 25+3*remort window gear can open past the practice adept (75) is worth
    // nothing, mirroring the cap-aware primary stats. Skill scope reads the char's real
    // % per named skill (0 for an unknown/off-class skill); group/all use the window.
    case APPLY_NONE:
    case APPLY_LEARNED: {
        bool isSkill = !af.global.empty( ) && af.global.getRegistry( ) == skillManager;
        bool isGroup = !af.global.empty( ) && af.global.getRegistry( ) == skillGroupManager;
        if (isSkill) {
            if (target != 0)
                for (int sn: af.global.toArray( )) {
                    int eff = target->getSkill( sn );
                    int room = 100 - eff;
                    if (eff > 0 && room > 0)
                        s += w.learnSkill * (m < room ? m : room);
                }
        }
        else if (isGroup) {
            // +N% to a group lands only on the group's skills the char knows, each up to
            // its own 100. Price it like the skill scope over exactly those skills, capped
            // at the old flat group price: a paladin whose only illusion skill is attract
            // other at 86% gains 14 real points, not a whole school (Dementia's Incubus ring).
            // A curse (m < 0) keeps the flat price, like a learned-skill curse.
            if (target != 0) {
                std::set<int> groups;
                for (int gi: af.global.toArray( ))
                    groups.insert( gi );
                double perSkill = 0;
                bool owns = false;
                for (int i = 0; i < skillManager->size( ); i++) {
                    Skill *sk = skillManager->find( i );
                    if (sk == 0)
                        continue;
                    bool inGroup = false;
                    for (int g: sk->getGroups( ).toArray( ))
                        if (groups.count( g )) { inGroup = true; break; }
                    int eff = inGroup ? target->getSkill( i ) : 0;
                    if (eff <= 0)
                        continue;
                    owns = true;
                    int room = 100 - eff;
                    if (room > 0)
                        perSkill += w.learnSkill * (m < room ? m : room);
                }
                if (owns) {
                    int cap = 25 + 3 * ga_remorts( target );
                    double flat = w.learnGroup * (m < cap ? m : cap);
                    s += m > 0 && perSkill < flat ? perSkill : flat;
                }
            }
        }
        else if (af.location == APPLY_LEARNED) {   // all-skills scope (empty global)
            int cap = 25 + 3 * ga_remorts( target );
            s += w.learnAll * (m < cap ? m : cap);
        }
        break;
    }
    // APPLY_AGE: cosmetic in score, no combat effect -- deliberately unscored.
    }
    ga_note( ga_termKey( af ), s - s0, m );

    // Flag affects: sanctuary/haste/stealth (affect_flags) and resist/immune/vuln
    // (res/imm/vuln_flags) live in the affect's bitvector, not location/modifier.
    // `affect_modify` ADDS these bits regardless of the modifier sign -- removal on a
    // negative modifier is implemented only for part_flags (loadsave/affects.cpp),
    // which we score 0 anyway. So score the flag as GRANTED, never negated (a display
    // that says "отнимает" on such an item is lying about what the engine does).
    const FlagTable *ft = af.bitvector.getTable( );
    bitstring_t bits = af.bitvector;
    if (ft != 0 && bits != 0) {
        double f0 = s;
        // Redundancy discount applies to a CANDIDATE only (worn ? 0): never dock the
        // worn baseline, so a gear delta can only ever shrink, never inflate. The
        // percentile's flag-once totals opt in for worn items via heldOnWorn.
        bool held = !worn || w.heldOnWorn;
        int kind = ft == &res_flags ? 0 : ft == &imm_flags ? 1 : ft == &vuln_flags ? 2 : -1;
        if (ft == &affect_flags)
            s += item_flag_points( bits, w.caster, target, held ? w.heldFlags : 0, itemLevel );
        else if (ft == &detect_flags)
            s += item_detect_points( bits, w.caster, target );
        // A resist the char already owns outside this slot adds nothing
        // (decision 6). Same gate as heldFlags: candidates, or worn with heldOnWorn.
        else if (kind >= 0 && held && target != 0)
            s += item_res_points_fit( bits, kind, w.heldImm, w.heldRes );
        else if (kind >= 0)
            s += ga_resValue( bits, kind );
        // Keyed by table and bits, so the render can name them via .tables.X.messages.
        const char *tab = ft == &affect_flags ? "affect_flags" : ft == &detect_flags ? "detect_flags"
                        : kind == 0 ? "res_flags" : kind == 1 ? "imm_flags" : kind == 2 ? "vuln_flags" : 0;
        if (tab != 0)
            ga_note( DLString( tab ) + ":" + DLString( (long long)bits ), s - f0, 0 );
    }
}

// Average landed weapon dice per hit at the char's real skill, the per-hit currency
// ga_scoreCore scores a weapon in (WeaponOneHit::damBase: dice * (20 + skill%)/100).
// skillOut (optional) gets the skill % used, for the damroll share OneHit::damApplyDamroll
// gives a hit (damroll * min(100, 20 + skill%)/100).
static double ga_weaponEff( Character *target, int weaponSn, int weaponAve, bool canCompound,
                            int *skillOut = 0 )
{
    // Base value (.itemPoints): no char, the dice count in full.
    if (target == 0) {
        if (skillOut != 0)
            *skillOut = 100;
        return weaponAve;
    }
    int skillPct = target->getSkill( weaponSn );
    // A cleric who can compound this weapon into a mace wields it at their
    // mace skill, so it scores like a real mace instead of collapsing to the
    // 20% unskilled floor. Trello #2854. Eligibility is resolved by the caller
    // (from the instance for a worn item, the proto for a candidate).
    if (canCompound) {
        Skill *mace = skillManager->findExisting( "mace" );
        if (mace != 0) {
            int macePct = target->getSkill( mace->getIndex( ) );
            if (macePct > skillPct)
                skillPct = macePct;
        }
    }
    if (skillOut != 0)
        *skillOut = skillPct;
    // Score the dice at the char's real skill in this weapon. available() alone is
    // the wrong gate: an EXOTIC weapon reports available()==false (it can never be
    // practiced) yet ExoticSkill::getLearned derives its skill from level+INT, so the
    // char swings it at up to 100%. So also fall through when getSkill() is already
    // positive. Exotic is the case that matters here: a normal off-class weapon skill
    // returns 0 once it is unusable (GenericSkill::getLearned), so it stays gated. A
    // weapon the char can neither train (available) nor already use (getSkill 0), e.g. a
    // warlock's mace, still scores 0 dice, so the sage never chases a weapon that would
    // sit at the unskilled floor. The item may still be worn for its stat affixes, which
    // the affect loop counted.
    Skill *wsk = skillManager->find( weaponSn );
    if (canCompound || wsk == 0 || wsk->available( target ) || skillPct > 0)
        return weaponAve * (20 + skillPct) / 100.0;
    return 0;
}

// Score a prototype for a profile. Flat pools (hp/mana/regen) and combat stats
// count in full; the six primary stats are cap-aware: a point at the cap adds 0.
// rawStat[k] = the char's uncapped stat (perm+mod); capStat[k] = its cap. worn =
// true when scoring an item the char already wears, so its own stat contribution
// is removed from the baseline (a stat held at cap by OTHER gear then scores 0).
// The core scorer: affects worth + stat gains + weapon dice + combat-proc / trigger
// / taught-skill boosts. Weapon output (skill number + dice-average) is passed in
// precomputed so a reset prototype and a live instance can each supply their own.
// Affects come in two lists: protoAff always, and instAff (the instance's own
// affects -- enchant deltas, a random weapon's rolled stats) when scoring a live
// item. Summing both mirrors Object::affectedValue, the engine's own view of an
// item's stats. Cleric-compound eligibility is resolved by the caller (from the
// instance for a worn item, the proto for a candidate) and arrives as canCompound.
// Everything else -- procs, Fenia triggers, taught skills -- is a property of the
// PROTOTYPE, read from pProto.
static double ga_scoreCore( Character *target, const GAWeights &w,
                            const AffectList &protoAff, const AffectList *instAff,
                            int itemType, int weaponSn, int weaponAve, bool canCompound,
                            obj_index_data *pProto,
                            const int rawStat[6], const int capStat[6], bool worn,
                            ::Object *inst = 0 )
{
    double s = 0;
    int statDelta[6] = { 0, 0, 0, 0, 0, 0 };
    int itemLevel = inst != 0 ? inst->level : (pProto != 0 ? pProto->level : -1);
    for (auto &paf: protoAff)
        ga_accumAffect( *paf, w, s, statDelta, target, worn, itemLevel );
    if (instAff != 0)
        for (auto &paf: *instAff)
            ga_accumAffect( *paf, w, s, statDelta, target, worn, itemLevel );
    for (int k = 0; k < 6; k++) {
        if (statDelta[k] == 0 || w.stat[k] == 0)
            continue;
        int base = rawStat[k] - (worn ? statDelta[k] : 0);
        double pts = w.stat[k] * ga_statgain( base, statDelta[k], capStat[k] );
        static const char *statKey[6] = { "str", "int", "wis", "dex", "con", "cha" };
        ga_note( statKey[k], pts, statDelta[k] );
        s += pts;
    }
    // Weapons: the affect loop above already counted a weapon's damroll/hitroll
    // and stat applies, but not its main worth -- the dice it swings. Per
    // WeaponOneHit::damBase the landed damage is dice * (20 + weaponSkill%)/100,
    // the same per-hit currency as damroll, so weight it with weaponWeight (split
    // from the damroll weight so a caster values weapon output below its damroll).
    // An unskilled weapon (a cleric holding an axe) collapses toward the 20% dice
    // floor and scores near nothing -- which is exactly why the sage will never
    // chase a weapon the character can't actually use.
    //
    // Dice only count for a weapon that can actually be swung -- one carrying the
    // WIELD flag (primary hand or a dual-wield off-hand, both wield-flagged). A
    // weapon that is HOLD-only (a throwing stone, a focus) is never swung as a
    // melee weapon, so its dice must not count: it competes for the hold slot
    // against a caster's stat focus and would otherwise out-rank it on raw dice
    // alone (a caster offered an 8d6 throwing stone over her +100hp/+100mana
    // scepter was the report). Its stat affixes still count via the affect loop.
    // Score value of one full swing of this weapon (weaponWeight * dice*skill), reused by
    // the proc scorer below so the item's own combathits credit each extra swing exactly
    // like a base swing. -1 = not a wieldable weapon -> ga_procScore keeps its flat fallback.
    double weaponSwing = -1.0;
    if (itemType == ITEM_WEAPON && pProto != 0
        && IS_SET( pProto->wear_flags, ITEM_WIELD )) {
        double eff = ga_weaponEff( target, weaponSn, weaponAve, canCompound );
        s += w.weaponWeight * eff;
        ga_note( "dice", w.weaponWeight * eff, weaponAve );
        // Only a weapon the char can actually swing (eff > 0) feeds its combathits proc; an
        // unusable weapon keeps weaponSwing -1 so its proc takes the flat fallback instead of
        // scoring 0 and dropping to the +50 Fenia-trigger bonus, which would over-rate it.
        if (eff > 0)
            weaponSwing = w.weaponWeight * eff;   // one swing's score; its combathits reuse it.
        // Weapon flags at the generator's price, x the alignment fit, only for a
        // weapon the char can swing (its flags fire on hits).
        if (eff > 0) {
            int wflags = inst != 0 ? inst->value4( ) : pProto->value[4];
            double pts = item_weapon_flag_points( wflags, itemLevel, w.caster, target );
            ga_note( "weapon_flags", pts, wflags );
            s += pts;
        }
    }
    else {
        // A non-weapon item's combathits (a belt, a ring granting extra attacks) fire the
        // char's currently-worn weapon, so score them on THAT swing, not a flat reference.
        weaponSwing = w.curWeaponSwing;
    }
    // Base armour class: an armour item's value[0..2] (pierce/bash/slash AC) is real
    // defence the affect loop never sees -- APPLY_AC scores enchant/spell deltas only,
    // so a plate's own plates counted for nothing and a stat-only cloth out-ranked it.
    // Credit the per-class average (value3/exotic dropped, mirroring the engine's own
    // `compare`) with the same level-decaying ac weight APPLY_AC uses, so an armour's
    // N-per-class value scores like an APPLY_AC of -N: real armour counts as armour
    // where it matters (low level), fading to 0 by L40. Read from the prototype -- the
    // base class is not rolled or enchanted (enchant armour adds an APPLY_AC affect).
    if (itemType == ITEM_ARMOR && pProto != 0) {
        // A random armor rolls its AC on the instance; everything else matches its prototype.
        double acAvg = inst != 0 ? (inst->value0( ) + inst->value1( ) + inst->value2( )) / 3.0
                                 : (pProto->value[0] + pProto->value[1] + pProto->value[2]) / 3.0;
        s += w.ac * acAvg;
        ga_note( "armor", w.ac * acAvg, (int)(acAvg + 0.5) );
    }
    // A generated item's worn buff is cast by the base vnum's onEquip, invisible to
    // the affect loop. Score it at its affix price in M, one M being one measure roll
    // set (dr + hr + 10 hp + 10 mana) times the rolls at the item's level -- the very
    // scale the price was measured on. Generated items are recognised by measure_m.
    bool generated = inst != 0 && !inst->getProperty( "measure_m" ).empty( );
    double t0 = s;
    if (generated) {
        DLString buff = inst->getProperty( "wornbuff" );
        if (!buff.empty( ))
            s += item_wornbuff_points( buff, inst->level, w.caster );
    }
    ga_note( "worn_buff", s - t0, 0 );
    // Extra flags (noremove, bless, anti_good...) and the material, at the
    // generator's prices. A material the char may not wear is worth nothing.
    if (pProto != 0) {
        int extras = inst != 0 ? inst->extra_flags : pProto->extra_flags;
        double pts = item_extra_points( extras, w.caster );
        ga_note( "extra_flags", pts, extras );
        s += pts;
        DLString mat = inst != 0 ? DLString( inst->getMaterial( ) ) : DLString( pProto->material );
        pts = item_material_points( mat, itemLevel, w.caster ) * item_fit_material( target, mat );
        ga_note( "material", pts, 0 );
        s += pts;
    }
    t0 = s;
    // Combat spell-procs get scored on what they actually cast (value table x
    // proc chance x item level). That supersedes the flat +50, which was only a
    // stand-in for "this triggers something good in a fight" -- the proc IS that
    // trigger. But a skill-teaching item that ALSO procs still deserves its teach
    // credit on top (different value), and non-proc special gear keeps the +50.
    double procScore = ga_procScore( pProto, weaponSwing, inst, itemLevel, w.caster );
    if (procScore > 0) {
        s += procScore;
        if (ga_grantsSkills( pProto ))
            s += 50;   // it teaches a skill too; procScore only covered the combat cast.
    }
    // A generated item's base vnum carries the shared worn-buff onEquip: that is
    // not "special gear", its worth was scored above.
    else if ((ga_hasFeniaTriggers( pProto ) && !generated) || ga_grantsSkills( pProto )) {
        s += 50;   // Fenia-triggered or skill-teaching gear is almost always very good.
    }
    ga_note( "special", s - t0, 0 );
    return s;
}

// Score a reset PROTOTYPE (a candidate the sage might recommend): base stats only,
// since a candidate has no rolled or enchanted instance yet.
static double ga_score( Character *target, obj_index_data *pObj, const GAWeights &w,
                        const int rawStat[6], const int capStat[6], bool worn )
{
    int sn = 0, ave = 0;
    bool compound = false;
    if (pObj->item_type == ITEM_WEAPON) {
        sn  = get_weapon_sn( pObj );
        ave = weapon_ave( pObj );
        compound = ga_clericCanCompound( target, pObj );
    }
    return ga_scoreCore( target, w, pObj->affected, 0,
                         pObj->item_type, sn, ave, compound, pObj, rawStat, capStat, worn );
}

// Score a LIVE worn instance from its actual stats, not its reset prototype. A
// random weapon carries its rolled dice, damroll and affects on the instance (its
// prototype is the level-0 "dummy random weapon" blank, vnum 104, which scores ~0);
// an enchanted item carries its enchant deltas on the instance too. Weapon dice come
// from the instance value getters (weapon_ave / get_weapon_sn resolve
// instance-or-prototype), and the instance's affects are summed on top of the
// prototype's, exactly as Object::affectedValue does.
static double ga_score( Character *target, ::Object *o, const GAWeights &w,
                        const int rawStat[6], const int capStat[6], bool worn )
{
    int sn = 0, ave = 0;
    bool compound = false;
    if (o->item_type == ITEM_WEAPON) {
        sn  = get_weapon_sn( o );
        ave = weapon_ave( o );
        compound = target != 0 && ga_clericCanCompound( target, o );
    }
    return ga_scoreCore( target, w, o->pIndexData->affected, &o->affected,
                         o->item_type, sn, ave, compound, o->pIndexData, rawStat, capStat, worn, o );
}

// ---- Off-hand model: what a second weapon is worth next to the main one ----------
// Everything below is in ga_score units, where one main-hand swing of a weapon scores
// weaponWeight * dice-per-hit. The off-hand is valued per main-hand swing, from the
// engine's own round (fight.cpp multi_hit_strikes / next_attack / second_weapon_hit):
//   - the off-hand rolls once after the first main-hand swing (chance 100) and once after
//     every extra attack (second..fifth) that fires, at that attack's own chance c;
//   - each roll lands with probability secondWeapon% * (c * m / 100) / 100, m being the
//     class x off-hand-weapon-class modifier (second_weapon_chance_class);
//   - an off-hand hit is a full hit (one_hit secondary): dice at its own skill + damroll.
// So r = off-hand swings per main-hand swing = [P(c=100) + sum_k p_k * P(c_k)] / (1 + sum_k p_k),
// p_k = c_k / 100. Haste and forest fighting add swings to both sides and are left out.
struct GAOffhand {
    int    swPct = 0;                 // second weapon effective %
    std::vector<double> extra;        // extra-attack chances c_k (0..100+)
    double mainSwings = 1.0;          // 1 + sum p_k
    double mainEff = 0;               // main-hand dice per hit
    double damroll = 0;
    double blowValue = 0;             // score of one of the char's own main-hand blows
    double reach = 1.0;               // share of enemy blows that get past parry
    double pShield = 0;               // shield block chance vs an equal-level attacker
    double pCross = 0;                // cross block chance vs an equal-level attacker
};

static double ga_pct( double chance )
{
    return URANGE( 0.0, chance, 100.0 ) / 100.0;
}

// Class bonus the block/parry formulas give warrior, samurai and paladin.
static bool ga_defenderClass( Character *ch )
{
    const DLString &n = ch->getProfession( )->getName( );
    return n == "warrior" || n == "samurai" || n == "paladin";
}

static GAOffhand ga_offhandModel( Character *target, const GAWeights &w, ::Object *primary )
{
    GAOffhand om;
    Skill *sw = skillManager->findExisting( "second weapon" );
    om.swPct = sw ? sw->getEffective( target ) : 0;

    // next_attack: chance = effective / coef + skill_level_bonus, no usable() gate.
    struct { const char *name; int coef; } atk[] = {
        { "second attack", 2 }, { "third attack", 3 }, { "fourth attack", 3 }, { "fifth attack", 3 } };
    for (int i = 0; i < 4; i++) {
        Skill *sk = skillManager->findExisting( atk[i].name );
        if (sk == 0)
            continue;
        double c = sk->getEffective( target ) / atk[i].coef + skill_level_bonus( *sk, target );
        if (c <= 0)
            continue;
        om.extra.push_back( c );
        om.mainSwings += ga_pct( c );
    }

    int mainSk = 0;
    if (primary != 0 && primary->item_type == ITEM_WEAPON)
        om.mainEff = ga_weaponEff( target, get_weapon_sn( primary ), weapon_ave( primary ),
                                   ga_clericCanCompound( target, primary ), &mainSk );
    om.damroll = target->damroll;
    om.blowValue = w.weaponWeight
        * (om.mainEff + om.damroll * std::min( 100, 20 + mainSk ) / 100.0);

    // Defence chain (onehit_undef.cpp canDamage): parry first, then shield block, then
    // cross block. Both builds parry with the main weapon, so only the blocks differ;
    // each is reached by the blows parry let through. Attacker = equal level, so the
    // formulas' "skill_level - attacker level" term is just the skill level bonus.
    bool defClass = ga_defenderClass( target );
    Skill *parry = skillManager->findExisting( "parry" );
    if (parry != 0) {
        int pe = parry->getEffective( target ) / 2;
        if (defClass)
            pe += pe / 5;
        om.reach = 1.0 - ga_pct( pe + skill_level_bonus( *parry, target ) );
    }
    Skill *sb = skillManager->findExisting( "shield block" );
    if (sb != 0 && sb->getEffective( target ) > 1) {
        int c = sb->getEffective( target ) / 2 - 10;
        if (defClass)
            c += 10;
        om.pShield = om.reach * ga_pct( c + skill_level_bonus( *sb, target ) );
    }
    Skill *cb = skillManager->findExisting( "cross block" );
    if (cb != 0 && cb->getEffective( target ) > 1) {
        int c = cb->getEffective( target ) / 3;
        if (defClass)
            c += c / 2;
        om.pCross = om.reach * ga_pct( c + skill_level_bonus( *cb, target ) );
    }
    return om;
}

// Off-hand swings per main-hand swing for an off-hand weapon of this class.
static double ga_offhandRatio( const GAOffhand &om, Character *target, int weaponClass )
{
    int m = second_weapon_chance_class( target->getProfession( ).getElement( ), weaponClass );
    double off = ga_pct( om.swPct * (100.0 * m / 100) / 100 );
    for (size_t k = 0; k < om.extra.size( ); k++)
        off += ga_pct( om.extra[k] ) * ga_pct( om.swPct * (om.extra[k] * m / 100) / 100 );
    return off / om.mainSwings;
}

// Worth of a weapon in the OFF hand. score = its ordinary ga_score, which counted its dice
// as a full main-hand swing (weaponWeight * eff); swap that for r off-hand swings, each
// with the no-shield +5% on the dice (WeaponOneHit::damApplyShield) and the char's damroll
// at this weapon's skill (OneHit::damApplyDamroll). Affixes and procs stay as scored.
static double ga_offhandValue( const GAOffhand &om, Character *target, const GAWeights &w,
                               double score, int weaponSn, int weaponAve, bool canCompound,
                               int weaponClass )
{
    int sk = 0;
    double eff = ga_weaponEff( target, weaponSn, weaponAve, canCompound, &sk );
    double r = ga_offhandRatio( om, target, weaponClass );
    double perSwing = 1.05 * eff + om.damroll * std::min( 100, 20 + sk ) / 100.0;
    return score - w.weaponWeight * eff + w.weaponWeight * r * perSwing;
}

// Worth of a completed set's declared <affects> bonus for this profile, cap-aware.
// The bonus stacks on top of the member items, so its stats score against the
// char's current baseline (a cheap, honest approximation -- not the post-assembly
// stats). No weapon dice, no Fenia-trigger boost: a set bonus is pure affects.
// 0 for a data-empty set (skills-only / full-Fenia), which is exactly why those
// never rate as "worth completing".
static double ga_setValue( Character *target, SetBehavior *sb, const GAWeights &w,
                           const int rawStat[6], const int capStat[6] )
{
    double s = 0;
    int statDelta[6] = { 0, 0, 0, 0, 0, 0 };
    for (auto &sa: sb->affects) {
        Affect af;
        sa.fill( af );
        // A set bonus is a chase reward (candidate), so it gets the redundancy check too.
        ga_accumAffect( af, w, s, statDelta, target, false );
    }
    for (int k = 0; k < 6; k++) {
        if (statDelta[k] == 0 || w.stat[k] == 0)
            continue;
        s += w.stat[k] * ga_statgain( rawStat[k], statDelta[k], capStat[k] );
    }
    return s;
}

// Read an int/bool from a behavior's props JSON, with a default.
static int ga_propInt( SetBehavior *sb, const char *key, int def )
{
    return sb->props.isMember( key ) ? sb->props[key].asInt( ) : def;
}
static bool ga_propBool( SetBehavior *sb, const char *key, bool def )
{
    return sb->props.isMember( key ) ? sb->props[key].asBool( ) : def;
}

// Allow-everything road predicates: pathfind like the immortal 'find' command,
// crossing locked doors, extra exits and portals. We only COUNT the obstacles on
// the shortest route, we do not route around them.
struct GAGoAlwaysDoor   { inline bool operator () ( Room *, EXIT_DATA * )       const { return true; } };
struct GAGoAlwaysEExit  { inline bool operator () ( Room *, EXTRA_EXIT_DATA * ) const { return true; } };
struct GAGoAlwaysPortal { inline bool operator () ( Room *, ::Object * )        const { return true; } };
typedef RoomRoadsIterator<GAGoAlwaysDoor, GAGoAlwaysEExit, GAGoAlwaysPortal> GAHookIterator;

// Records the rooms of the found path (target..start, order irrelevant to us).
struct GAPathComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    GAPathComplete( Room *t, std::vector<Room *> &r ) : target( t ), rooms( r ) { }
    inline bool operator () ( const MyNodesEntry *const head, bool )
    {
        if (head->node != target)
            return false;
        for (const MyNodesEntry *i = head; i; i = i->prev)
            rooms.push_back( i->node );
        return true;
    }
    Room *target;
    std::vector<Room *> &rooms;
};

// Walk the route from 'start' to room 'destVnum' and tally the danger a player
// meets: aggressive mobs that reset along it, locked doors crossed, and whether
// any room needs flight. 0/0/0 for no/unreachable destination. Reset-prototype
// view (stable), not live wandering mobs. Only mobs that would actually aggress
// THIS char count (aggression.cpp canAggressNormal): a mob 6+ levels below the
// char never aggresses, and nothing aggresses a vampire.
static void ga_pathcost( Room *start, int destVnum, int chLevel, bool isVampire, int &aggros, int &doors, int &fly )
{
    aggros = 0; doors = 0; fly = 0;
    if (!start || destVnum <= 0)
        return;
    Room *dest = get_room_instance( destVnum );
    if (!dest || dest == start)
        return;

    for (RoomVector::iterator r = roomInstances.begin( ); r != roomInstances.end( ); r++)
        REMOVE_BIT( (*r)->room_flags, ROOM_MARKER );

    std::vector<Room *> rooms;
    GAGoAlwaysDoor gd; GAGoAlwaysEExit ge; GAGoAlwaysPortal gp;
    GAHookIterator iter( gd, ge, gp );
    GAPathComplete complete( dest, rooms );
    room_traverse( start, iter, complete, 10000 );

    if (rooms.empty( ))
        return;

    // Aggressive resets + flight, per room on the route.
    for (size_t i = 0; i < rooms.size( ); i++) {
        Room *rm = rooms[i];
        if (rm->getSectorType( ) == SECT_AIR)
            fly = 1;
        if (rm->pIndexData && !isVampire)
            for (ResetList::iterator pr = rm->pIndexData->resets.begin( ); pr != rm->pIndexData->resets.end( ); pr++)
                if ((*pr)->command == 'M') {
                    MOB_INDEX_DATA *m = get_mob_index( (*pr)->arg1 );
                    if (m && IS_SET(m->act, ACT_AGGRESSIVE) && m->level >= chLevel - 5)
                        aggros++;
                }
    }

    // Locked doors between adjacent rooms (check either direction; a portal or
    // extra-exit hop simply matches nothing and counts zero).
    for (size_t i = 0; i + 1 < rooms.size( ); i++) {
        Room *a = rooms[i], *b = rooms[i + 1];
        bool locked = false;
        for (int d = 0; d < DIR_SOMEWHERE && !locked; d++) {
            if (a->exit[d] && a->exit[d]->u1.to_room == b && IS_SET(a->exit[d]->exit_info, EX_LOCKED))
                locked = true;
            if (b->exit[d] && b->exit[d]->u1.to_room == a && IS_SET(b->exit[d]->exit_info, EX_LOCKED))
                locked = true;
        }
        if (locked)
            doors++;
    }
}

// The character who ultimately holds this object (worn or carried), walking out of
// any nested containers. Null when the item rests in a room / on a shop floor.
static Character * ga_rootCarrier( ::Object *o )
{
    ::Object *top = o;
    while (top->in_obj != 0)
        top = top->in_obj;
    return top->carried_by;
}

// Difficulty band 0 easy / 1 medium / 2 hard / 3 deadly. Single source of truth: the
// Fenia render only names and colours it. Quests are always medium. The guard is read
// as guard level plus its tier's extra levels (ga_effGuard), and the tier sets a floor:
// an elite is never easy, a champion is at least hard, a boss or a legend is deadly.
// Deadly is also any fight 15+ effective levels above the char.
static int ga_band( int method, int guard, int tier, int chLevel, int aggros, int doors, int fly )
{
    if (method == GA_QUEST || method == GA_UNKNOWN)   // no known route -> not "easy"
        return 1;
    int eff = ga_effGuard( guard, tier );
    int band;
    if (eff >= chLevel + 15)
        band = 3;
    else if (eff >= chLevel + 6 || aggros >= 4 || doors >= 3)
        band = 2;
    else if (eff <= chLevel && aggros == 0 && doors <= 1 && fly == 0)
        band = 0;
    else
        band = 1;
    int floor = tier <= 2 ? 3 : tier <= 3 ? 2 : tier <= 5 ? 1 : 0;
    return std::max( band, floor );
}

// Build one result row for Fenia: [objW, method, aux, room, cost, guard, aggros,
// doors, fly, band]. Path cost is computed here (cached per destination room) so
// it only runs for the <=10 final picks, never the whole candidate set. Quest and
// sourceless picks carry no path.
static Register ga_buildEntry( GACand &c, Room *msm, int chLevel, bool isVampire,
                               std::map<int, std::vector<int> > &pathCache, double gain,
                               bool fillsFree = false, int replaceVnum = 0 )
{
    GAAcq &ac = c.acq;
    int aggros = 0, doors = 0, fly = 0;
    if (ac.method != GA_QUEST && ac.method != GA_UNKNOWN && ac.room > 0) {
        std::map<int, std::vector<int> >::iterator ci = pathCache.find( ac.room );
        if (ci != pathCache.end( )) {
            aggros = ci->second[0]; doors = ci->second[1]; fly = ci->second[2];
        } else {
            ga_pathcost( msm, ac.room, chLevel, isVampire, aggros, doors, fly );
            std::vector<int> v; v.push_back( aggros ); v.push_back( doors ); v.push_back( fly );
            pathCache[ac.room] = v;
        }
    }
    int band = ga_band( ac.method, ac.guard, ac.tier, chLevel, aggros, doors, fly );

    RegList::Pointer e( NEW );
    if (c.inst)
        e->push_back( WrapperManager::getThis( )->getWrapper( c.inst ) );
    else
        e->push_back( WrapperManager::getThis( )->getWrapper( c.pObj ) );
    e->push_back( Register( ac.method ) );
    e->push_back( Register( ac.aux ) );
    e->push_back( Register( ac.room ) );
    e->push_back( Register( ac.cost ) );
    e->push_back( Register( ac.guard ) );
    e->push_back( Register( aggros ) );
    e->push_back( Register( doors ) );
    e->push_back( Register( fly ) );
    e->push_back( Register( band ) );
    // Profile-weighted score improvement over the worn item, rounded, for display.
    e->push_back( Register( gain >= 0 ? (int)(gain + 0.5) : (int)(gain - 0.5) ) );
    // 1 when this pick goes into a still-empty position of a multi-position slot (a
    // second ring/bracelet, a dual-wield off-hand): an ADDITION, not a swap, so the
    // render shows it as a fill (gains only, no "Replaces"). 0 for a plain replacement.
    e->push_back( Register( fillsFree ? 1 : 0 ) );
    // 1 when the acquisition route is actionable now; 0 only for a limited item with no
    // reachable copy and no quest route -- the render then says its whereabouts are
    // unknown instead of a kill/pickup route pointing at a copy that isn't there.
    e->push_back( Register( c.present ? 1 : 0 ) );
    // vnum of the worn item this pick would replace, when that is not simply the one
    // piece the render finds by slot -- a paired slot's WEAKER of two. 0 = a fill or a
    // single-slot swap, and the render falls back to its own .tmp.advice.worn lookup.
    e->push_back( Register( replaceVnum ) );
    return wrap( e );
}

// One swing of the char's current weapon, in score units -- the fallback swing value
// for a non-weapon item's combathits (its extra attacks fire the worn weapon). Gate
// mirrors ga_scoreCore's dice gate so exotic (available()==false, getLearned>0) counts.
static double ga_curWeaponSwing( Character *target, const GAWeights &w )
{
    Wearlocation *wieldLoc = wearlocationManager->findExisting( "wield" );
    ::Object *wep = wieldLoc ? wieldLoc->find( target ) : 0;
    if (wep != 0 && wep->item_type == ITEM_WEAPON) {
        int wsn  = get_weapon_sn( wep );
        int wpct = target->getSkill( wsn );
        Skill *wsk = skillManager->find( wsn );
        if (wsk == 0 || wsk->available( target ) || wpct > 0)
            return w.weaponWeight * weapon_ave( wep ) * (20 + wpct) / 100.0;
    }
    return -1.0;
}

// Permanent (duration < 0) affect_flags: the slot-independent floor of heldFlags.
// Temp spell buffs excluded.
static bitstring_t ga_permaFlags( Character *target )
{
    bitstring_t perma = 0;
    for (auto &paf: target->affected)
        if (paf->duration < 0 && paf->bitvector.getTable( ) == &affect_flags) {
            bitstring_t hb = paf->bitvector; perma |= hb;
        }
    return perma;
}

// imm/res the char owns regardless of gear: race + permanent affects (decision 6 floor).
static void ga_permaResist( Character *target, bitstring_t &imm, bitstring_t &res )
{
    imm = target->getRace( )->getImm( ).getValue( );
    res = target->getRace( )->getRes( ).getValue( );
    for (auto &paf: target->affected) {
        if (paf->duration >= 0)
            continue;
        bitstring_t b = paf->bitvector;
        if (paf->bitvector.getTable( ) == &imm_flags) imm |= b;
        if (paf->bitvector.getTable( ) == &res_flags) res |= b;
    }
}

// affect_flags and imm/res bits an item grants: prototype affects, plus the instance's
// own when there is one (a worn item, or a rolled item the char carries).
static void ga_grantBits( obj_index_data *pObj, ::Object *inst,
                          bitstring_t &aff, bitstring_t &imm, bitstring_t &res )
{
    aff = imm = res = 0;
    auto take = [&]( const AffectList &list ) {
        for (auto &paf: list) {
            bitstring_t b = paf->bitvector;
            if (paf->bitvector.getTable( ) == &affect_flags) aff |= b;
            if (paf->bitvector.getTable( ) == &imm_flags)    imm |= b;
            if (paf->bitvector.getTable( ) == &res_flags)    res |= b;
        }
    };
    take( pObj->affected );
    if (inst != 0)
        take( inst->affected );
}

// imm/res bits one item grants through its affects (proto + instance).
static void ga_itemResist( ::Object *o, bitstring_t &imm, bitstring_t &res )
{
    for (auto &paf: o->pIndexData->affected) {
        bitstring_t b = paf->bitvector;
        if (paf->bitvector.getTable( ) == &imm_flags) imm |= b;
        if (paf->bitvector.getTable( ) == &res_flags) res |= b;
    }
    for (auto &paf: o->affected) {
        bitstring_t b = paf->bitvector;
        if (paf->bitvector.getTable( ) == &imm_flags) imm |= b;
        if (paf->bitvector.getTable( ) == &res_flags) res |= b;
    }
}

// Stat baseline + cap for cap-aware scoring (a stat point at the cap is worth
// nothing). rawStat = perm+mod (uncapped current); capStat = the char's cap.
static void ga_statBaseline( Character *target, int rawStat[6], int capStat[6] )
{
    PCharacter *pch = target->getPC( );
    for (int k = 0; k < 6; k++) {
        int sc = ga_statMap[k];
        rawStat[k] = target->perm_stat[sc] + target->mod_stat[sc];
        capStat[k] = pch ? pch->getMaxStat( sc ) : MAX_STAT;
    }
}

// Root .itemPoints: an item's BASE value through the sage's scorer -- no char, no fit
// clauses, weights (AC) at the item's level. Stats uncapped: a baseline far from both
// ends makes ga_statgain return the delta itself.
double ga_item_points( ::Object *o, bool caster )
{
    GAWeights w;
    item_weights( w, caster, o->level );
    int rawStat[6], capStat[6];
    for (int k = 0; k < 6; k++) {
        rawStat[k] = 500;
        capStat[k] = 1000;
    }
    return ga_score( 0, o, w, rawStat, capStat, false );
}

// Root .itemScore and ch.gearTerms: an item scored for one char the way gearAdvice
// scores a candidate (base x fit): same weights, stat caps, spell factor, weapon skill,
// and the flags and resists the char already holds from perma affects and worn gear
// outside this item's slot (self, the item being scored, is skipped when worn).
static void ga_itemContext( Character *target, int mySlot, ::Object *self, bool caster,
                            GAWeights &w, int rawStat[6], int capStat[6] )
{
    w.caster = caster;
    item_weights( w, caster, target->getRealLevel( ) );
    w.spellFactor = ga_spellFactor( target );
    w.curWeaponSwing = ga_curWeaponSwing( target, w );

    w.heldFlags = ga_permaFlags( target );
    ga_permaResist( target, w.heldImm, w.heldRes );
    for (::Object *wo = target->carrying; wo; wo = wo->next_content) {
        if (wo == self || wo->wear_loc == wear_none)
            continue;
        int slot = wo->pIndexData->wear_flags;
        REMOVE_BIT( slot, ITEM_TAKE );
        if (slot == mySlot)
            continue;
        ga_itemResist( wo, w.heldImm, w.heldRes );
        for (auto &paf: wo->pIndexData->affected)
            if (paf->bitvector.getTable( ) == &affect_flags) {
                bitstring_t hb = paf->bitvector; w.heldFlags |= hb;
            }
        for (auto &paf: wo->affected)
            if (paf->bitvector.getTable( ) == &affect_flags) {
                bitstring_t hb = paf->bitvector; w.heldFlags |= hb;
            }
    }

    ga_statBaseline( target, rawStat, capStat );
}

// One item scored for one char -- a live object o, or a prototype pObj (a candidate
// the sage names) when o is null -- optionally filing the score's terms into terms.
static double ga_item_terms( Character *target, ::Object *o, obj_index_data *pObj, bool caster, GATerms *terms )
{
    GAWeights w;
    int rawStat[6], capStat[6];
    obj_index_data *proto = o != 0 ? o->pIndexData : pObj;
    int mySlot = proto->wear_flags;
    REMOVE_BIT( mySlot, ITEM_TAKE );
    bool worn = o != 0 && o->carried_by == target && o->wear_loc != wear_none;
    ga_itemContext( target, mySlot, o, caster, w, rawStat, capStat );

    // Only this item's own ga_score files terms; the context above scored other gear.
    struct TermsScope {
        TermsScope( GATerms *t ) { ga_terms = t; }
        ~TermsScope( ) { ga_terms = 0; }
    } scope( terms );
    return o != 0 ? ga_score( target, o, w, rawStat, capStat, worn )
                  : ga_score( target, pObj, w, rawStat, capStat, false );
}

double ga_item_score( Character *target, ::Object *o, bool caster )
{
    return ga_item_terms( target, o, 0, caster, 0 );
}

NMI_INVOKE( CharacterWrapper, gearTerms, "(obj, profile): what the sage's score of obj (an object or a prototype) is made of for this char, as a list of [key, points, raw]: key = apply name ('hit', 'move', 'saves', 'str', 'learned:illusion'), a flag table with its bits ('res_flags:4096'), or dice/weapon_flags/armor/worn_buff/extra_flags/material/special; points = score points, rounded; raw = summed modifier (dice average, armor class average, flag bits). Unscored terms are left out. profile caster|melee" )
{
    checkTarget( );
    const Register &r = argnum( args, 1 );
    DLString profile = argnum2string( args, 2 );
    obj_index_data *pObj = 0;
    ::Object *obj = 0;
    if (r.type == Register::OBJECT) {
        ObjIndexWrapper *iw = r.toHandler( ).getDynamicPointer<ObjIndexWrapper>( );
        if (iw)
            pObj = iw->getTarget( );
    }
    if (pObj == 0)
        obj = arg2item( r );

    GATerms terms;
    ga_item_terms( target, obj, pObj, profile == "caster", &terms );

    RegList::Pointer rc( NEW );
    for (auto &t: terms) {
        RegList::Pointer e( NEW );
        e->push_back( Register( t.first ) );
        double p = t.second.pts;
        e->push_back( Register( p >= 0 ? (int)(p + 0.5) : (int)(p - 0.5) ) );
        e->push_back( Register( t.second.raw ) );
        rc->push_back( wrap( e ) );
    }
    return wrap( rc );
}

NMI_INVOKE( CharacterWrapper, gearAdvice, "(profile, [lockedSlots], [slotFilter]): [pct, optimal, best, dual] -- dual = [state(0 n/a/1 second weapon not yet learned/2 verdict), unlockLevel, dualScore(best off-hand weapon build), keepScore(best shield + held item build), entry(an optimal-style entry for a better off-hand weapon, or null)]. best gear the char can wear now, ranked. best is retired (always empty, kept for shape): the chase list 'optimal' now carries the single best-obtainable pick per slot, no dream list beside it. Each optimal/best entry is [objW, method(0kill/1buy/2pickup/3quest/4unknown/5inpack/6request -- 5 = an upgrade the char already carries unworn, render says put it on; 6 = a good char can politely ask a good, roughly-peer mob for it with no fight), aux(holder/shop/quest vnum), roomVnum, cost, guardLevel, aggrosOnWay, lockedDoorsOnWay, flyRequired, band(0easy/1med/2hard/3deadly -- guard level plus its mob tier), scoreGain(profile-weighted score improvement over the worn item, rounded), fillsFree(1 if this pick adds to a still-empty position of a multi-position slot -- second ring/bracelet or dual-wield off-hand -- rather than replacing a worn item; 0 otherwise), present(1 if the route is actionable now; 0 only for a limited item with no reachable copy and no quest route -> render says whereabouts unknown), replaceVnum(vnum of the worn item this pick replaces when it is the weaker of two in a paired finger/neck/wrist slot; 0 = a fill or a single-slot swap -> render names the worn piece via its own slot lookup)]. profile=caster|melee; lockedSlots=wear_flags bitmask of complete-set slots to skip; slotFilter=single wear_flags bit (or GA_SLOT_LIGHT = 1<<30 for the light slot, which has no wear bit) -> optimal is the top-5 for that slot only (pct 0, best empty)" )
{
    checkTarget( );

    Scripting::RegisterList::const_iterator ai = args.begin( );
    DLString profile = (ai != args.end( )) ? (ai++)->toString( ) : DLString("caster");
    int lockedSlots = (ai != args.end( )) ? (int)((ai++)->toNumber( )) : 0;
    REMOVE_BIT( lockedSlots, ITEM_TAKE );   // never let the TAKE bit false-match a real slot
    int slotFilter = (ai != args.end( )) ? (int)((ai++)->toNumber( )) : 0;
    REMOVE_BIT( slotFilter, ITEM_TAKE );    // slot-browse mode: rank only this wear slot
    GAWeights w;
    if (profile == "melee" || profile == "agile" || profile == "hybrid") {
        // saves: a melee's defence is hp/ac, not save-vs-spell, so a save point is
        // worth less to it than to a caster. 3/pt (was 5) so a pure +save cloth no
        // longer out-values real body armour (hp/hitroll/dex + armour class). The
        // caster branch below keeps 5 -- a squishy caster leans on saves defensively.
        w.caster = false;   // weights: config/fight/item_value.json "melee", defaults = the old constants
    } else {                                    // caster (default)
        w.caster = true;    // weights: config/fight/item_value.json "caster"
    }
    // Stat weights, applies, AC at the char's level: one table with the generators
    // (fight_core/itemmodel.cpp item_weights). Melee values weapon output like its
    // damroll; a caster well under it.
    item_weights( w, w.caster, target->getRealLevel( ) );

    w.curWeaponSwing = ga_curWeaponSwing( target, w );

    int chLevel  = target->getRealLevel( );
    int levelGap = target->getModifyLevel( ) - chLevel;
    // Aggressive mobs never touch a vampire, so they add no danger to its routes.
    bool isVampire = target->getProfession( )->getName( ) == "vampire";

    // slevel only counts as far as the char really casts (ac/slevel/level/beats
    // weights: item_weights above; flag/res values are level-independent).
    w.spellFactor     = ga_spellFactor( target );   // scales slevel by real spell knowledge

    // Redundancy discount (bug: unicorn horn out-ranked lion paw on a sanctuary the
    // char already wore). A flag the char already has for free is worth 0 on a
    // CANDIDATE. But "already has" must exclude the slot being replaced, or a strictly
    // better sanctuary body armour scores its sanctuary at 0 against a worn one that
    // scores it full -> the upgrade goes invisible. So heldFlags is built PER SLOT
    // (perma affects + every worn item OUTSIDE that slot) and set on w just before each
    // candidate is scored. gaPerma is the slot-independent floor: permanent (duration
    // < 0) affect_flags, temp spell buffs excluded. w.heldFlags stays 0 for the worn
    // baseline (worn -> 0 in ga_accumAffect), so a delta can only shrink, never inflate.
    bitstring_t gaPerma = ga_permaFlags( target );
    // APPLY_LEARNED (+N% skill knowledge), valued per +1% by how broad the scope is:
    // one skill < a skill group < all skills. 187 live items carry these.
    // learnSkill/learnGroup/learnAll come from item_weights; ga_accumAffect caps them:
    // x min(m, 100 - effective%) per named skill, x min(m, 25 + 3*remort) for a group/all.

    PCharacter *pch = target->getPC( );
    int rawStat[6], capStat[6];
    ga_statBaseline( target, rawStat, capStat );

    // Live instance census per vnum (single pass over the world). `spawned` = every
    // live copy (feeds the obtainability score below); `gettable` = copies a player
    // could still get -- those NOT locked in another player's hands (held by an NPC,
    // lying in a room, on a shop floor, or in a container that roots to either). Zero
    // gettable copies of a fully-claimed limited item means its reset stands bare (see
    // the skip below).
    std::map<int,int> spawned, gettable;
    // Vnums the char carries UNWORN (loose in inventory OR inside a carried bag): an
    // upgrade in the pack must be routed "put it on" (GA_INPACK), not sent on a
    // treasure hunt, and must survive the unobtainable gate below. rootCarrier walks
    // out of nested containers, so a bagged copy the char holds counts too.
    std::map<int,int> carriedVnum;
    // NPCs (by prototype vnum) actually holding a live copy right now. A limited item
    // reset onto several mobs exists on only ONE of them, so its route must name that
    // holder, not whichever reset source is cheapest (the halberd 15219 sent askers to
    // Tim while the only copy sat with the Instructor).
    std::map<int,std::set<int> > liveHolders;
    for (::Object *o = object_list; o; o = o->next) {
        int vn = o->pIndexData->vnum;
        spawned[vn]++;
        Character *rc = ga_rootCarrier( o );
        if (rc == 0 || rc->is_npc( ))
            gettable[vn]++;
        if (rc != 0 && rc->is_npc( ))
            liveHolders[vn].insert( rc->getNPC( )->pIndexData->vnum );
        else if (rc == target && o->wear_loc == wear_none)
            carriedVnum[vn] = 1;
    }

    // Area-quest reward map: obj vnum -> quest vnum. Two declarative sources: a
    // step's rewardVnum (the engine grants it) and the quest's lootVnums (gear its
    // Fenia hands out or buries in a chest, advertised here but granted by Fenia).
    // A quest source always wins the acquisition, and is let past the special-area
    // filter.
    std::map<int,int> questReward;
    for (std::map<int,AreaQuest *>::iterator qk = areaQuests.begin( ); qk != areaQuests.end( ); qk++) {
        AreaQuest *q = qk->second;
        if (!q)
            continue;
        // Class / alignment / hometown / prerequisite eligibility -- a quest the char
        // can never start (druid-excluded, wrong align/hometown, prereq unreachable)
        // is a dead end, so don't advertise its rewards. The min/max level gates
        // below stay: aquest_can_participate_ever checks eligibility, not level.
        PCharacter *questPch = target->getPC( );
        if (questPch && !aquest_can_participate_ever( questPch, q ))
            continue;
        // Quests the char can't yet start are dead ends, so their rewards are
        // unreachable -- don't advertise them. Mirror both level gates of
        // aquest_can_participate (areaquestutils.cpp): too old (past maxLevel) and
        // too young (below minLevel). Both use getLevel() == the real level, so a
        // remort bonus level bought from Baba Yaga never fakes quest eligibility --
        // it only lowers an item's wear level, not the quest's entry level.
        if (q->maxLevel.getValue( ) < LEVEL_MORTAL && target->getLevel( ) > q->maxLevel.getValue( ))
            continue;
        if (q->minLevel.getValue( ) > 0 && target->getLevel( ) < q->minLevel.getValue( ))
            continue;
        for (int s = 0; s < (int)q->steps.size( ); s++) {
            int rv = q->steps[s]->rewardVnum.getValue( );
            if (rv > 0 && !questReward.count( rv ))
                questReward[rv] = q->vnum.getValue( );
        }
        for (int li = 0; li < (int)q->lootVnums.size( ); li++) {
            int lv = q->lootVnums[li];
            if (lv > 0 && !questReward.count( lv ))
                questReward[lv] = q->vnum.getValue( );
        }
    }

    // Easiest reset acquisition per object vnum, from the world's resets. Per room:
    // pass 1 finds roomMax -- the toughest AGGRESSIVE-or-ASSIST mob that resets here,
    // i.e. the guard you must survive (passive mobs don't guard). Pass 2 walks the
    // resets in order tracking the last-loaded mob (cleared on 'O'/'R', per the
    // olc.cpp badresets walk) and records how each object is obtained, keeping the
    // lowest-guard source: bought from a shopkeeper (guard 0), taken off a mob you
    // kill (guard roomMax), or picked up off the floor / from a container.
    // The newbie-only tutorial zones (Moehewa, MUD School) are barred to a char the game
    // won't let in, so their reset gear must not be advised to one (bug 3334: gremlin
    // boots in Moehewa recommended to a char who cannot reach the cave). The real gate is
    // can_see (character.cpp:50, enforced in walkment): getRealLevel() > PK_MIN_LEVEL
    // cannot enter a newbies_only room -- that is the level-6+ first-lifer the bug hits.
    // The isNewbie half additionally covers a low remort: Moehewa is a one-way onboarding
    // zone, so a non-newbie who technically passes the level gate still cannot walk in.
    // Any room flagged newbies_only marks the whole gated area; collect those areas once,
    // only for a gated asker (an in-onboarding newbie still gets the advice).
    bool advGated = !target->is_immortal( )
        && (target->getRealLevel( ) > PK_MIN_LEVEL || (pch && !Player::isNewbie( pch )));
    std::set<AreaIndexData *> newbieAreas;
    if (advGated)
        for (std::map<int,RoomIndexData *>::iterator rk = roomIndexMap.begin( ); rk != roomIndexMap.end( ); rk++)
            if (rk->second->areaIndex && IS_SET( rk->second->room_flags, ROOM_NEWBIES_ONLY ))
                newbieAreas.insert( rk->second->areaIndex );

    std::map<int,GAAcq> acq;
    // The same mob-sourced routes keyed by holder: mob vnum -> (obj vnum -> route).
    std::map<int,std::map<int,GAAcq> > acqByHolder;
    for (std::map<int,RoomIndexData *>::iterator rk = roomIndexMap.begin( ); rk != roomIndexMap.end( ); rk++) {
        RoomIndexData *pRoom = rk->second;
        int roomVnum = rk->first;

        // A reset in a system/hidden/wizlock area (Limbo, dev zones) is not a
        // place a player can walk to, so an item that also resets there must be
        // found by its real resets -- never record the unreachable one. Bug 3334
        // advised a hat as "kill a player in Limbo" when it resets in a live zone
        // too. DUNGEON/CLAN/MANSION stay in: those are legit, reachable loot.
        if (pRoom->areaIndex
            && IS_SET(pRoom->areaIndex->area_flag, AREA_SYSTEM|AREA_HIDDEN|AREA_WIZLOCK))
            continue;

        // Newbie-gated zone, gated asker: unreachable to them, skip its reset gear
        // (bug 3334). Quest rewards bypass this scan and are already onboarding-gated.
        if (advGated && pRoom->areaIndex && newbieAreas.count( pRoom->areaIndex ))
            continue;

        // roomTier: the toughest tier among the same guards.
        int roomMax = 0, roomTier = MobTiers::TIER_WORST;
        for (ResetList::iterator pr = pRoom->resets.begin( ); pr != pRoom->resets.end( ); pr++)
            if ((*pr)->command == 'M') {
                MOB_INDEX_DATA *m = get_mob_index( (*pr)->arg1 );
                if (m && (IS_SET(m->act, ACT_AGGRESSIVE) || IS_SET(m->off_flags, GA_ASSIST_MASK))) {
                    roomMax = std::max( roomMax, m->level );
                    roomTier = std::min( roomTier, m->tier );
                }
            }

        MOB_INDEX_DATA *lastMob = 0;
        for (ResetList::iterator pr = pRoom->resets.begin( ); pr != pRoom->resets.end( ); pr++) {
            char cmd = (*pr)->command;
            int a1 = (*pr)->arg1;
            if (cmd == 'M') { lastMob = get_mob_index( a1 ); continue; }
            if (cmd == 'R') { lastMob = 0; continue; }
            if (cmd == 'O') { ga_record( acq, a1, GA_PICKUP, 0, roomVnum, 0, roomMax, roomTier ); lastMob = 0; continue; }
            if (cmd == 'P') { ga_record( acq, a1, GA_PICKUP, 0, roomVnum, 0, roomMax, roomTier ); continue; }
            if (cmd == 'G' || cmd == 'E') {
                if (!lastMob)
                    continue;
                // Shop stock is 'G' onto a shopkeeper; 'E' on one is its own worn gear.
                // Shopkeepers are old-style behaviors: the prototype carries a
                // <behavior type="ShopTrader"> XML doc in ->behavior, NOT a bedit entry
                // in the ->behaviors bitvector, so behaviorManager->findExisting never
                // sees them (that lookup returned null and every shop read as GA_KILL).
                // Read the doc's root type attribute -- the idiom MobileBehaviorManager
                // ::assign uses. All 276 shop mobs in the world are this old style.
                bool trader = false;
                if (lastMob->behavior) {
                    XMLNode::Pointer root = lastMob->behavior->getFirstNode( );
                    if (root && root->getAttribute( XMLNode::ATTRIBUTE_TYPE ) == "ShopTrader")
                        trader = true;
                }
                if (trader && cmd == 'G') {
                    obj_index_data *po = get_obj_index( a1 );
                    ga_record( acq, a1, GA_BUY, lastMob->vnum, roomVnum, po ? po->cost : 0, 0 );
                    ga_record( acqByHolder[lastMob->vnum], a1, GA_BUY, lastMob->vnum, roomVnum, po ? po->cost : 0, 0 );
                } else {
                    // You must kill the holder, so its own level floors the fight
                    // even when it is passive and roomMax (aggro/assist only) is lower.
                    int killGuard = std::max( roomMax, lastMob->level );
                    int killTier = std::min( roomTier, lastMob->tier );
                    // A good asker can politely REQUEST the item from a good, roughly-peer
                    // mob (command/request) -- no fight, so the holder's level drops out of
                    // the difficulty and only the room's other aggro (roomMax) remains.
                    // Mirrors request/runFunc's core gate: good-to-good, owner under
                    // asker+10 and under 2x asker level, item not an anti-good limited one
                    // nor the Knight's key (vnum 520).
                    obj_index_data *rpo = get_obj_index( a1 );
                    // Asker level uses getModifyLevel -- the value the command actually
                    // compares -- so a level-drained char is not over-promised (F4).
                    int askerLvl = target->getModifyLevel( );
                    // A cursed item (nodrop, or worn-noremove) can't be handed over via
                    // request -- UNLESS the asker knows remove curse and can strip it once
                    // it's theirs (mirrors the relaxed request/runFunc gate; Kit's steer).
                    bool cursedStuck = rpo
                        && IS_SET( rpo->extra_flags, ITEM_NODROP|ITEM_NOREMOVE )
                        && !ga_canSelfCast( target, "remove curse" );
                    bool canRequest =
                           IS_GOOD( target )
                        && lastMob->alignment >= 350
                        && lastMob->level < askerLvl + 10
                        && lastMob->level < askerLvl * 2
                        && a1 != 520
                        && !( rpo && IS_SET( rpo->extra_flags, ITEM_ANTI_GOOD ) && rpo->limit >= 0 )
                        && !cursedStuck
                        // A safe room or an ACT_SAFE mob makes the exchange impossible
                        // (ch.is_safe -> "под защитой богов"): never route there. The KILL
                        // fallback is equally blocked, so this is an honest refusal, not a
                        // regression (F2).
                        && !IS_SET( pRoom->room_flags, ROOM_SAFE|ROOM_NO_DAMAGE )
                        && !IS_SET( lastMob->act, ACT_SAFE );
                    if (canRequest) {
                        ga_record( acq, a1, GA_REQUEST, lastMob->vnum, roomVnum, 0, roomMax, roomTier );
                        ga_record( acqByHolder[lastMob->vnum], a1, GA_REQUEST, lastMob->vnum, roomVnum, 0, roomMax, roomTier );
                    } else {
                        ga_record( acq, a1, GA_KILL, lastMob->vnum, roomVnum, 0, killGuard, killTier );
                        ga_record( acqByHolder[lastMob->vnum], a1, GA_KILL, lastMob->vnum, roomVnum, 0, killGuard, killTier );
                    }
                }
            }
        }
    }

    // Worn gear: best score per slot-type (for the gap + percentile), plus every
    // vnum already worn (so we never recommend re-getting one).
    std::map<int,double> wornSlot;
    std::map<int,int> wornVnum;
    std::map<int,bitstring_t> wornFlagsBySlot;   // slot -> affect_flags its worn item(s) grant
    std::map<int,bitstring_t> wornImmBySlot, wornResBySlot;   // slot -> imm/res bits, decision 6
    for (::Object *o = target->carrying; o; o = o->next_content) {
        if (o->wear_loc == wear_none)   // unworn -> handled by carriedVnum (census loop above)
            continue;
        wornVnum[o->pIndexData->vnum] = 1;
        int slot = o->pIndexData->wear_flags;
        REMOVE_BIT( slot, ITEM_TAKE );
        // Record which affect_flags this worn item grants, keyed by its slot, so the
        // per-candidate redundancy set can exclude a candidate's OWN slot (F1).
        for (auto &paf: o->pIndexData->affected)
            if (paf->bitvector.getTable( ) == &affect_flags) {
                bitstring_t hb = paf->bitvector; wornFlagsBySlot[slot] |= hb;
            }
        for (auto &paf: o->affected)
            if (paf->bitvector.getTable( ) == &affect_flags) {
                bitstring_t hb = paf->bitvector; wornFlagsBySlot[slot] |= hb;
            }
        ga_itemResist( o, wornImmBySlot[slot], wornResBySlot[slot] );
        // Set slots now score normally: the set optimizer below credits a complete
        // set's bonus into the ceiling and protects its slots from break-advice.
        double sc = ga_score( target, o, w, rawStat, capStat, true );
        if (sc > wornSlot[slot])
            wornSlot[slot] = sc;
    }

    // Per-slot redundancy set: perma + every worn slot EXCEPT this one, so a same-slot
    // upgrade is never scored against a flag it would itself preserve. heldExclSlot is
    // built for occupied slots; a candidate for an empty slot uses heldAll.
    bitstring_t heldAll = gaPerma;
    for (std::map<int,bitstring_t>::iterator wi = wornFlagsBySlot.begin( ); wi != wornFlagsBySlot.end( ); wi++)
        heldAll |= wi->second;
    std::map<int,bitstring_t> heldExclSlot;
    for (std::map<int,bitstring_t>::iterator wi = wornFlagsBySlot.begin( ); wi != wornFlagsBySlot.end( ); wi++) {
        bitstring_t h = gaPerma;
        for (std::map<int,bitstring_t>::iterator wj = wornFlagsBySlot.begin( ); wj != wornFlagsBySlot.end( ); wj++)
            if (wj->first != wi->first)
                h |= wj->second;
        heldExclSlot[wi->first] = h;
    }
    // The same "owned outside this slot" sets for imm/res (decision 6, model only).
    bitstring_t permaImm, permaRes;
    ga_permaResist( target, permaImm, permaRes );
    bitstring_t immAll = permaImm, resAll = permaRes;
    for (auto &wi: wornImmBySlot) immAll |= wi.second;
    for (auto &wi: wornResBySlot) resAll |= wi.second;
    std::map<int,bitstring_t> immExclSlot, resExclSlot;
    for (auto &wi: wornImmBySlot) {
        bitstring_t hi = permaImm, hr = permaRes;
        for (auto &wj: wornImmBySlot)
            if (wj.first != wi.first) hi |= wj.second;
        for (auto &wj: wornResBySlot)
            if (wj.first != wi.first) hr |= wj.second;
        immExclSlot[wi.first] = hi;
        resExclSlot[wi.first] = hr;
    }

    // Catalogue every wearable prototype this char can wear right now.
    std::vector<GACand> cands;
    // Second-copy fills for an explicit slot-browse of a paired slot (finger/neck/wrist,
    // or a dual-wield off-hand): a copy of an already-worn item that could go in its
    // empty twin position. Collected here so it never touches the percentile/optimal
    // passes, then merged into slotCands when that slot has a free position.
    std::vector<GACand> secondCopyCands;
    std::map<int,double> bestSlot;

    // Body slots a race may lack -- resolved once (findExisting is what
    // arg2wearloc / hasWearloc do). WEAR_HORSE/WEAR_HOOVES/WEAR_FEET are
    // deprecated wear_loc_flags enum codes, NOT wearlocationManager indices, so
    // getWearloc().isSet() must be handed the slot OBJECT, never the enum code.
    Wearlocation *horseLoc  = wearlocationManager->findExisting( "horse" );
    Wearlocation *hoovesLoc = wearlocationManager->findExisting( "hooves" );
    Wearlocation *feetLoc   = wearlocationManager->findExisting( "feet" );

    // Material restriction: some classes/religions can't wear whole material
    // groups (druids shun metal). Computed once; the per-candidate skip is below.
    int badMaterials = material_types_forbidden( target );

    for (int i = 0; i < MAX_KEY_HASH; i++)
    for (obj_index_data *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
        if (pObj->level > LEVEL_MORTAL)
            continue;
        // Gear from areas a player can't normally loot -- system holders,
        // hidden/wizlock dev zones, clan halls, mansions, dungeons. Engine's own
        // special-area mask (cf. where.cpp / traverse). Area-quest rewards live in
        // such zones but are legitimately earned -- keep those (e.g. cassandra 89).
        if (pObj->area
            && IS_SET(pObj->area->area_flag,
                      AREA_SYSTEM|AREA_HIDDEN|AREA_CLAN|AREA_MANSION|AREA_WIZLOCK|AREA_DUNGEON)
            && !questReward.count( pObj->vnum ))
            continue;
        if (!IS_SET(pObj->wear_flags, ITEM_TAKE))
            continue;
        int slot = pObj->wear_flags;
        REMOVE_BIT( slot, ITEM_TAKE );
        if (slot == 0 && pObj->item_type != ITEM_LIGHT)
            continue;
        // Already wearing this exact item: normally skip. Exception -- a SECOND copy
        // may fill the empty twin position of a paired slot (finger/neck/wrist, or a
        // dual-wield off-hand), as long as the world isn't capped to one instance
        // (limit 1). In an explicit slot browse this covers the browsed slot (wield
        // included); in the general advice it covers a paired ring/neck/wrist with a
        // free position, so "get a second <ring>" competes for that slot's optimal
        // pick instead of a weaker replacement surfacing alone. Route those to
        // secondCopyCands; they never feed the percentile. The free-position gate is
        // applied where the lists are built (slot-browse capacity, or the generic
        // fill merge below).
        bool secondCopy = false;
        if (wornVnum.count( pObj->vnum )) {
            bool eligible;
            if (slotFilter != 0)
                eligible = (slot & slotFilter) != 0 && pObj->limit != 1;
            else
                // ITEM_WIELD: a second copy of the main-hand weapon is an off-hand
                // option for the shield-or-second-weapon verdict (never a general pick).
                eligible = (slot & (ITEM_WEAR_FINGER | ITEM_WEAR_NECK | ITEM_WEAR_WRIST | ITEM_WIELD)) != 0
                           && pObj->limit != 1;
            if (!eligible)
                continue;
            secondCopy = true;
        }

        // Wearable now? native per-item-type wear level (armour +3, weapon +0, ...).
        int wearMod = target->getProfession( )->getWearModifier( pObj->item_type );
        int wearLvl = pObj->level - wearMod - levelGap;
        if (wearLvl < 1) wearLvl = 1;
        if (wearLvl > chLevel)
            continue;

        // Alignment restriction.
        if (IS_SET(pObj->extra_flags, ITEM_ANTI_EVIL)    && IS_EVIL(target))    continue;
        if (IS_SET(pObj->extra_flags, ITEM_ANTI_GOOD)    && IS_GOOD(target))    continue;
        if (IS_SET(pObj->extra_flags, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(target)) continue;

        // A follower's tattoo slot holds their fixed deity sign -- never advise
        // replacing it. Atheists (no sign) may wear tattoos freely.
        if (IS_SET(pObj->wear_flags, ITEM_WEAR_TATTOO) && target->getReligion( ) != god_none)
            continue;

        // Forbidden material (e.g. a druid can't wear anything metal) -- mirror
        // DefaultWearlocation::canEquip so the sage never advises gear the char
        // would be refused at wear time.
        if (badMaterials != 0 && material_is_typed( pObj->material.c_str( ), badMaterials ))
            continue;

        // Skip items whose body slot the char's race lacks. Bipeds have no
        // horse/hooves slot (saddles, horseshoes); quadrupeds like centaurs have
        // no feet slot (boots). Keeps them out of both the recs and the %.
        if (IS_SET(pObj->wear_flags, ITEM_WEAR_HORSE)  && horseLoc  && !target->getWearloc( ).isSet( horseLoc ))  continue;
        if (IS_SET(pObj->wear_flags, ITEM_WEAR_HOOVES) && hoovesLoc && !target->getWearloc( ).isSet( hoovesLoc )) continue;
        if (IS_SET(pObj->wear_flags, ITEM_WEAR_FEET)   && feetLoc   && !target->getWearloc( ).isSet( feetLoc ))   continue;

        // A limited item crumbles to dust and never saves once the char is more than
        // 20 levels above its level or more than 3 below it (loadsave/character.cpp
        // keep-check, limit>0). Outside that band it can be worn but never kept, so it
        // is not really obtainable -- keep it out of both the recs and the percentile.
        if (pObj->limit > 0) {
            int ml = target->getModifyLevel( );
            if (ml > pObj->level + 20 || ml < pObj->level - 3)
                continue;
        }

        // A limited item that is fully claimed -- every allowed copy exists (live, or
        // in an offline player's profile: both are tallied in pObj->count, the same
        // count resets gate on, update_areas.cpp) -- and not one of them is reachable
        // (each is worn/carried by a player). The reset source stands bare and no new
        // copy will spawn, so no one can get it: keep it out of the recs and the
        // percentile, like an item with no known route. (count < limit passes this
        // skip -- the reset can still make a copy -- but a count<limit item with no
        // reachable copy still drops out downstream at the present gate below, unless
        // the char carries it or it is a quest reward.)
        // ...but if the char is the one holding it (carried unworn), it is reachable
        // to them right now -- keep it so the sage can say "put it on".
        if (pObj->limit > 0 && pObj->count >= pObj->limit && gettable[pObj->vnum] == 0
            && carriedVnum.count( pObj->vnum ) == 0)
            continue;

        // A melee profile is never told to chase a weapon it cannot use. An unskilled
        // weapon collapses toward the 20% dice floor (ga_scoreCore), so it only ever
        // floats up as a chase pick for a class starved of real options -- an axe to a
        // paladin, a staff to a warrior. Drop it from the candidate pool outright.
        // Candidate path ONLY: the worn overload keeps scoring an unskilled weapon so
        // "you already wear the best" still holds for a char stuck with one. Casters
        // are unchanged -- they don't swing, so a weapon is worn for its passive stats.
        // A cleric who can compound the weapon into a mace DOES wield it: keep it.
        if (!w.caster && pObj->item_type == ITEM_WEAPON
            && target->getSkill( get_weapon_sn( pObj ) ) == 0
            && !ga_clericCanCompound( target, pObj ))
            continue;

        // Redundancy set for THIS candidate: perma + worn items in OTHER slots (never
        // the slot it would replace). Empty slot -> nothing to exclude, use heldAll.
        {
            std::map<int,bitstring_t>::iterator hi = heldExclSlot.find( slot );
            w.heldFlags = (hi != heldExclSlot.end( )) ? hi->second : heldAll;
            std::map<int,bitstring_t>::iterator ii = immExclSlot.find( slot );
            w.heldImm = (ii != immExclSlot.end( )) ? ii->second : immAll;
            std::map<int,bitstring_t>::iterator ri = resExclSlot.find( slot );
            w.heldRes = (ri != resExclSlot.end( )) ? ri->second : resAll;
        }
        double sc = ga_score( target, pObj, w, rawStat, capStat, false );
        if (sc <= 0)
            continue;

        // Obtainability 0..1: limited-and-none-spawned is hard; deeper items harder.
        double obtain = 1.0;
        int sp = spawned[pObj->vnum];
        if (pObj->limit > 0 && sp == 0)
            obtain = 0.25;
        else if (pObj->limit > 0)
            obtain = 0.7;
        if (pObj->level > chLevel)
            obtain *= 0.6;

        // How to get it: carried-in-pack wins (it is already in hand); then a quest
        // reward; else the easiest reset source; else no known source. Fold a mild
        // guard demotion into the ranking (cheap, guard only -- the rich path-cost
        // band is computed later, per final pick).
        bool inPack = carriedVnum.count( pObj->vnum ) != 0;
        GAAcq ac;
        if (inPack) {
            ac.method = GA_INPACK; ac.aux = 0; ac.guard = 0;
        } else if (questReward.count( pObj->vnum )) {
            ac.method = GA_QUEST; ac.aux = questReward[pObj->vnum]; ac.guard = 0;
        } else {
            // A limited item goes through whichever NPC holds a live copy now (easiest
            // of them). Unlimited gear repops on every source, so the cheapest reset wins.
            bool fromHolder = false;
            if (pObj->limit > 0) {
                std::map<int,std::set<int> >::iterator lh = liveHolders.find( pObj->vnum );
                if (lh != liveHolders.end( ))
                    for (std::set<int>::iterator h = lh->second.begin( ); h != lh->second.end( ); h++) {
                        std::map<int,std::map<int,GAAcq> >::iterator bh = acqByHolder.find( *h );
                        if (bh == acqByHolder.end( ))
                            continue;
                        std::map<int,GAAcq>::iterator r = bh->second.find( pObj->vnum );
                        if (r != bh->second.end( ) && (!fromHolder
                                || ga_effGuard( r->second.guard, r->second.tier ) < ga_effGuard( ac.guard, ac.tier ))) {
                            ac = r->second;
                            fromHolder = true;
                        }
                    }
            }
            if (!fromHolder) {
                std::map<int,GAAcq>::iterator it = acq.find( pObj->vnum );
                if (it != acq.end( ))
                    ac = it->second;
                else {
                    ac.method = GA_UNKNOWN; ac.guard = pObj->level;
                }
            }
        }
        int effGuard = ga_effGuard( ac.guard, ac.tier );
        if (ac.method == GA_QUEST)
            obtain *= 0.85;
        else if (effGuard <= chLevel - 5)
            ;                        // easy: x1.0
        else if (effGuard <= chLevel + 5)
            obtain *= 0.85;
        else
            obtain *= 0.6;
        if (inPack)
            obtain = 1.0;            // in hand -> fully obtainable, no scarcity/guard penalty

        GACand c;
        c.pObj = pObj; c.slot = slot; c.score = sc; c.obtain = obtain; c.value = 0;
        c.acq = ac;
        // "present" = the route the render will show is actionable right now. Only a
        // scarcity-limited item (limit>0) can ever fail it: unlimited gear repops on
        // every reset, so its "kill X in zone Y" route is good even when the holder is
        // momentarily dead. A limited item is present iff a copy is physically reachable
        // now (gettable>0) OR it is a quest reward (route = complete the quest, needs no
        // copy -- and quest copies sit on the PCs who finished, so gettable==0 is the
        // norm there), OR the char already carries it (GA_INPACK). Otherwise (limited,
        // no reachable copy, no quest, not in the pack) -> not obtainable right now.
        c.present = pObj->limit <= 0 || gettable[pObj->vnum] > 0
                 || ac.method == GA_QUEST || inPack;
        // Don't recommend gear the player cannot obtain right now. A limited item whose
        // every copy sits in other players' hands (none reachable, no quest route) is
        // present=0: drop it from the recs AND from the slot ceiling, exactly as an
        // unknown-route item is dropped, instead of surfacing it as a top pick tagged
        // "whereabouts unknown". Unlimited / quest / in-pack gear is present -> kept.
        if (!c.present)
            continue;
        if (secondCopy) {
            // A second copy never feeds the percentile, optimal or set passes -- the
            // char already wears one. Held only for the slot-browse fill list.
            secondCopyCands.push_back( c );
            continue;
        }
        cands.push_back( c );

        // Only gear the char can actually get sets the slot ceiling: an item with no
        // known route (GA_UNKNOWN -- includes outleveled quest rewards) can't be
        // obtained or worn, so it must not inflate bestSlot / drag down the percentile.
        if (ac.method != GA_UNKNOWN && !ga_overCap( ac, chLevel ) && sc > bestSlot[slot])
            bestSlot[slot] = sc;
    }

    // Rolled items the char carries unworn (random weapons and armor, tagged "tier"
    // by their generators). Their prototype is an empty stub in a system area, so the
    // loop above never sees them: score each copy as itself, same wear gates, and
    // route it "put it on".
    for (::Object *o = object_list; o; o = o->next) {
        if (o->wear_loc != wear_none || ga_rootCarrier( o ) != target)
            continue;
        if (o->getProperty( "tier" ).empty( ))
            continue;
        if (o->level > LEVEL_MORTAL || !IS_SET(o->wear_flags, ITEM_TAKE))
            continue;
        int slot = o->wear_flags;
        REMOVE_BIT( slot, ITEM_TAKE );
        if (slot == 0)
            continue;

        int wearMod = target->getProfession( )->getWearModifier( o->item_type );
        int wearLvl = o->level - wearMod - levelGap;
        if (wearLvl < 1) wearLvl = 1;
        if (wearLvl > chLevel)
            continue;

        if (IS_SET(o->extra_flags, ITEM_ANTI_EVIL)    && IS_EVIL(target))    continue;
        if (IS_SET(o->extra_flags, ITEM_ANTI_GOOD)    && IS_GOOD(target))    continue;
        if (IS_SET(o->extra_flags, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(target)) continue;
        if (badMaterials != 0 && material_is_typed( o->getMaterial( ).c_str( ), badMaterials ))
            continue;
        if (IS_SET(o->wear_flags, ITEM_WEAR_HORSE)  && horseLoc  && !target->getWearloc( ).isSet( horseLoc ))  continue;
        if (IS_SET(o->wear_flags, ITEM_WEAR_HOOVES) && hoovesLoc && !target->getWearloc( ).isSet( hoovesLoc )) continue;
        if (IS_SET(o->wear_flags, ITEM_WEAR_FEET)   && feetLoc   && !target->getWearloc( ).isSet( feetLoc ))   continue;
        if (!w.caster && o->item_type == ITEM_WEAPON
            && target->getSkill( get_weapon_sn( o ) ) == 0
            && !ga_clericCanCompound( target, o ))
            continue;

        {
            std::map<int,bitstring_t>::iterator hi = heldExclSlot.find( slot );
            w.heldFlags = (hi != heldExclSlot.end( )) ? hi->second : heldAll;
            std::map<int,bitstring_t>::iterator ii = immExclSlot.find( slot );
            w.heldImm = (ii != immExclSlot.end( )) ? ii->second : immAll;
            std::map<int,bitstring_t>::iterator ri = resExclSlot.find( slot );
            w.heldRes = (ri != resExclSlot.end( )) ? ri->second : resAll;
        }
        // worn=false: it is not on the char yet, its stats are not in rawStat.
        double sc = ga_score( target, o, w, rawStat, capStat, false );
        if (sc <= 0)
            continue;

        GACand c;
        c.pObj = o->pIndexData; c.inst = o; c.slot = slot; c.score = sc;
        c.obtain = 1.0; c.value = 0; c.present = true;
        c.acq.method = GA_INPACK; c.acq.aux = 0; c.acq.guard = 0;
        cands.push_back( c );
        if (sc > bestSlot[slot])
            bestSlot[slot] = sc;
    }

    // Percentile: per slot-type, worn against that slot's own ceiling (best of the
    // worn item or the best available). Capping each slot at its own best stops a
    // best-in-slot surplus on one slot from papering over a deficit on another, so
    // 100% now means best-in-slot everywhere -- i.e. the optimal list is empty.
    // Each slot-type counted once; multi-slot types (two fingers) are Phase 2.
    std::map<int,double> slotCeil;
    for (auto &kv: wornSlot)
        slotCeil[kv.first] = kv.second;
    for (auto &kv: bestSlot)
        if (kv.second > slotCeil[kv.first])
            slotCeil[kv.first] = kv.second;

    // Flag-once kit totals (percentile only). The per-slot scores above price a flag
    // against the worn gear OUTSIDE that slot, which is right for a swap delta but not
    // for a whole-kit sum: take off the only haste item and every other slot's haste
    // candidate jumps to full price at once, so the ceiling counts haste N times and
    // the % drops after a good swap. Here both the ideal kit and the worn kit grant
    // each affect_flags / imm / res bit once: slots are claimed greedily, highest
    // score first, and a claimed bit is held for every slot after it. Advice deltas
    // (c.score, wornSlot) are untouched.
    // legacyCeil keeps the per-slot model for the comparisons that set against c.score /
    // full-price worn numbers (set worth-it, the left-hand swap): like with like.
    // hybridCeil is the fallback ceiling the totals loop weighs against the greedy ideal.
    std::map<int,double> wornPct = wornSlot;
    std::map<int,double> legacyCeil = slotCeil;
    std::map<int,double> hybridCeil = slotCeil;
    if (slotFilter == 0) {   // a slot browse returns pct 0, skip the work
        struct GAOnce {
            obj_index_data *pObj; ::Object *inst; bool isWorn;
            bitstring_t aff, imm, res;
            double base;   // score as already computed; final when the item grants no bits
        };
        std::map<int,std::vector<GAOnce>> pool;
        for (::Object *o = target->carrying; o; o = o->next_content) {
            if (o->wear_loc == wear_none)
                continue;
            int slot = o->pIndexData->wear_flags;
            REMOVE_BIT( slot, ITEM_TAKE );
            GAOnce e = { o->pIndexData, o, true, 0, 0, 0, 0 };
            ga_grantBits( o->pIndexData, o, e.aff, e.imm, e.res );
            e.base = ga_score( target, o, w, rawStat, capStat, true );
            pool[slot].push_back( e );
        }
        for (auto &c: cands) {
            if (c.acq.method == GA_UNKNOWN || ga_overCap( c.acq, chLevel ))   // same gate as bestSlot
                continue;
            GAOnce e = { c.pObj, c.inst, false, 0, 0, 0, c.score };
            ga_grantBits( c.pObj, c.inst, e.aff, e.imm, e.res );
            pool[c.slot].push_back( e );
        }

        bitstring_t savedFlags = w.heldFlags, savedImm = w.heldImm, savedRes = w.heldRes;
        bitstring_t hAff, hImm, hRes;
        auto best = [&]( int slot, bool wornOnly, const GAOnce *&arg ) -> double {
            double b = 0;
            arg = 0;
            for (auto &e: pool[slot]) {
                if (wornOnly && !e.isWorn)
                    continue;
                double sc = e.base;
                if (e.aff | e.imm | e.res) {
                    w.heldFlags = hAff; w.heldImm = hImm; w.heldRes = hRes;
                    w.heldOnWorn = e.isWorn;
                    sc = e.inst ? ga_score( target, e.inst, w, rawStat, capStat, e.isWorn )
                                : ga_score( target, e.pObj, w, rawStat, capStat, false );
                    w.heldOnWorn = false;
                }
                if (sc > b) { b = sc; arg = &e; }   // floor 0, as wornSlot/bestSlot
            }
            return b;
        };
        auto claim = [&]( bool wornOnly, std::map<int,double> &out ) {
            hAff = gaPerma; hImm = permaImm; hRes = permaRes;
            std::map<int,double> cur;
            std::map<int,const GAOnce *> curArg;
            for (auto &kv: slotCeil) {
                const GAOnce *a;
                cur[kv.first] = best( kv.first, wornOnly, a );
                curArg[kv.first] = a;
            }
            while (!cur.empty( )) {
                std::map<int,double>::iterator top = cur.begin( );
                for (std::map<int,double>::iterator ci = cur.begin( ); ci != cur.end( ); ci++)
                    if (ci->second > top->second)
                        top = ci;
                out[top->first] = top->second;
                const GAOnce *a = curArg[top->first];
                cur.erase( top );
                if (a == 0)
                    continue;
                bitstring_t nAff = a->aff & ~hAff, nImm = a->imm & ~hImm, nRes = a->res & ~hRes;
                if ((nAff | nImm | nRes) == 0)
                    continue;
                hAff |= nAff; hImm |= nImm; hRes |= nRes;
                // Only a slot whose contenders grant a newly claimed bit can change.
                // imm/res don't price bit by bit (an imm zeroes a res on the same element,
                // spell/magic/prayer are one group), so any new imm/res touches every
                // contender that carries one.
                bool resNew = (nImm | nRes) != 0;
                for (auto &ci: cur) {
                    bool touched = false;
                    for (auto &e: pool[ci.first])
                        if ((e.aff & nAff) || (resNew && (e.imm | e.res))) { touched = true; break; }
                    if (touched) {
                        const GAOnce *na;
                        ci.second = best( ci.first, wornOnly, na );
                        curArg[ci.first] = na;
                    }
                }
            }
        };
        std::map<int,double> ceilOnce;
        wornPct.clear( );
        claim( false, ceilOnce );
        claim( true, wornPct );
        w.heldFlags = savedFlags; w.heldImm = savedImm; w.heldRes = savedRes;

        // The greedy can misplace a flag and lose to the worn kit. Hybrid = the worn kit's
        // flag-once allocation on slots where some contender carries a bit, the ideal
        // on plain slots: still flag-once, achievable, and never below the worn kit, so
        // a plain-slot upgrade can't vanish from the %. The totals loop picks whichever
        // whole allocation sums higher, never a per-slot max (that could grant a flag
        // twice).
        // A bit slot also takes its best plain item when that beats the worn share:
        // dropping the worn item's bits can only lower other slots, so the hybrid stays
        // flag-once and a plain upgrade there still counts.
        for (auto &kv: slotCeil) {
            bool bits = false;
            double plain = 0;
            for (auto &e: pool[kv.first]) {
                if (e.aff | e.imm | e.res)
                    bits = true;
                else if (e.base > plain)
                    plain = e.base;
            }
            kv.second = ceilOnce[kv.first];
            hybridCeil[kv.first] = bits ? std::max( wornPct[kv.first], plain ) : ceilOnce[kv.first];
        }
    }
    // Known limits: the left-hand verdict (keepScore/dualScore) and the sc<=0 candidate
    // gate still price flags against worn gear outside the slot.

    // ---- Set awareness (perma-affects #2758 phase 3b) --------------------------
    // Value each data-scorable set's completion bonus (SetBehavior <affects>) with
    // the same scorer as items, decide which sets are worth assembling
    // (assemble = best obtainable members + V_set  >  break = best individual picks),
    // and fold that into the percentile ceiling (ideal-kit model), the chase list,
    // and set protection. On-demand path -- the extra passes over ~a dozen sets and
    // the candidate pool are cheap. V_set is 0 only for a truly data-empty set
    // (skills-only / full-Fenia: ashigaru, secret of sidhe, master scout). Since F3,
    // ga_score reads flags/res/slevel/ac too, so res/slevel sets like myrvale/shevale
    // now score in (they were 0 before F3). A worn set the scorer still can't value
    // stays protected and neutral in %.
    struct GASet {
        SetBehavior *sb;
        double vset;                   // completion bonus worth for this profile
        int    total;                  // props total_count (pieces needed)
        bool   dneck, dwrist;          // a doubled neck/wrist slot supplies 2 pieces
        std::map<int,double> memScore; // slot -> best available member score
        int    wornSlots;              // slots where the char already wears a member
        bool   wornComplete;           // char carries this set's completion affect
    };
    std::map<int,GASet> gaSets;        // behavior index -> aggregate (data-scorable)

    for (int bi = 0; bi < behaviorManager->size( ); bi++) {
        SetBehavior *sb = dynamic_cast<SetBehavior *>( behaviorManager->find( bi ) );
        if (sb == 0 || sb->affects.empty( ))
            continue;
        // Outleveled sets can't be completed -- eqset refuses assembly above max_level
        // ("Ты уже слишком опытен..."), so the sage must neither value nor chase them.
        if (target->getLevel( ) > ga_propInt( sb, "max_level", 999 ))
            continue;
        GASet g;
        g.sb = sb;
        // A set spans several slots and its bonus feeds both the worn and best ceilings,
        // so discount its flags only against permanent affects (gaPerma), never worn
        // gear -- otherwise a set's own member items would deflate its bonus (F1-style).
        w.heldFlags = gaPerma;
        g.vset = ga_setValue( target, sb, w, rawStat, capStat );
        g.total = ga_propInt( sb, "total_count", 999 );
        g.dneck = ga_propBool( sb, "double_neck", false );
        g.dwrist = ga_propBool( sb, "double_wrist", false );
        g.wornSlots = 0;
        g.wornComplete = false;
        gaSets[bi] = g;
    }

    // Complete sets the char wears now: each installs a "set <X>" completion affect,
    // whose skill name equals the set behavior's name. Resolve those to indices so
    // both data-scorable and data-empty worn sets can be detected and protected.
    std::map<int,int> wornSetIdx;
    for (auto &paf: target->affected) {
        Skill *sk = paf->type.getElement( );
        if (sk == 0 || sk->getName( ).find( "set " ) != 0)
            continue;
        Behavior *b = behaviorManager->findExisting( sk->getName( ) );
        if (b != 0)
            wornSetIdx[b->getIndex( )] = 1;
    }

    // Best obtainable member per slot from the candidate pool (not-worn, has a route,
    // not boss-capped -- the chase won't send the char after it).
    for (auto &c: cands) {
        if (c.acq.method == GA_UNKNOWN || ga_overCap( c.acq, chLevel ))
            continue;
        for (auto &kv: gaSets)
            if (c.pObj->behaviors.isSet( kv.first ) && c.score > kv.second.memScore[c.slot])
                kv.second.memScore[c.slot] = c.score;
    }
    // Worn members mark their slot filled and float their worn score in; worn
    // complete sets (data or data-empty) protect their slots from break-advice.
    int protectedSlots = lockedSlots;   // honor any legacy hint the caller still passes
    // Slots of a worn complete set whose bonus the scorer can't value (data-empty, or
    // a bonus made only of things ga_score ignores -- res/slevel/ac): kept OUT of the
    // percentile, exactly as the old lockedSlots did, so wearing such a set is neutral
    // and never a regression. Valued worn sets instead score in + earn wornSetBonus.
    int neutralSlots = 0;
    for (::Object *o = target->carrying; o; o = o->next_content) {
        if (o->wear_loc == wear_none)
            continue;
        int slot = o->pIndexData->wear_flags;
        REMOVE_BIT( slot, ITEM_TAKE );
        double sc = ga_score( target, o, w, rawStat, capStat, true );
        for (auto &kv: gaSets)
            if (o->pIndexData->behaviors.isSet( kv.first )) {
                kv.second.wornSlots |= slot;
                if (sc > kv.second.memScore[slot])
                    kv.second.memScore[slot] = sc;
            }
        for (auto &wk: wornSetIdx)
            if (o->pIndexData->behaviors.isSet( wk.first )) {
                protectedSlots |= slot;
                std::map<int,GASet>::iterator gi = gaSets.find( wk.first );
                if (gi == gaSets.end( ) || gi->second.vset <= 0)
                    neutralSlots |= slot;
                break;
            }
    }
    for (auto &kv: gaSets)
        kv.second.wornComplete = wornSetIdx.count( kv.first ) > 0;

    // Feasibility (hard: every needed piece obtainable) + worth-it decision, then a
    // greedy non-overlapping claim by descending margin for the ideal-kit ceiling.
    struct GAWorth { int bi; double margin; };
    std::vector<GAWorth> worth;
    double wornSetBonus = 0;
    for (auto &kv: gaSets) {
        GASet &g = kv.second;
        int fill = 0;
        double assemble = g.vset, brk = 0;
        for (auto &ms: g.memScore) {
            int mslot = ms.first;
            // Piece capacity of a slot type = how many equipment locations it fills,
            // matching .tmp.eqset.count: finger has two locations with NO dedup, a
            // weapon fills wield + second_wield (dual), neck/wrist have two locations
            // that dedup a same-vnum pair unless the double flag is set. (Assumes no
            // set has two distinct neck/wrist members without the flag -- true across
            // all 18 sets today; a future such set would undercount here.)
            int pieces = 1;
            if (mslot & ITEM_WEAR_FINGER)                   pieces = 2;
            else if (mslot & ITEM_WIELD)                    pieces = 2;
            else if ((mslot & ITEM_WEAR_NECK) && g.dneck)   pieces = 2;
            else if ((mslot & ITEM_WEAR_WRIST) && g.dwrist) pieces = 2;
            fill += pieces;
            // Value counts each slot type once (the percentile's own two-fingers-is-
            // Phase-2 limitation), while fill above counts locations for feasibility.
            assemble += ms.second;
            std::map<int,double>::iterator sc = legacyCeil.find( mslot );
            brk += (sc != legacyCeil.end( )) ? sc->second : 0.0;
        }
        bool feasible = fill >= g.total;
        if (feasible && g.wornComplete)
            wornSetBonus += g.vset;
        if (feasible && assemble > brk)
            worth.push_back( GAWorth{ kv.first, assemble - brk } );
    }
    std::sort( worth.begin( ), worth.end( ),
        []( const GAWorth &a, const GAWorth &b ){ return a.margin > b.margin; } );

    double bestSetBonus = 0;
    int claimedSetSlots = 0;
    std::map<int,int> claimedSets;
    for (auto &wsel: worth) {
        GASet &g = gaSets[wsel.bi];
        int occ = 0;
        for (auto &ms: g.memScore)
            occ |= ms.first;
        if (occ & claimedSetSlots) {    // this set overlaps a slot a higher-margin one took
            LogStream::sendNotice( ) << "gearAdvice: set '" << g.sb->getName( )
                << "' shares a slot with a higher-value set; scored conservatively." << endl;
            continue;                   // rare: two worth-it sets seldom overlap at once
        }
        claimedSetSlots |= occ;
        claimedSets[wsel.bi] = 1;
        bestSetBonus += g.vset;
        for (auto &ms: g.memScore) {    // a member may raise its slot above the individual best
            if (ms.second > slotCeil[ms.first])
                slotCeil[ms.first] = ms.second;
            if (ms.second > legacyCeil[ms.first])
                legacyCeil[ms.first] = ms.second;
            if (ms.second > hybridCeil[ms.first])
                hybridCeil[ms.first] = ms.second;
        }
    }
    // ---- end set awareness (folded into the percentile + chase below) ----------

    double wornTotal = 0, bestTotal = 0;
    // The ceiling is summed for both whole allocations (ideal, hybrid), the shield and
    // held-item share kept apart: the left-hand verdict below may replace it, so the
    // ideal/hybrid choice is made after the verdict, over what actually counts.
    double coreIdeal = 0, coreHybrid = 0, leftIdeal = 0, leftHybrid = 0;
    double leftWornSlots = 0, leftBestLegacy = 0;
    for (auto &kv: slotCeil) {
        if (neutralSlots & kv.first)   // unvaluable worn-set slot: neutral, out of the %
            continue;
        std::map<int,double>::iterator wi = wornPct.find( kv.first );
        double wv = (wi != wornPct.end( )) ? wi->second : 0.0;
        wornTotal += wv;
        if (kv.first & (ITEM_WEAR_SHIELD | ITEM_HOLD)) {
            leftIdeal      += kv.second;
            leftHybrid     += hybridCeil[kv.first];
            leftWornSlots  += wornSlot[kv.first];
            leftBestLegacy += legacyCeil[kv.first];
        } else {
            coreIdeal  += kv.second;
            coreHybrid += hybridCeil[kv.first];
        }
    }
    // Set bonuses: a complete worn set is real kit value; the ideal kit assembles the
    // best worth-it sets. bestSetBonus >= wornSetBonus in the common case (your set is
    // among the worth-it claimed), so the final clamp only bites on a mixed kit.
    wornTotal += wornSetBonus;
    bestTotal += bestSetBonus;

    // Paired armour slots (finger/neck/wrist) have two positions. The general chase
    // must (a) offer a FILL when a position is empty (baseline 0 -- a pure gain), and
    // (b) when both positions are full, measure a candidate against the WEAKER of the
    // two worn pieces (the one it would replace), not the stronger. Without (a) an
    // empty ring/wrist/neck yields no advice at all; without (b) a candidate that
    // beats the weaker piece but not the stronger is wrongly hidden. This mirrors the
    // slot-browse bar. (Percentile stays Phase 2: one count per slot-type, so none of
    // this shifts the %.)
    struct GAPaired { int count; double minScore; int minVnum; };
    GAPaired pairFinger = { 0, 0.0, 0 }, pairNeck = { 0, 0.0, 0 }, pairWrist = { 0, 0.0, 0 };
    for (::Object *o = target->carrying; o; o = o->next_content) {
        if (o->wear_loc == wear_none)
            continue;
        int ws = o->pIndexData->wear_flags;
        REMOVE_BIT( ws, ITEM_TAKE );
        if ((ws & (ITEM_WEAR_FINGER | ITEM_WEAR_NECK | ITEM_WEAR_WRIST)) == 0)
            continue;
        double osc = ga_score( target, o, w, rawStat, capStat, true );
        if (ws & ITEM_WEAR_FINGER) {
            if (pairFinger.count == 0 || osc < pairFinger.minScore) { pairFinger.minScore = osc; pairFinger.minVnum = o->pIndexData->vnum; }
            pairFinger.count++;
        }
        if (ws & ITEM_WEAR_NECK) {
            if (pairNeck.count == 0 || osc < pairNeck.minScore) { pairNeck.minScore = osc; pairNeck.minVnum = o->pIndexData->vnum; }
            pairNeck.count++;
        }
        if (ws & ITEM_WEAR_WRIST) {
            if (pairWrist.count == 0 || osc < pairWrist.minScore) { pairWrist.minScore = osc; pairWrist.minVnum = o->pIndexData->vnum; }
            pairWrist.count++;
        }
    }
    // Bar for a candidate's slot + the vnum a pick there would replace. Free paired
    // position -> 0 (fills, replaces nothing); both full -> weakest worn (replaces it);
    // any other slot -> best worn (unchanged single-slot behaviour, and replaceVnum
    // stays 0 so the render names the one worn piece via .tmp.advice.worn).
    auto gaBar = [&]( int slot, bool &fills, int &replaceVnum ) -> double {
        fills = false; replaceVnum = 0;
        GAPaired *p = 0;
        if (slot & ITEM_WEAR_FINGER)     p = &pairFinger;
        else if (slot & ITEM_WEAR_NECK)  p = &pairNeck;
        else if (slot & ITEM_WEAR_WRIST) p = &pairWrist;
        if (p == 0)
            return wornSlot[slot];
        if (p->count < 2) { fills = true; return 0.0; }
        replaceVnum = p->minVnum;
        return p->minScore;
    };

    // "optimal" = gap * obtainability (real upgrades only).
    for (auto &c: cands) {
        bool fills; int rv;
        double bar = gaBar( c.slot, fills, rv );
        double gap = c.score - bar;
        c.value = gap > 0 ? gap * c.obtain : 0;
    }
    // Nudge the chase toward completing a worth-it set: a member filling a slot the
    // char has not filled yet earns a share of the completion bonus, so "grab the
    // last set piece" outranks a marginal individual swap.
    for (auto &c: cands)
        for (auto &cs: claimedSets) {
            GASet &g = gaSets[cs.first];
            if (c.pObj->behaviors.isSet( cs.first ) && (g.wornSlots & c.slot) == 0)
                c.value += (g.vset / (g.total > 0 ? g.total : 1)) * c.obtain;
        }
    // General advice (not a slot browse): merge in the second-copy FILLS. A ring/
    // neck/wrist the char already wears can fill an empty twin position -- the same
    // upgrade "service advice <slot>" surfaces. Score it as a fill (bar 0, so value =
    // score * obtain, exactly what gaBar gives a free paired position) and let it
    // compete for that slot's single optimal pick, so a bigger second-<ring> win
    // beats a weaker plain replacement instead of being hidden by the worn-vnum skip.
    // Only free-position paired slots qualify; a both-full paired slot and dual-wield
    // off-hand stay slot-browse-only. secondCopyCands already dropped limit-1 and
    // unobtainable items.
    if (slotFilter == 0) {
        for (auto &c: secondCopyCands) {
            GAPaired *p = 0;
            if (c.slot & ITEM_WEAR_FINGER)     p = &pairFinger;
            else if (c.slot & ITEM_WEAR_NECK)  p = &pairNeck;
            else if (c.slot & ITEM_WEAR_WRIST) p = &pairWrist;
            if (p == 0 || p->count >= 2 || c.acq.method == GA_UNKNOWN)
                continue;
            c.value = c.score * c.obtain;   // bar 0: fills a free position
            cands.push_back( c );
        }
    }
    std::vector<GACand> byOpt = cands;
    std::sort( byOpt.begin( ), byOpt.end( ),
        []( const GACand &a, const GACand &b ){ return a.value > b.value; } );

    // "optimal" = the practical upgrades (gap * obtainability): the highest-value
    // item PER wear-slot-type (never two of the same slot), top 5 slots. This is now
    // the ONLY chase list -- one best-obtainable pick per slot, no dream list beside it.
    // Path-cost start point, cached per destination room.
    Room *msm = get_room_instance( GA_START_ROOM );
    std::map<int, std::vector<int> > pathCache;   // destVnum -> [aggros, doors, fly]

    // ---- Second weapon or shield? ---------------------------------------------------
    // The general chase treats the weapon slot as one position and the slot browse only
    // offers an off-hand when the left hand is already free, so neither ever says whether
    // a shield build should go dual. This weighs the two builds, best of each (worn or
    // obtainable), in per-main-swing score units:
    //   dual = best off-hand weapon (ga_offhandValue) + the no-shield +5% on main-hand dice
    //          (only when the other build has a shield) + cross block, if the char has it;
    //   keep = best shield + best held item + shield block.
    // A blocked enemy blow is worth one of the char's own blows (the COMBAT_PROC_SCORING
    // rule for heals): an equal-level enemy swings about as often as the char, so per
    // main-hand swing the block is worth pBlock * blowValue.
    // dualInfo = [state(0 n/a / 1 not yet available / 2 verdict), unlockLevel, dualScore,
    //             keepScore, entry(pick entry for a better off-hand weapon, or null)].
    Wearlocation *gaWieldLoc  = wearlocationManager->findExisting( "wield" );
    Wearlocation *gaOffLoc    = wearlocationManager->findExisting( "second_wield" );
    Wearlocation *gaShieldLoc = wearlocationManager->findExisting( "shield" );
    Wearlocation *gaHoldLoc   = wearlocationManager->findExisting( "hold" );
    ::Object *gaPrimary = gaWieldLoc  ? gaWieldLoc->find( target )  : 0;
    ::Object *gaOffhand = gaOffLoc    ? gaOffLoc->find( target )    : 0;
    ::Object *gaShield  = gaShieldLoc ? gaShieldLoc->find( target ) : 0;
    ::Object *gaHold    = gaHoldLoc   ? gaHoldLoc->find( target )   : 0;
    bool gaHuge = target->getRace( )->getSize( ) >= SIZE_HUGE;
    GAOffhand om = ga_offhandModel( target, w, gaPrimary );

    // Redundancy set for an off-hand weapon: everything worn stays except what the left
    // hand gives up (shield, held item).
    bitstring_t offHeld = gaPerma;
    for (std::map<int,bitstring_t>::iterator wi = wornFlagsBySlot.begin( ); wi != wornFlagsBySlot.end( ); wi++)
        if ((wi->first & (ITEM_WEAR_SHIELD | ITEM_HOLD)) == 0)
            offHeld |= wi->second;
    int offCap = wield_weight_cap( target, true );
    // Can this weapon prototype go in the off-hand? Two-hander rule and the off-hand
    // weight cap (a player gets no native-weapon exemption, see too_heavy_to_wield).
    auto offhandFits = [&]( obj_index_data *p ) -> bool {
        return p->item_type == ITEM_WEAPON && IS_SET( p->wear_flags, ITEM_WIELD )
            && !( IS_SET( p->value[4], WEAPON_TWO_HANDS ) && !gaHuge )
            && p->weight <= offCap;
    };
    auto offhandValueProto = [&]( obj_index_data *p ) -> double {
        bitstring_t saved = w.heldFlags;
        w.heldFlags = offHeld;
        double sc = ga_score( target, p, w, rawStat, capStat, false );
        w.heldFlags = saved;
        return ga_offhandValue( om, target, w, sc, get_weapon_sn( p ), weapon_ave( p ),
                                ga_clericCanCompound( target, p ), p->value[0] );
    };
    auto offhandValueWorn = [&]( ::Object *o ) -> double {
        double sc = ga_score( target, o, w, rawStat, capStat, true );
        return ga_offhandValue( om, target, w, sc, get_weapon_sn( o ), weapon_ave( o ),
                                ga_clericCanCompound( target, o ), get_weapon_class( o ) );
    };
    double offWornVal = gaOffhand != 0 ? offhandValueWorn( gaOffhand ) : 0.0;
    // Same reach rule as the chase list: no boss-guarded kill/pickup routes.
    auto reachable = [&]( const GACand &c ) -> bool {
        if (c.acq.method == GA_UNKNOWN)
            return false;
        if (ga_overCap( c.acq, chLevel ))
            return false;
        return true;
    };

    RegList::Pointer dualInfo( NEW );
    // Set by a left-hand verdict: the chase list must not push the losing build, and the
    // percentile scores the left hand as one build instead of shield + hold slots that a
    // dual-wielder leaves empty on purpose.
    bool gaGoDual = false, gaLeftScored = false;
    double gaLeftWorn = 0, gaLeftDual = 0, gaLeftKeepNet = 0;
    {
        int dState = 0, dUnlock = 0;
        double dualScore = 0, keepScore = 0;
        Register dEntry;
        Skill *swSkill = skillManager->findExisting( "second weapon" );
        if (swSkill != 0 && !swSkill->usable( target, false )) {
            // Not yet: the class learns it later. Shown only to a char who will get it.
            int lvl = swSkill->getLevel( target );
            if (lvl > target->getRealLevel( ) && lvl <= LEVEL_MORTAL) {
                dState = 1;
                dUnlock = lvl;
            }
        }
        else if (swSkill != 0 && ga_dualBody( target )
                 && gaPrimary != 0 && gaPrimary->item_type == ITEM_WEAPON
                 && !ga_twoHandBlocks( target, IS_WEAPON_STAT( gaPrimary, WEAPON_TWO_HANDS ) )) {
            // Best off-hand weapon: carried, obtainable, or a second copy of a worn one.
            const GACand *bestOff = 0;
            double bestOffVal = 0;
            for (int pass = 0; pass < 2; pass++) {
                std::vector<GACand> &pool = pass == 0 ? cands : secondCopyCands;
                for (auto &c: pool) {
                    if (!offhandFits( c.pObj ) || !reachable( c ))
                        continue;
                    double v = offhandValueProto( c.pObj );
                    if (bestOff == 0 || v > bestOffVal) { bestOff = &c; bestOffVal = v; }
                }
            }
            bool pickBetter = bestOff != 0 && bestOffVal > offWornVal;
            if (pickBetter || gaOffhand != 0) {
                // Keep side: the best shield and the best held item, worn or obtainable.
                double shieldVal = gaShield ? ga_score( target, gaShield, w, rawStat, capStat, true ) : 0;
                double holdVal   = gaHold   ? ga_score( target, gaHold,   w, rawStat, capStat, true ) : 0;
                bool haveShield = gaShield != 0;
                for (auto &c: cands) {
                    if (!reachable( c ))
                        continue;
                    if ((c.slot & ITEM_WEAR_SHIELD) && c.score > shieldVal) { shieldVal = c.score; haveShield = true; }
                    if ((c.slot & ITEM_HOLD) && c.score > holdVal) holdVal = c.score;
                }
                keepScore = shieldVal + holdVal;
                if (haveShield)
                    keepScore += om.pShield * om.blowValue;

                double dualExtra = om.pCross * om.blowValue;
                if (haveShield)
                    dualExtra += w.weaponWeight * 0.05 * om.mainEff;
                dualScore = (pickBetter ? bestOffVal : offWornVal) + dualExtra;

                // What the left hand holds now, in the same units as the two builds.
                if (gaOffhand != 0)
                    gaLeftWorn = offWornVal + dualExtra;
                else {
                    gaLeftWorn = (gaShield ? ga_score( target, gaShield, w, rawStat, capStat, true ) : 0)
                               + (gaHold   ? ga_score( target, gaHold,   w, rawStat, capStat, true ) : 0);
                    if (gaShield)
                        gaLeftWorn += om.pShield * om.blowValue;
                }
                // Best left-hand share: a dual build replaces the shield + hold share
                // outright, a kept build is full-price per slot, so it is netted against
                // the legacy share and added to the allocation's own. Taking the max of
                // the two keeps the ceiling continuous where the keep/dual verdict flips.
                gaLeftDual = dualScore;
                gaLeftKeepNet = keepScore - leftBestLegacy;
                gaLeftScored = true;
                // Same rounded values the dual line prints, so the list and the verdict agree.
                gaGoDual = (int)(dualScore + 0.5) > (int)(keepScore + 0.5);

                if (pickBetter) {
                    GACand pick = *bestOff;
                    int rv = gaOffhand != 0 ? gaOffhand->pIndexData->vnum : 0;
                    dEntry = ga_buildEntry( pick, msm, chLevel, isVampire, pathCache,
                                            bestOffVal - offWornVal, gaOffhand == 0, rv );
                }
                dState = 2;
            }
        }
        dualInfo->push_back( Register( dState ) );
        dualInfo->push_back( Register( dUnlock ) );
        dualInfo->push_back( Register( (int)(dualScore + 0.5) ) );
        dualInfo->push_back( Register( (int)(keepScore + 0.5) ) );
        dualInfo->push_back( dEntry );
    }

    if (gaLeftScored)
        wornTotal += gaLeftWorn - leftWornSlots;
    // Ideal vs hybrid: whichever whole allocation sums higher, left hand included, never
    // a per-slot max (that could grant a flag twice).
    auto leftShare = [&]( double own ) -> double {
        return gaLeftScored ? std::max( gaLeftDual, own + gaLeftKeepNet ) : own;
    };
    bestTotal += std::max( coreIdeal + leftShare( leftIdeal ), coreHybrid + leftShare( leftHybrid ) );
    int pct = 0;
    if (bestTotal > 0)
        pct = (int)( 100.0 * wornTotal / bestTotal + 0.5 );
    if (pct > 100) pct = 100;   // set slots relax the per-slot cap; this is the backstop
    if (pct < 0)   pct = 0;     // a kit full of cursed maledictions can sum negative

    // Slot-browse mode: the char asked for one wear slot ("service advice neck").
    // Return the top-5 wearable-now items in that slot by raw score (Fenia orders
    // them by difficulty band). No percentile, no dream list; the caller passes
    // lockedSlots=0 so an explicit slot browse is never suppressed by a set.
    if (slotFilter != 0) {
        bool wieldBrowse = (slotFilter & ITEM_WIELD) != 0;
        bool lightBrowse = (slotFilter == GA_SLOT_LIGHT);
        // Worn scores in this slot + its position count. Most slots read the prototype's
        // wear flag, but the WIELD slot must use the ACTUAL wear locations: an arrow stuck
        // in the char or a sheathed weapon carries the "wield" flag yet holds no hand, so a
        // proto-flag count would falsely read the off-hand as occupied. Finger/neck/wrist
        // have two positions; wield has two for a dual-wielder with a free off-hand.
        std::vector<double> wornScores;
        int capacity = 1;
        int wornMinVnum = 0;      // vnum of the weakest worn piece in this slot (paired/dual)
        double wornMinScore = 0;  // its score -- the piece a full-slot pick would replace
        if (wieldBrowse) {
            Wearlocation *wieldLoc = wearlocationManager->findExisting( "wield" );
            Wearlocation *offLoc   = wearlocationManager->findExisting( "second_wield" );
            ::Object *primary = wieldLoc ? wieldLoc->find( target ) : 0;
            ::Object *offhand = offLoc   ? offLoc->find( target )   : 0;
            if (primary != 0) {
                double psc = ga_score( target, primary, w, rawStat, capStat, true );
                wornScores.push_back( psc );
                if (wornMinVnum == 0 || psc < wornMinScore) { wornMinScore = psc; wornMinVnum = primary->pIndexData->vnum; }
            }
            if (offhand != 0) {
                double osc = ga_score( target, offhand, w, rawStat, capStat, true );
                wornScores.push_back( osc );
                if (wornMinVnum == 0 || osc < wornMinScore) { wornMinScore = osc; wornMinVnum = offhand->pIndexData->vnum; }
            }
            if (ga_canDualWield( target ))
                capacity = 2;
        } else if (lightBrowse) {
            // Light items carry no wear_flags bit -- match on item_type. One light slot.
            for (::Object *o = target->carrying; o; o = o->next_content) {
                if (o->wear_loc == wear_none)
                    continue;
                if (o->pIndexData->item_type != ITEM_LIGHT)
                    continue;
                wornScores.push_back( ga_score( target, o, w, rawStat, capStat, true ) );
            }
        } else {
            for (::Object *o = target->carrying; o; o = o->next_content) {
                if (o->wear_loc == wear_none)
                    continue;
                int ws = o->pIndexData->wear_flags;
                REMOVE_BIT( ws, ITEM_TAKE );
                if ((ws & slotFilter) == 0)
                    continue;
                double osc = ga_score( target, o, w, rawStat, capStat, true );
                wornScores.push_back( osc );
                if (wornMinVnum == 0 || osc < wornMinScore) { wornMinScore = osc; wornMinVnum = o->pIndexData->vnum; }
            }
            if ((slotFilter & ITEM_WEAR_FINGER) || (slotFilter & ITEM_WEAR_NECK) || (slotFilter & ITEM_WEAR_WRIST))
                capacity = 2;
        }

        // Dual-wield weapon browse: each weapon is offered where it does the most good --
        // as the new MAIN weapon (gain over the worn primary; a non-giant's two-hander also
        // drops the off-hand weapon) or in the OFF hand, valued at its real swing ratio
        // (ga_offhandValue) instead of as a second primary. An off-hand fill used to show
        // its whole primary score as the gain, 2-3x what the off-hand actually swings.
        if (wieldBrowse && capacity >= 2) {
            double psc = gaPrimary != 0 ? ga_score( target, gaPrimary, w, rawStat, capStat, true ) : 0.0;
            struct GAWieldOpt { GACand c; double gain; bool fills; int rv; };
            std::vector<GAWieldOpt> opts;
            for (int pass = 0; pass < 2; pass++) {
                // pass 1 = second copies of a worn weapon: they can only join it, off-hand.
                std::vector<GACand> &pool = pass == 0 ? cands : secondCopyCands;
                for (auto &c: pool) {
                    if ((c.slot & ITEM_WIELD) == 0 || c.acq.method == GA_UNKNOWN)
                        continue;
                    bool any = false, fills = false;
                    double best = 0;
                    int rv = 0;
                    if (pass == 0) {
                        bool blocks = ga_twoHandBlocks( target, IS_SET( ga_candValue4( c ), WEAPON_TWO_HANDS ) );
                        best = c.score - psc - (blocks ? offWornVal : 0.0);
                        any = true;
                        fills = gaPrimary == 0;
                        rv = gaPrimary != 0 ? gaPrimary->pIndexData->vnum : 0;
                    }
                    if (gaPrimary != 0 && offhandFits( c.pObj )) {
                        double g = offhandValueProto( c.pObj ) - offWornVal;
                        if (!any || g > best) {
                            best = g;
                            any = true;
                            fills = gaOffhand == 0;
                            rv = gaOffhand != 0 ? gaOffhand->pIndexData->vnum : 0;
                        }
                    }
                    // Same bars as the generic path: >= for a new item, strict > for a
                    // second copy (never "replace your X with an X").
                    if (!any || (pass == 0 ? best < 0 : best <= 0))
                        continue;
                    opts.push_back( GAWieldOpt{ c, best, fills, rv } );
                }
            }
            std::sort( opts.begin( ), opts.end( ),
                []( const GAWieldOpt &a, const GAWieldOpt &b ){ return a.gain > b.gain; } );
            RegList::Pointer slotList( NEW );
            for (size_t k = 0; k < opts.size( ) && k < 5; k++)
                slotList->push_back( ga_buildEntry( opts[k].c, msm, chLevel, isVampire, pathCache,
                                                    opts[k].gain, opts[k].fills, opts[k].rv ) );
            RegList::Pointer emptyBest( NEW );
            RegList::Pointer result( NEW );
            result->push_back( Register( 0 ) );
            result->push_back( wrap( slotList ) );
            result->push_back( wrap( emptyBest ) );
            result->push_back( wrap( dualInfo ) );
            return wrap( result );
        }

        // The bar a candidate must clear. Multi-position slot (capacity 2): an empty
        // position means bar 0 (add with no loss), both filled means beat the WEAKEST worn
        // piece. Single-position slots keep the old max bar exactly. primaryScore (best
        // worn) is the separate bar a two-handed weapon must clear: it OUSTS the one-hander
        // instead of joining it, so it is a replacement even when the off-hand is free.
        double wornScore = 0, primaryScore = 0;
        for (size_t i = 0; i < wornScores.size( ); i++)
            if (wornScores[i] > primaryScore) primaryScore = wornScores[i];
        if (capacity >= 2) {
            if ((int)wornScores.size( ) >= capacity)
                wornScore = *std::min_element( wornScores.begin( ), wornScores.end( ) );
        } else {
            wornScore = primaryScore;
        }
        bool freePos = (int)wornScores.size( ) < capacity;
        // Both positions of a capacity-2 slot (paired ring/neck/wrist, or a dual-wield pair)
        // are full: a one-handed / armour pick OUSTS the WEAKER of the two worn pieces -- name
        // it via replaceVnum so the render doesn't grab whichever the slot lookup finds first.
        // 0 elsewhere (a fill, or a single-worn slot the render can label on its own). A
        // two-handed pick is excluded per-entry below: it replaces the primary, not the
        // weaker. Mirrors gaBar's paired logic, now covering the wield pair too.
        int slotReplaceVnum = (capacity >= 2 && !freePos) ? wornMinVnum : 0;

        std::vector<GACand> slotCands;
        for (auto &c: cands) {
            bool inSlot = lightBrowse ? (c.pObj->item_type == ITEM_LIGHT)
                                      : ((c.slot & slotFilter) != 0);
            if (!inSlot || c.acq.method == GA_UNKNOWN)
                continue;
            // A two-handed weapon can never take the off-hand (SecondWieldWearloc refuses
            // it) -- it replaces the primary, so it must clear the primary's score, not the
            // free-slot bar.
            double bar = wornScore;
            if (wieldBrowse && IS_SET( ga_candValue4( c ), WEAPON_TWO_HANDS ))
                bar = primaryScore;
            if (c.score >= bar)
                slotCands.push_back( c );
        }
        // A multi-position slot (paired ring/neck/wrist, or a dual-wield off-hand) can take
        // a SECOND copy of a worn item -- as a fill when a position is empty, or, when both
        // are full, as a replacement of the weaker worn piece (a second laerkai power
        // ousting a lesser bracelet). Same bar as any pick; single-capacity slots never
        // offer a duplicate. Second copies are never two-handed (a two-handed primary
        // blocks dual capacity).
        // Strict > (not >=): every second copy scores > 0 (main-loop gate), so a fill of an
        // empty position (wornScore 0) still passes, but a full slot only takes a copy that
        // truly beats the weaker worn piece -- never "replace your laerkai with a laerkai".
        if (capacity >= 2)
            for (auto &c: secondCopyCands)
                if ((c.slot & slotFilter) && c.acq.method != GA_UNKNOWN && c.score > wornScore)
                    slotCands.push_back( c );
        std::sort( slotCands.begin( ), slotCands.end( ),
            []( const GACand &a, const GACand &b ){ return a.score > b.score; } );

        RegList::Pointer slotList( NEW );
        for (size_t k = 0; k < slotCands.size( ) && k < 5; k++) {
            // A two-handed weapon replaces the primary rather than filling the off-hand:
            // show it as a swap (fillsFree 0), gain over the primary, not the fill bar.
            bool twoHand = wieldBrowse && IS_SET( ga_candValue4( slotCands[k] ), WEAPON_TWO_HANDS );
            bool fills = freePos && !twoHand;
            double gain = twoHand ? (slotCands[k].score - primaryScore) : (slotCands[k].score - wornScore);
            // slotReplaceVnum is 0 for fills and single-worn slots; a two-handed pick ousts the
            // PRIMARY, not the weaker, so it keeps the render's own slot lookup (rv 0).
            int rv = twoHand ? 0 : slotReplaceVnum;
            slotList->push_back( ga_buildEntry( slotCands[k], msm, chLevel, isVampire, pathCache, gain, fills, rv ) );
        }

        RegList::Pointer emptyBest( NEW );
        RegList::Pointer result( NEW );
        result->push_back( Register( 0 ) );
        result->push_back( wrap( slotList ) );
        result->push_back( wrap( emptyBest ) );
        result->push_back( wrap( dualInfo ) );
        return wrap( result );
    }

    RegList::Pointer optimal( NEW );
    std::map<int,double> optSlot;   // slot-type -> raw score of its optimal pick
    int nOpt = 0;
    for (size_t k = 0; k < byOpt.size( ) && nOpt < 5; k++) {
        if (byOpt[k].value <= 0) break;
        // Slot held by a complete set the char wears: don't advise breaking it (a
        // set bonus the raw score can't see). Replaces the old Fenia lockedSlots skip.
        if (protectedSlots & byOpt[k].slot)
            continue;
        // No known route (not a quest reward, no reset/shop source): can't tell the
        // char how to get it, so never put it on the chase list. Fenia-triggered gear
        // gets a flat score boost, which can float an unobtainable downgrade up here.
        if (byOpt[k].acq.method == GA_UNKNOWN)
            continue;
        // Boss cap (OPTIMAL list only -- the finest/dream list still shows these):
        // don't tell the char to chase gear whose easiest route is killing/looting
        // more than 10 levels above them. Buy/quest have no such guard.
        if (ga_overCap( byOpt[k].acq, chLevel ))
            continue;
        // The verdict says go dual: a shield or held item would take the off-hand back.
        if (gaGoDual && (byOpt[k].slot & (ITEM_WEAR_SHIELD | ITEM_HOLD)))
            continue;
        if (optSlot.count( byOpt[k].slot )) continue;   // one item per slot-type
        optSlot[byOpt[k].slot] = byOpt[k].score;
        bool optFills; int optReplace;
        double optBar = gaBar( byOpt[k].slot, optFills, optReplace );
        double optGain = byOpt[k].score - optBar;
        optimal->push_back( ga_buildEntry( byOpt[k], msm, chLevel, isVampire, pathCache, optGain, optFills, optReplace ) );
        nOpt++;
    }

    // The "finest / dream" list is retired. It named a SECOND pick per slot -- the
    // raw-best item even when boss-gated or limited -- which read as two conflicting
    // recommendations for one wear slot (a cap in the chase list, a helmet in the
    // dream list). The sage now names ONE best-obtainable pick per slot -- the chase
    // list above -- and nothing beside it. `best` stays in the return for shape only
    // (always empty, like the slot-browse path); the renderer omits an empty list.
    RegList::Pointer best( NEW );

    RegList::Pointer result( NEW );
    result->push_back( Register( pct ) );
    result->push_back( wrap( optimal ) );
    result->push_back( wrap( best ) );
    result->push_back( wrap( dualInfo ) );
    return wrap( result );
}

NMI_INVOKE(CharacterWrapper, can_drop_obj, "(obj): может ли избавиться от предмета obj в инвентаре" )
{
    checkTarget( );
    ::Object *obj = arg2item( get_unique_arg( args ) );
    return Item::canDrop(target, obj, false);
}

NMI_INVOKE( CharacterWrapper, mortality, "(): включает-выключает бессмертие для кодеров" )
{
    checkTarget( );

    if (target->is_npc( ) || !target->getPC( )->getAttributes( ).isAvailable( "mortality" ))
        throw Scripting::Exception( "Attribute not found" );
    
    if (target->getPC( )->getAttributes( ).isAvailable( "coder" )) {
        target->getPC( )->getAttributes( ).eraseAttribute( "coder" );
        target->getPC( )->setSecurity( 0 );
        target->pecho("Now you are mortal.");
        return 1;
    }
    else {
        target->getPC( )->getAttributes( ).getAttr<XMLAttributeCoder>( "coder" );
        target->getPC( )->setSecurity( 999 );
        target->pecho("Now you are immortal.");
        return 0;
    }
}

NMI_INVOKE( CharacterWrapper, echoOn, "(): включает отображение введенного текста в терминале" )
{
    checkTarget( );
    
    if (target->desc)
        target->desc->echoOn( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, echoOff, "(): выключает отображение введенного текста в терминале" )
{
    checkTarget( );
    
    if (target->desc)
        target->desc->echoOff( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, save, "(): сохранить профайл на диск" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->save( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, skills, "([origin[,category]]): список названий доступных скилов, всех или с данным происхождением (.tables.skill_origin_table) и категориями (.tables.skill_category_flags)" )
{
    checkTarget();
    CHK_NPC

    RegList::Pointer list(NEW);
    int origin = args.size() > 0 ?   argnum2flag(args, 1, skill_origin_table) : NO_FLAG;
    Bitstring category = args.size() > 1 ? argnum2flag(args, 2, skill_category_flags) : NO_FLAG;
    
    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        if (!skill->available(target))
            continue;

        PCSkillData &data = target->getPC()->getSkillData(sn);

        // Filter by requested origin (fenia, religion, etc) or return all.
        if (origin != NO_FLAG && data.origin != origin)
            continue;

        // Filter by requested category, or all.
        if (category != NO_FLAG && !category.isSet(skill->getCategory()))
            continue;

        list->push_back(Register(skill->getName()));
    }

    return ::wrap(list);
}

NMI_INVOKE( CharacterWrapper, skillLookup, "(arg): нестрогий поиск скила по имени; сперва ищет среди доступных; возвращает его англ имя или null" )
{
    checkTarget();
    DLString skillArg = args2string(args);
    int skillIndex = skill_lookup(skillArg, target);

    if (skillIndex == -1)
        return Register();

    Skill *skill = skillManager->find(skillIndex);
    return skill->getName();
}

NMI_INVOKE( CharacterWrapper, skillsInfo, "(): список структур для доступных скилов")
{
    checkTarget();
    CHK_NPC

    RegList::Pointer list(NEW);
    PCharacter *pch = target->getPC();
    
    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        if (!skill->available(target))
            continue;

        Spell::Pointer spell = skill->getSpell();    
        PCSkillData &data = pch->getSkillData(sn);
        Register infoReg = Register::handler<IdContainer>();
        IdContainer *info = infoReg.toHandler().getDynamicPointer<IdContainer>();
        bool isSpell = spell && spell->isCasted();
        bool isPrayer = spell && spell->isCasted() && spell->isPrayer(pch);
        bool isPassive = skill->isPassive();
        bool isActive = !isSpell && !isPassive;
        const DLString &cmdName = skill->getCommand() ? skill->getCommand()->getNameFor(pch) : "";

        info->setField(IdRef("learned"), data.learned.getValue());
        info->setField(IdRef("effective"), skill->getEffective(pch));
        info->setField(IdRef("spell"), isSpell);
        info->setField(IdRef("prayer"), isPrayer);
        info->setField(IdRef("passive"), isPassive);
        info->setField(IdRef("active"), isActive);
        info->setField(IdRef("cmdname"), cmdName);
        info->setField(IdRef("name"), skill->getNameFor( pch ).ruscase('1'));
        // Canonical (English) name for stable .Skill() lookups from Fenia: the
        // localized `name` above can't be resolved back for language skills whose
        // Russian names aren't indexed in the skill matcher (breaks prac lists in
        // the RU locale). Consumers should look skills up by `sysname`, not `name`.
        info->setField(IdRef("sysname"), skill->getName());
        info->setField(IdRef("adept"), skill->getAdept( pch ));
        info->setField(IdRef("maximum"), skill->getMaximum(pch));
        info->setField(IdRef("origin"), data.origin.getValue());
        info->setField(IdRef("category"), skill->getCategory());
        info->setField(IdRef("groups"), skill->getGroups().toString());
        info->setField(IdRef("autobuff"), skill->isAutobuff());

        if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
            info->setField(IdRef("help_id"), skill->getSkillHelp()->getID());
        else
            info->setField(IdRef("help_id"), 0);

        list->push_back(infoReg);
    }

    return ::wrap(list);
}

// Weapon/utility skills that make no sense in a weaponless pet's report.
// Mirrors skill_is_invalid() in comm/report.cpp.
static bool report_skill_invalid(int sn, bool noCarry)
{
    if (!noCarry)
        return false;
    return sn == gsn_lash || sn == gsn_bash || sn == gsn_hand_to_hand
        || sn == gsn_sword || sn == gsn_polearm || sn == gsn_dagger
        || sn == gsn_whip || sn == gsn_grip || sn == gsn_axe
        || sn == gsn_mace || sn == gsn_shield_block || sn == gsn_flail
        || sn == gsn_slice || sn == gsn_second_weapon || sn == gsn_pick_lock;
}

// Skills that can never be ordered while the pet is fighting.
// Mirrors skill_is_invalid_in_fight() in comm/report.cpp.
static bool report_skill_invalid_in_fight(int sn)
{
    return sn == gsn_recall || sn == gsn_concentrate || sn == gsn_hide
        || sn == gsn_sneak || sn == gsn_detect_hide || sn == gsn_pick_lock
        || sn == gsn_bash_door;
}

NMI_INVOKE( CharacterWrapper, reportSkills, "(): для очарованного NPC -- список структур приказываемых скилов для рапорта. Поля: bucket (0 внебоевая команда, 1 боевая, 2 заклинание, 3 пассив), help_id, cmdname (имя для хозяина), sysname (англ. имя скила), showall_only (показывать только в 'рапорт все')" )
{
    checkTarget();

    RegList::Pointer list(NEW);

    NPCharacter *pet = target->getNPC();
    if (!pet || pet->master == 0)
        return ::wrap(list);
    Character *master = pet->master;

    bool noCarry = Char::canCarryNumber(pet) == 0 || !pet->wearloc.isSet(wear_wield);
    // properOrder inspects the pet's fighting flag to test fight-context
    // orderability; toggle it around the check and restore, never clobber.
    Character *savedFighting = pet->fighting;

    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        Spell::Pointer spell = skill->getSpell();
        Command::Pointer cmd = skill->getCommand().getDynamicPointer<Command>();
        bool passive = skill->isPassive();
        int bucket = -1;
        bool showallOnly = false;

        if (!skill->usable(pet, false))
            continue;

        if (cmd && !cmd->getExtra().isSet(CMD_NO_INTERPRET)) {
            bool canOrder = cmd->properOrder(pet) == RC_ORDER_OK;
            pet->fighting = pet;
            bool canOrderFight = (cmd->properOrder(pet) == RC_ORDER_OK) && !report_skill_invalid_in_fight(sn);
            pet->fighting = savedFighting;

            if (report_skill_invalid(sn, noCarry))
                continue;

            if (sn == gsn_second_weapon) {
                if (!(canOrder && pet->wearloc.isSet(wear_second_wield)))
                    continue;
                bucket = 0;
            }
            else if (canOrderFight) {
                bucket = 1;
            }
            else if (canOrder) {
                bucket = 0;
                showallOnly = true;
            }
            else
                continue;
        }
        else if (spell && spell->isCasted()) {
            if (!spell->properOrder(pet))
                continue;
            bucket = 2;
            showallOnly = !IS_SET(spell->getTarget(), TAR_CHAR_ROOM);
        }
        else if (passive && !report_skill_invalid(sn, noCarry)) {
            bucket = 3;
            showallOnly = true;
        }
        else
            continue;

        Register infoReg = Register::handler<IdContainer>();
        IdContainer *info = infoReg.toHandler().getDynamicPointer<IdContainer>();
        info->setField(IdRef("bucket"), bucket);
        int hid = (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
                    ? skill->getSkillHelp()->getID() : 0;
        info->setField(IdRef("help_id"), hid);
        // getNameFor lives on SkillCommand (skill->getCommand()), not on the
        // Command base we down-cast to for properOrder/getExtra.
        DLString dname = (bucket <= 1 && skill->getCommand())
                            ? skill->getCommand()->getNameFor(master)
                            : skill->getNameFor(master);
        info->setField(IdRef("cmdname"), dname);
        info->setField(IdRef("sysname"), skill->getName());
        info->setField(IdRef("showall_only"), showallOnly);
        list->push_back(infoReg);
    }

    return ::wrap(list);
}

NMI_INVOKE( CharacterWrapper, updateSkills, "(): освежить разученность умений (при входе в мир)" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->updateSkills( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, hasAttribute, "(attr): true если установлен аттрибут с именем attr" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getAttributes( ).isAvailable( args2string( args ) );
}

NMI_INVOKE( CharacterWrapper, eraseAttribute, "(attr): удаляет аттрибут с именем attr" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->getAttributes( ).eraseAttribute( args2string( args ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, visible, "(): проявиться из невидимости" )
{
    checkTarget();
    do_visible(target);
    return Register();
}

NMI_INVOKE( CharacterWrapper, get_eq_char, "(wearloc): предмет экипировки, надетый на эту локацию" )
{
    checkTarget( );
    return wrap( arg2wearloc( get_unique_arg( args ) )->find( target ) );
}

NMI_INVOKE( CharacterWrapper, pickUp, "(obj): поднять с земли именно этот предмет обычной командой get (со всеми проверками и тригерами); true если он теперь у нас" )
{
    checkTarget( );
    ::Object *obj = argnum2item( args, 1 );

    if (!target->in_room || obj->in_room != target->in_room)
        return Register( false );

    ::interpret_raw( target, "get", "%lld", obj->getID( ) );
    return Register( !obj->extracted && obj->carried_by == target );
}

NMI_INVOKE( CharacterWrapper, canWearAt, "(obj, wearloc): молча проверить, можно ли надеть obj на локацию wearloc: подходит ли предмет, есть ли у тела такое место, хватает ли уровня и сил. Занятость локации не проверяется" )
{
    checkTarget( );
    ::Object *obj = argnum2item( args, 1 );
    Wearlocation *loc = arg2wearloc( argnum( args, 2 ) );

    if (!loc->matches( obj ))
        return Register( false );

    return Register( loc->canWear( target, obj, 0 ) == RC_WEAR_OK );
}

NMI_INVOKE( CharacterWrapper, hasWearloc, "(wearloc): обладает ли данным слотом в экипировке")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getWearloc( ).isSet( 
                 arg2wearloc( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, add_charmed, "(victim,time): очаровать victim на время time и добавить нам в последователи" )
{
    Character *victim;
    int duration;
    Affect af;
    RegisterList::const_iterator i;
    
    checkTarget( );
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    i = args.begin( );
    victim = arg2character( *i++ );
    duration = (i == args.end( ) ? -1 : i->toNumber( ));

    if (victim->master)
        follower_stop(victim);

    follower_add(victim, target);
    victim->leader = target;
    
    af.bitvector.setTable(&affect_flags);
    af.type      = gsn_charm_person;
    af.level     = target->getRealLevel( );
    af.duration  = duration;
    af.bitvector.setValue(AFF_CHARM);
    affect_to_char( victim, &af );

    // Ensure that when charm is off, default mobile AI won't be active for a while.
    if (victim->is_npc() && victim->getNPC()->behavior) {
        BasicMobileBehavior::Pointer bhv = victim->getNPC()->behavior.getDynamicPointer<BasicMobileBehavior>();
        if (bhv)
            bhv->setLastCharmTime();
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, add_pet, "(pet): добавить пета нам в последователи" )
{
    Character *pet;

    checkTarget( );
    CHK_NPC

    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );
    
    pet = arg2character( args.front( ) );
    if (!pet->is_npc( ))
        throw Scripting::Exception( "NPC field requested on PC" ); 
    
    if (pet->getNPC( )->behavior) {
        Pet::Pointer bhv = pet->getNPC( )->behavior.getDynamicPointer<Pet>( );
        if (bhv)
            bhv->config( target->getPC( ), pet->getNPC( ) );
    }

    if (pet->master)
        follower_stop(pet);

    affect_add_charm(pet);

    target->getPC( )->pet = pet->getNPC( );
    follower_add( pet, target );
    pet->leader = target;

    // Ensure that when charm is off, default mobile AI won't be active for a while.
    if (pet->getNPC()->behavior) {
        BasicMobileBehavior::Pointer bhv = pet->getNPC()->behavior.getDynamicPointer<BasicMobileBehavior>();
        if (bhv)
            bhv->setLastCharmTime();
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, look_auto, "(room): вывести описание комнаты room, будто там набрали look" )
{
    checkTarget( );
    Room *room = arg2room( get_unique_arg( args ) );

    Room *was_in = target->in_room;
    target->in_room = room;
    interpret_raw(target, "look", "auto");
    target->in_room = was_in;

    return Register( );
}

NMI_GET(CharacterWrapper, screenreader, "пользуется ли персонаж клиентом или режимом для незрячих")
{
    checkTarget();
    return uses_screenreader(target);
}

NMI_GET( CharacterWrapper, affected, "список всех аффектов (List из структур Affect)" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (auto &paf: target->affected) 
        rc->push_back( AffectWrapper::wrap( *paf ) );
        
    return wrap(rc);
}

NMI_GET( CharacterWrapper, hasDestiny, "моб имеет предназначение (квестовые и спец-мобы)" )
{
    checkTarget( );
    CHK_PC
    
    if (target->getNPC( )->behavior)
        return target->getNPC( )->behavior->hasDestiny( );
    else
        return Register( false );
}

NMI_INVOKE( CharacterWrapper, hasOccupation, "(): моб имеет занятие (shopper,practicer,repairman,quest_trader,quest_master,healer,blacksmith,trainer,clanguard,adept)" )
{
    checkTarget( );
    CHK_PC

    DLString occName = args2string( args );
    return mob_has_occupation( target->getNPC( ), occName );
}

NMI_INVOKE( CharacterWrapper, switchTo, "(mob): вселиться в тело моба" )
{
    Character *victim;
        
    checkTarget();
    CHK_NPC

    victim = arg2character( args.front( ) );
    if (!victim->is_npc( ))
        throw Scripting::Exception( "Impossible to switch to PC" ); 
    
    if (target->desc == 0)
        throw Scripting::Exception( "Zero descriptor for switch" ); 

    if (target->getPC( )->switchedTo) 
        throw Scripting::Exception( "Character already switched" ); 

    if (victim->desc != 0)
        throw Scripting::Exception( "Switch victim is already in use" ); 

    wiznet( WIZ_SWITCHES, WIZ_SECURE, target->get_trust( ), "%C1 switches into %C4.", target, victim );

    victim->getNPC( )->switchedFrom = target->getPC( );
    target->getPC( )->switchedTo = victim->getNPC( );
    
    target->desc->associate( victim );
    target->desc = 0;

    return Register( );
}


NMI_INVOKE( CharacterWrapper, switchFrom, "(): выселиться из моба обратно" )
{
    checkTarget( );
    CHK_PC
    
    if (target->desc == 0)
        throw Scripting::Exception( "Switched mobile has no descriptor" ); 
    
    if (!target->getNPC( )->switchedFrom) 
        throw Scripting::Exception( "Try to return from non-switched mobile" );

    wiznet( WIZ_SWITCHES, WIZ_SECURE, target->get_trust( ), "%C1 returns from %C2.", target->getNPC( )->switchedFrom, target );
    
    target->desc->associate( target->getNPC( )->switchedFrom );
    target->getNPC( )->switchedFrom->switchedTo = 0;
    target->getNPC( )->switchedFrom = 0;
    target->desc = 0;

    return Register( );
}

NMI_INVOKE( CharacterWrapper, setDead, "(): DEPRECATED" )
{
    checkTarget( );
    CHK_PC
    target->setDead( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, isDead, "(): DEPRECATED" )
{
    checkTarget( );
    CHK_PC
    return target->isDead( );
}

NMI_INVOKE( CharacterWrapper, writeWSCommand, "(cmd,args...): отправить в веб-клиент команду с аргументами" )
{
    checkTarget( );
    CHK_NPC

    Json::Value val;
    RegisterList::const_iterator i = args.begin( );

    val["command"] = (i++)->toString();

    for(int j=0;i != args.end();i++, j++) {
        switch(i->type) {
            case Register::NONE:
                val["args"][j] = Json::Value::null;
                break;
            case Register::NUMBER:
                val["args"][j] = i->toNumber();
                break;
            case Register::STRING:
                val["args"][j] = i->toString();
                break;
            default:
                throw Scripting::Exception( "Unsupported type exception" );
        }
    }

    return target->desc->writeWSCommand(val);
}

NMI_INVOKE( CharacterWrapper, eat, "(ob): заполнить желудок так, будто obj был съеден" )
{
    checkTarget( );
    ::Object *obj = arg2item( args.front( ) );

    if (obj->item_type == ITEM_FOOD) {
        desire_hunger->eat( target->getPC( ), obj->value0() * 2 );
    }

    return Register( );
}

// Apply every desire's drink() for one liquid, mirroring CMDRUN(drink)'s
// desireManager loop. Post-hunger-rework the registered desires are exactly
// these four (FullDesire removed); callers guard on !is_npc, so getPC() is safe.
static void wrapper_apply_drink( PCharacter *pch, int amount, Liquid *liq )
{
    desire_thirst->drink( pch, amount, liq );
    desire_drunk->drink( pch, amount, liq );
    desire_hunger->drink( pch, amount, liq );
    desire_bloodlust->drink( pch, amount, liq );
}

NMI_INVOKE( CharacterWrapper, drink, "(obj,amount): применить ВСЕ желания как от amount глотков жидкости из obj (drink_con/fountain)" )
{
    checkTarget( );
    ::Object *obj;
    int amount;

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );

    obj = arg2item( args.front( ) );
    amount = args.back( ).toNumber( );

    if ((obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOUNTAIN)
        && !target->is_npc( ))
    {
        Liquid *liq = liquidManager->find( obj->value2() );
        if (liq)
            wrapper_apply_drink( target->getPC( ), amount, liq );
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, drinkLiquid, "(liqIndex,amount): применить ВСЕ желания как от amount глотков жидкости с индексом liqIndex (питье из комнаты/лужи, без объекта)" )
{
    checkTarget( );

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );

    Liquid *liq = liquidManager->find( args.front( ).toNumber( ) );
    int amount = args.back( ).toNumber( );

    if (liq && !target->is_npc( ))
        wrapper_apply_drink( target->getPC( ), amount, liq );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, canDrink, "(): можно ли пить сейчас; false если желание мешает (напр. слишком пьян). Само печатает причину отказа." )
{
    checkTarget( );

    if (target->is_npc( ))
        return Register( true );

    PCharacter *pch = target->getPC( );
    // Mirror CMDRUN(drink)'s canDrink gate over all desires; only DrunkDesire
    // actually blocks (and pechos "*ИК*"), the rest return true silently.
    if (!desire_thirst->canDrink( pch )) return Register( false );
    if (!desire_drunk->canDrink( pch )) return Register( false );
    if (!desire_hunger->canDrink( pch )) return Register( false );
    if (!desire_bloodlust->canDrink( pch )) return Register( false );

    return Register( true );
}

NMI_INVOKE( CharacterWrapper, drinkWasted, "(liqIndex): true если ВСЕ желания, что поит эта жидкость, уже заполнены -- глоток впустую (жажда/голод/алкоголь с запасом -> false)" )
{
    checkTarget( );

    if (target->is_npc( ))
        return Register( false );

    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );

    Liquid *liq = liquidManager->find( args.front( ).toNumber( ) );
    if (!liq)
        return Register( false );

    PCharacter *pch = target->getPC( );

    // Not wasted if the liquid feeds ANY applicable desire that still has room:
    // covers thirst-not-full, hunger-feeding drinks (milk/juice), and alcohol
    // (drunk below cap; a maxed drunk already stopped drink() at canDrink).
    //   `feeds` guards the vacuous case: a liquid that feeds NO applicable desire
    // -- salt water/ink/swill (thirst <= 0), blood for a mortal, anything but blood
    // for a vampire -- is NOT wasted; it is a drinkable trap/flavor, let it through.
    bool feeds = false;
    if (desire_thirst->applicable( pch )    && liq->getDesires( )[desire_thirst->getIndex( )] > 0)    { feeds = true; if (!desire_thirst->isFull( pch ))    return Register( false ); }
    if (desire_drunk->applicable( pch )     && liq->getDesires( )[desire_drunk->getIndex( )] > 0)     { feeds = true; if (!desire_drunk->isFull( pch ))     return Register( false ); }
    if (desire_hunger->applicable( pch )    && liq->getDesires( )[desire_hunger->getIndex( )] > 0)    { feeds = true; if (!desire_hunger->isFull( pch ))    return Register( false ); }
    if (desire_bloodlust->applicable( pch ) && liq->getDesires( )[desire_bloodlust->getIndex( )] > 0) { feeds = true; if (!desire_bloodlust->isFull( pch )) return Register( false ); }

    return Register( feeds );
}

NMI_INVOKE(CharacterWrapper, give, "(vict,vnum|obj): дать персонажу vict предмет obj, создав его, если указан внум")
{
    checkTarget( );
    Character *vict = argnum2character(args, 1);
    Register arg2 = argnum(args, 2);
    ::Object *item;

    if (arg2.type == Register::NUMBER) {
        OBJ_INDEX_DATA *pObj = get_obj_index(arg2.toNumber());
        if (!pObj)
            throw Scripting::Exception("Object with this vnum does not exist.");

        item = create_object(pObj, 0);
    } else {
        item = arg2item(arg2);
    }

    obj_from_anywhere(item);
    obj_to_char(item, vict);

    vict->pecho(_("%^C1 дает тебе %O4."), target, item);
    vict->recho(_("%^C1 дает %C3 %O4."), target, vict, item);

    return Register();
}

NMI_INVOKE(CharacterWrapper, giveBack, "(vict,obj): вернуть персонажу vict предмет obj")
{
    checkTarget( );
    Character *vict = argnum2character(args, 1);
    ::Object *item = argnum2item(args, 2);

    if (item->carried_by != target)
        throw Scripting::Exception("Object you're trying to give back is not carried by this character.");
    
    obj_from_char(item);
    obj_to_char(item, vict);

    vict->pecho(_("%^C1 возвращает тебе %O4."), target, item);
    vict->recho(_("%^C1 возвращает %C3 %O4."), target, vict, item);

    return Register();
}

NMI_INVOKE(CharacterWrapper, attribute, "(name[,value]): установить или вернуть аттрибут с данным именем")
{
    checkTarget();
    CHK_NPC
    DLString name = argnum2string(args, 1);
    auto &attrs = target->getPC()->getAttributes();
    bool exists = attrs.isAvailable(name);

    // Setting a new attribute
    if (args.size() > 1) {
        if (exists)
            throw Scripting::Exception("Attribute " + name + " already exists");

        Register value = args.back();

        // Determine attribute type from the value type: either an empty attr, a string or an integer attribute
        if (value.type == Register::NONE)
            attrs.getAttr<XMLEmptyAttribute>(name);
        else if (value.type == Register::NUMBER)
            attrs.getAttr<XMLIntegerAttribute>(name)->setValue(value.toNumber());
        else if (value.type == Register::STRING)
            attrs.getAttr<XMLStringAttribute>(name)->setValue(value.toString());
        else
            throw Scripting::Exception("Invalid attribute value for " + name);

        return value;
    }

    if (!exists)
        return Register();

    // Return existing attribute value
    XMLAttribute::Pointer attr = attrs.find(name)->second;
    return attr->toRegister();
}

NMI_INVOKE(CharacterWrapper, trustCheck, "(action, ch): выполнить проверку на траст для персонажа ch, вернет true если действие разрешено")
{
    checkTarget();
    CHK_NPC
    DLString action = argnum2string(args, 1);
    Character *ch = argnum2character(args, 2);
        
    XMLAttributeTrust::Pointer trust = target->getPC( )->getAttributes( ).findAttr<XMLAttributeTrust>( action );
    if (!trust)
        return false;

    return trust->check(ch);
}

NMI_INVOKE(CharacterWrapper, trustParse, "(action, trustArgs, successMsg): задать новый тип траста для действия action, вернет true если задано успешно")
{
    checkTarget();
    CHK_NPC
    DLString action = argnum2string(args, 1);
    DLString trustArgs = argnum2string(args, 2);
    DLString successMsg = argnum2string(args, 3);
    ostringstream buf;

    XMLAttributeTrust::Pointer trust = target->getPC( )->getAttributes( ).getAttr<XMLAttributeTrust>( action );
    
    bool rc = trust->parse(trustArgs, buf);
    if (rc)
        target->send_to(successMsg);
    target->pecho(buf.str());

    return rc;
}

NMI_INVOKE(CharacterWrapper, restring, "(skill,key,names,short,long,extra): установить аттрибут для рестринга результатов заклинаний")
{
    checkTarget( );
    CHK_NPC

    args2restringAttribute(args, target->getPC());
    target->getPC( )->save( );
    
    return Register( );
}

NMI_INVOKE(CharacterWrapper, hasBehavior, "(bhvName): true если среди поведений моба есть указанное")
{
    checkTarget();

    if (!target->is_npc())
        return false;

    DLString bhvName = args2string(args);
    Behavior *bhv = behaviorManager->findExisting(bhvName);
    if (!bhv)
        throw Scripting::Exception("Behavior " + bhvName + " doesnt exist");

    return Register(target->getNPC()->pIndexData->behaviors.isSet(bhv->getIndex()));
}

NMI_INVOKE(CharacterWrapper, behaviorMethod, "(methodName[, args...]): вызвать метод MobileBehavior с аргументами")
{
    checkTarget();
    CHK_PC
    DLString methodName = argnum2string(args, 1);

    if (methodName == "shot") {
        Character *attacker = argnum2character(args, 2);
        int door = argnum2number(args, 3);
        if (target->getNPC()->behavior)
            target->getNPC()->behavior->shot(attacker, door);
        return Register();
    }

	if (methodName == "flee") {
        if (target->getNPC()->behavior)
            target->getNPC()->behavior->flee();
        return Register();
	}

    throw Scripting::Exception(methodName + " behavior method not supported yet");
}

// Call aquest trigger if applicable for this target.
static bool call_aquest_trigger(CharacterWrapper *mobWrapper, const DLString &trigName, const RegisterList &trigArgs)
{
    Character *mob = mobWrapper->getTarget();

    if (!mob->is_npc())
        return false;

    WrapperBase *indexWrapper = get_wrapper(mob->getNPC()->pIndexData->wrapper);
    if (!indexWrapper)
        return false;

    // Assume first argument is the PC quest doer
    CharacterWrapper *playerWrapper = wrapper_cast<CharacterWrapper>(trigArgs.front());
    if (!playerWrapper)
        return false;

    Character *player = playerWrapper->getTarget();
    if (player->is_npc())
        return false;

    // Mob on whom the quest trigger is invoked should be the first argument
    RegisterList questTrigArgs = trigArgs;
    questTrigArgs.push_front(Register(mobWrapper->getSelf()));

    return aquest_trigger(indexWrapper, player->getPC(), trigName, questTrigArgs);
}

// Call behavior triggers if applicable for this target.
static bool call_behavior_trigger(CharacterWrapper *mobWrapper, const DLString &trigName, const RegisterList &trigArgs)
{
    Character *mob = mobWrapper->getTarget();

    if (!mob->is_npc())
        return false;

    GlobalBitvector &behaviors = mob->getNPC()->pIndexData->behaviors;
    if (behaviors.empty())
        return false;

    // Make this mob a first argument
    RegisterList behaviorTrigArgs = trigArgs;
    behaviorTrigArgs.push_front(Register(mobWrapper->getSelf()));

    auto results = behavior_trigger_with_result(behaviors, trigName, behaviorTrigArgs);
    return reglist_to_bool(results);
}

// Call normal onXXX, postXXX Fenia triggers on the target.
static Register call_fenia_trigger(CharacterWrapper *wrapper, const DLString &trigName, const RegisterList &trigArgs)
{
    Character *target = wrapper->getTarget();

    // If it's a mob, access its mob index data wrapper.
    WrapperBase *proto = target->is_npc() ? get_wrapper(target->getNPC()->pIndexData->wrapper) : 0;

    // Helper function will invoke onDeath, postDeath triggers on character and proto.
    Register result(false);
    fenia_trigger(result, trigName, trigArgs, wrapper, proto);
    return result;
}

NMI_INVOKE(CharacterWrapper, trigger, "(trigName, trigArgs...): вызвать триггер у персонажа или прототипа")
{
    checkTarget();

    // Get trig name such as "Death" or "Get", and trig arguments (all but first one)
    DLString trigName = argnum2string(args, 1);
    RegisterList trigArgs = args;
    trigArgs.pop_front();

    // 1. area quests
    call_aquest_trigger(this, trigName, trigArgs);

    // 2. behaviors
    if (call_behavior_trigger(this, trigName, trigArgs))
        return true;

    // 3. onXXX, postXXX triggers
    return call_fenia_trigger(this, trigName, trigArgs);

    // 4. legacy C++ behaviors: but hopefully wont' be needed from Fenia
}

NMI_INVOKE(CharacterWrapper, menu, "([number, action[, number, action]]): очистить или установить пункты меню number с действием action")
{
    checkTarget( );
    CHK_NPC

    if (args.empty()) {
        Player::menuClear(target->getPC());
        return Register();
    }

    // Numbers and actions count need to match.
    if (args.size() % 2 != 0)
        throw Scripting::Exception("Menu expects EVEN number of arguments");

    for (auto a = args.begin(); a != args.end(); a++) {
        DLString choice = arg2number(*a++);
        DLString action = arg2string(*a);
        Player::menuSet(target->getPC(), choice, action);
    }

    Player::menuPrint(target->getPC());
    return Register();
}

NMI_INVOKE(CharacterWrapper, hash, "(mod): вернуть ключ к хеш-таблице по модулю mod")
{
    checkTarget();
    int mod = args2number(args);
    return Register((int)(target->getID() % mod));
}
 
NMI_INVOKE( CharacterWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    Scripting::traitsAPI<CharacterWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

NMI_GET(CharacterWrapper, killed, "статистика убийств мобов")
{
    checkTarget();
    CHK_NPC
    auto killingAttr = target->getPC()->getAttributes().getAttr<XMLKillingAttribute>("killed");
    return killingAttr->toRegister();
}

NMI_GET(CharacterWrapper, gquest, "статистика побед в глобальных квестах")
{
    checkTarget();
    CHK_NPC
    auto statAttr = target->getPC()->getAttributes().findAttr<XMLAttributeStatistic>("gquest");
    if (!statAttr)
        return Register();
    return statAttr->toRegister(target->getPC(), "gquest");
}

NMI_GET(CharacterWrapper, quest, "статистика побед в авто квестах")
{
    checkTarget();
    CHK_NPC
    auto statAttr = target->getPC()->getAttributes().findAttr<XMLAttributeStatistic>("questdata");
    if (!statAttr)
        return Register();
    return statAttr->toRegister(target->getPC(), "questdata");
}

NMI_GET(CharacterWrapper, questNextMinutes, "минут до того, как можно попросить у квестора новое задание; 0 если уже можно прямо сейчас")
{
    checkTarget();
    CHK_NPC
    // countdown is dual-purpose: while a quest is RUNNING it counts down the
    // time left to COMPLETE that quest, and only with no active quest does it
    // mean "minutes until you may request a new one" (xmlattributequestdata
    // pull() branches on the quest attr; cquest doTime prints two different
    // sentences off it). Report 0 during an active quest so callers never
    // mislabel the completion timer as a request cooldown.
    if (target->getPC()->getAttributes().findAttr<Quest>("quest"))
        return Register(0);

    // The questor cooldown lives in the questdata attribute's countdown field
    // (XMLAttributeQuestData::getTime), the same value `quest time` prints and
    // the questor sets on completion/cancel. A player with no questdata attr yet
    // (never quested) can quest right away -> 0. Never report a negative.
    auto attr = target->getPC()->getAttributes().findAttr<XMLAttributeQuestData>("questdata");
    if (!attr)
        return Register(0);
    int t = attr->getTime();
    return Register(t > 0 ? t : 0);
}

NMI_GET(CharacterWrapper, questVictimVnum, "внум цели активного авто-задания на убийство, 0 если такой цели нет")
{
    checkTarget();
    CHK_NPC

    // The running autoquest lives as the "quest" attribute on the hero. Only a
    // LIVE victim-quest answers. A Fenia kill quest sits at QSTAT_INIT from
    // hand-out until the target dies -- the kill scenario never sets STARTED
    // (only steal/locate do, on client greet) -- so both non-terminal states
    // count. The moment the marked target dies deathAsVictim flips the state to
    // FINISHED/BROKEN (both >= 42) synchronously inside mprog_death, which runs
    // BEFORE the global onDeath trigger that reads this -- so the correct kill
    // reports 0 and never trips the "wrong target" hint. An unmarked same-vnum
    // kill leaves the quest live and the real victim alive elsewhere, which is
    // exactly the newbie confusion this teaches (Trello 398).
    // findMarkedMobile("victim") returns NULL for every non-kill type, so no
    // extra type check is needed.
    Quest::Pointer q = target->getPC()->getAttributes().findAttr<Quest>("quest");
    if (!q)
        return Register(0);

    int st = q->state.getValue();
    if (st != QSTAT_INIT && st != QSTAT_STARTED)
        return Register(0);

    FeniaQuest *fq = q.getDynamicPointer<FeniaQuest>();
    if (!fq)
        return Register(0);

    NPCharacter *victim = fq->findMarkedMobile("victim");
    if (!victim)
        return Register(0);

    return Register(victim->pIndexData->vnum);
}

NMI_GET(CharacterWrapper, attributes, "Array всех аттрибутов, ключ - имя аттрибута, значение - Map с полями аттрибута либо пустая строка")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getAttributes().toRegister();
}


