/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>
#include <cstdlib>

#include "questtrader.h"
#include "xmlattributequestreward.h"
#include "xmlattributequestdata.h"
#include "occupations.h"
#include "defaultreligion.h"

#include "affect.h"
#include "object.h"
#include "skill_utils.h"
#include "itemevents.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "lang.h"
#include "npcharacter.h"
#include "string_utils.h"
#include "merc.h"
#include "wearloc_utils.h"
#include "arg_utils.h"
#include "wiznet.h"
#include "interp.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

/*------------------------------------------------------------------------
 * QuestTrader 
 *-----------------------------------------------------------------------*/
int QuestTrader::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_QUEST_TRADER);
}

void QuestTrader::doTrouble( PCharacter *client, const DLString &constArguments )
{
    Article::Pointer article;
    PersonalQuestArticle::Pointer personal;
    DLString arguments, arg;
    
    if (!canServeClient( client ))
        return;
    
    arguments = constArguments;
    arg = arguments.getOneArgument( );
    if (arg.empty( )) {
        tell_act( client, getKeeper( ), _("Какую именно вещь ты хочешь вернуть?") );
        return;
    }

    article = findArticle( client, arg );

    if (!article) {
        msgArticleNotFound( client );
        return;
    }
    
    personal = article.getDynamicPointer<PersonalQuestArticle>( );
    
    if (!personal)
        tell_act( client, getKeeper( ), _("Извини, $c1, я не могу вернуть тебе эту вещь.") );
    else
        personal->trouble( client, getKeeper( ) );
}

bool QuestTrader::canServeClient( Character *client )
{
    if (client->is_npc( ))
        return false;

    if (IS_GHOST( client )) {
        say_act( client, getKeeper( ), _("Наслажденье жизнью недоступно призракам.") );
        return false;
    }

    if (IS_CHARMED(client)) {
        say_act( client, getKeeper( ), _("Ты не можешь сделать этого, пока ты не владеешь собой!") );
        return false;
    }
   
    if (getKeeper( )->fighting) {
        say_act( client, getKeeper( ), _("Подожди немного, $c1, мне сейчас не до тебя.") );
        return false;
    }

    if (!getKeeper( )->can_see( client )) {
        say_act( client, getKeeper( ), _("Я не общаюсь с невидимками.") );
        return false;
    }
    
    return true;
}

void QuestTrader::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), _("Извини, $c1, мне нечего тебе предложить.") );
}

void QuestTrader::msgListRequest( Character *client ) 
{
    oldact(_("$c1 просит $C4 показать список вещей."), client, 0, getKeeper( ), TO_ROOM );
    oldact(_("Ты просишь $C4 показать список вещей."), client, 0, getKeeper( ), TO_CHAR );
}

void QuestTrader::msgListBefore( Character *client ) 
{
    client->pecho(_("Перечень квестовых вещей для покупки:"));
}

void QuestTrader::msgListAfter( Character *client )
{
    client->pecho( _("Для покупки чего-либо используй {yквест купить {Dназвание{x.") );
}

void QuestTrader::msgArticleNotFound( Character *client ) 
{
    say_act( client, getKeeper( ), _("У меня нет этого, $c1.") );
}

void QuestTrader::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), _("Не жадничай.") );
}

void QuestTrader::msgBuyRequest( Character *client )
{
    oldact(_("$c1 о чем-то просит $C4."), client, 0, getKeeper( ), TO_ROOM );
}

/*----------------------------------------------------------------------------
 * QuestTradeArticle 
 *---------------------------------------------------------------------------*/
void QuestTradeArticle::toStream( Character *client, ostringstream &buf ) const
{
    DLString myname = viewerLang(client) != LANG_EN && !rname.empty() ? rname : name;
    buf << "    " << setiosflags( ios::right ) << setw( 7 );
    
    price->toStream( client, buf );

    buf << resetiosflags( ios::left )
        << ".........." << descr << " ({D" << myname << "{x)" << endl;
}

bool QuestTradeArticle::visible( Character * ) const
{
    return true;
}

bool QuestTradeArticle::available( Character *, NPCharacter * ) const
{
    return true;
}

bool QuestTradeArticle::matches( const DLString &argument ) const
{
    if (argument.empty())
        return false;
    
    return arg_oneof(argument, name.c_str(), rname.c_str());
}

int QuestTradeArticle::getQuantity( ) const
{
    return 1;
}

bool QuestTradeArticle::purchase( Character *client, NPCharacter *questman, const DLString &, int )
{
    if (!price->canAfford( client )) {
        say_act( client, questman, _("Извини, $c1, но у тебя недостаточно $n2 для этого."),
                 price->toCurrency( ).c_str( ) );
        return false;
    } else if (!client->is_npc( )) {
        price->deduct( client );
        buy( client->getPC( ), questman );
        return true;
    }

    return false;
}

/*----------------------------------------------------------------------------
 * ObjectQuestArticle 
 *---------------------------------------------------------------------------*/
void ObjectQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    Object *obj;

    obj = create_object( get_obj_index( vnum ), client->getRealLevel( ) );
    
    buyObject( obj, client, questman );

    oldact(_("$C1 дает $o4 $c3."), client, obj, questman, TO_ROOM );
    oldact(_("$C1 дает тебе $o4."), client, obj, questman, TO_CHAR );
    obj_to_char( obj, client );
}

void ObjectQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman ) 
{
}

/*----------------------------------------------------------------------------
 * PersonalQuestArticle 
 *---------------------------------------------------------------------------*/
PersonalQuestArticle::PersonalQuestArticle( ) 
                          : gender( 0, &sex_table )
{
}

/** The adjectives the quest master engraves on a hero item, indexed
 *  [alignment: good, neutral, evil][gender: neuter, male, female]. Russian and
 *  Ukrainian carry the six-case pad, English has neither case nor gender. The
 *  Ukrainian nouns in limbo have to keep the same gender as the Russian ones,
 *  or the adjective will not agree with them. Read backwards by
 *  PersonalNameRepair at the bottom of this file, so keep the Russian strings
 *  exactly as they have always been written into objects. */
static const LangText HERO_ADJECTIVES[3][3] = {
    {
        { "holy", "Священн|ое|ого|ому|ое|ым|ом", "Священн|е|ого|ому|е|им|ому" },
        { "holy", "Священн|ый|ого|ому|ый|ым|ом", "Священн|ий|ого|ому|ий|им|ому" },
        { "holy", "Священн|ая|ой|ой|ую|ой|ой",   "Священн|а|ої|ій|у|ою|ій" },
    },
    {
        { "shimmering", "Мерцающ|ее|его|ему|ее|им|ем", "Мерехтлив|е|ого|ому|е|им|ому" },
        { "shimmering", "Мерцающ|ий|его|ему|ий|им|ем", "Мерехтлив|ий|ого|ому|ий|им|ому" },
        { "shimmering", "Мерцающ|ая|ей|ей|ую|ей|ей",   "Мерехтлив|а|ої|ій|у|ою|ій" },
    },
    {
        { "devilish", "Дьявольск|ое|ого|ому|ое|им|ом", "Диявольськ|е|ого|ому|е|им|ому" },
        { "devilish", "Дьявольск|ий|ого|ому|ий|им|ом", "Диявольськ|ий|ого|ому|ий|им|ому" },
        { "devilish", "Дьявольск|ая|ой|ой|ую|ой|ой",   "Диявольськ|а|ої|ій|у|ою|ій" },
    },
};

/** Pick the adjective for this buyer and this article's declared gender. */
static const LangText & hero_adjective( PCharacter *client, int gender )
{
    const LangText (&adjectives)[3][3] = HERO_ADJECTIVES;

    int a = IS_GOOD(client) ? 0 : IS_NEUTRAL(client) ? 1 : 2;
    int g;

    switch (gender) {
    case SEX_MALE:   g = 1; break;
    case SEX_FEMALE: g = 2; break;
    default:         g = 0; break;
    }

    return adjectives[a][g];
}

void PersonalQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman )
{
    obj->setOwner( client->getNameC() );

    // Engrave the adjective and the owner's name into every language slot: all
    // four hero items template both into their short descr in en/ru/ua. Writing
    // only LANG_DEFAULT left EN and UA readers with the raw "%s" template, and
    // every purchase minted another one.
    const LangText &adjective = hero_adjective( client, gender.getValue( ) );

    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        const DLString &pattern = obj->getShortDescr(lang);

        if (!pattern.empty())
            obj->setShortDescr( fmt(0, pattern.c_str(),
                                    adjective.get(lang),
                                    client->getNameP('2', lang).c_str()), lang );
    }

    if (troubled) {
        XMLAttributeQuestReward::Pointer attr;

        attr = client->getAttributes( ).getAttr<XMLAttributeQuestReward>( "questreward" );

        if (attr->getCount( vnum ) == 0)
            attr->setCount( vnum, 1 );
    }
}

void PersonalQuestArticle::trouble( PCharacter *client, NPCharacter *questman ) 
{
    Object *obj;
    int count = 0;
    XMLAttributeQuestReward::Pointer attr;
    
    if (troubled) {
        attr = client->getAttributes( ).findAttr<XMLAttributeQuestReward>( "questreward" );
        
        if (attr)
            count = attr->getCount( vnum );
    }

    if (count == 0 || count > 3) {
        tell_act( client, questman, _("Извини, $c1, я не могу вернуть тебе эту вещь.") );
        return;
    }
    
    obj = get_obj_world_unique( vnum, client );

    if (obj) {
        tell_act( client, questman, _("Извини, но у тебя уже есть $o1."), obj );
        // extract_obj( obj ); у вас все было 
        return;
    }

    buy( client, questman );
    tell_act( client, questman, _("Я возвращаю тебе эту вещь $t-й раз."), DLString( count ).c_str( ) );
    
    if (count == 3) 
        tell_act( client, questman, _("Будь внимательнее! В следующий раз я не смогу помочь тебе.") );
    
    attr->setCount( vnum, count + 1 );
}

/*---------------------------------------------------------------------------
 * GoldQI 
 *---------------------------------------------------------------------------*/
void GoldQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    client->gold += amount.getValue( );
    
    oldact(_("$C1 дает $t золотых монет $c3."), client, DLString(amount.getValue( )).c_str( ), questman, TO_ROOM );
    oldact(_("$C1 дает тебе $t золотых монет."), client, DLString(amount.getValue( )).c_str( ), questman, TO_CHAR );
}

/*---------------------------------------------------------------------------
 * ConQI 
 *---------------------------------------------------------------------------*/
void ConQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    client->perm_stat[STAT_CON]++;

    oldact(_("$C1 повышает телосложение $c2."), client, 0, questman, TO_ROOM );
    oldact(_("$C1 повышает твое телосложение."), client, 0, questman, TO_CHAR );
}
    
bool ConQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (client->perm_stat[STAT_CON] < client->getPC( )->getMaxTrain( STAT_CON ))
        return true;
    
    say_act( client, questman, _("Извини, $c1, но твое телосложение уже на максимуме.") );
    return false;
}

/*---------------------------------------------------------------------------
 * PocketsQI 
 *---------------------------------------------------------------------------*/
#define OBJ_VNUM_QUESTBAG      103

Object * PocketsQuestArticle::findBag( PCharacter *client ) const
{
    Object *obj;

    for (obj = client->carrying; obj; obj = obj->next_content) 
        if (obj->pIndexData->vnum == OBJ_VNUM_QUESTBAG 
            && !IS_SET(obj->value1(), CONT_WITH_POCKETS)) 
            break;
    
    return obj;
}

void PocketsQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    Object *obj = findBag( client );
    
    if (obj) {
        obj->value1(obj->value1() | CONT_WITH_POCKETS);
        oldact(_("$C1 пришивает карманы на $o4."), client, obj, questman, TO_CHAR);
        oldact(_("$C1 пришивает $c5 карманы на $o4."), client, obj, questman, TO_ROOM);
    }
}

bool PocketsQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (findBag( client->getPC( ) )) 
        return true;

    say_act( client, questman, _("Извини, $c1, но я не вижу у тебя сумки без карманов.") );
    return false;
}

/*---------------------------------------------------------------------------
 * KeyringQI 
 *---------------------------------------------------------------------------*/
#define OBJ_VNUM_QUESTGIRTH    94
#define OBJ_VNUM_QUESTKEYRING  119

void KeyringQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    Object *girth, *keyring;
    Wearlocation *waist;
    
    if (( girth = get_obj_carry_vnum( client, OBJ_VNUM_QUESTGIRTH ) ) == NULL)
        return;
    
    keyring = create_object( get_obj_index( OBJ_VNUM_QUESTKEYRING ), 0 );
    obj_to_char( keyring, client );
    
    keyring->setOwner( girth->getOwner( ) );

    // The keyring wears the girth's personalised name, so it has to copy every
    // language slot -- a girth bought after this change carries all three. Copy
    // only what the girth itself holds: getShortDescr(lang) falls back to the
    // girth PROTOTYPE, and a girth bought earlier has empty EN/UA instance slots,
    // so copying through the fallback would stamp the raw "%s" template onto the
    // keyring instead of leaving it its own clean name. The keyword is already
    // copied whole below, as an XMLMultiString.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        if (!girth->getRealShortDescr(lang).empty())
            keyring->setShortDescr( girth->getRealShortDescr(lang), lang );

        if (!girth->getRealDescription(lang).empty())
            keyring->setDescription( girth->getRealDescription(lang), lang );
    }


    if (!girth->getRealKeyword( ).empty())
        keyring->setKeyword( girth->getRealKeyword( ) );
        
    if (!girth->getRealMaterial( ).empty())
        keyring->setMaterial( girth->getRealMaterial( ) );

    keyring->extra_flags = girth->extra_flags;
    keyring->condition = girth->condition;
    keyring->level = girth->level;

    for (auto &paf: girth->affected)
        affect_to_obj( keyring, paf );

    for (auto &ed: girth->extraDescriptions)
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            lang_t lang = (lang_t)l;

            if (!ed->description.get(lang).empty())
                keyring->addExtraDescr( ed->keyword, ed->description.get(lang), lang );
        }

    waist = &*girth->wear_loc;
    waist->unequip( girth );
    waist->equip( keyring );
    extract_obj( girth );

    oldact(_("$C1 прикрепляет огромный брелок к $o3."), client, keyring, questman, TO_CHAR);
    oldact(_("$C1 прикрепляет огромный брелок к $o3."), client, keyring, questman, TO_ROOM);
}

bool KeyringQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (get_obj_carry_vnum( client, OBJ_VNUM_QUESTGIRTH ))
        return true;

    say_act( client, questman, _("Извини, $c1, но я не вижу у тебя пояса без брелков.") );
    return false;
}

/*----------------------------------------------------------------------
 * OwnerPrice 
 *---------------------------------------------------------------------*/
const DLString OwnerPrice::LIFE_NAME = "перерождени|я|й|ям|я|ями|ях";
const DLString OwnerPrice::VICTORY_NAME = "побед|ы||ам|ы|ами|ах в квестах";

DLString OwnerPrice::toCurrency( ) const
{
    return LIFE_NAME + " или " + VICTORY_NAME;
}

// Both of these deliberately return Flexer PADS, not finished sentences: every
// caller hands the result on as a declinable argument ($n4 in mkey.cpp, $n2 in
// the refusal below), so the caller is what picks the case. Wrapping either in a
// catalog frame would destroy that. Making them trilingual therefore means
// per-language pads, plus a viewer on Price::toCurrency(), which is a pure
// virtual shared with MoneyPrice/QuestPointPrice and has none -- its own change.
DLString OwnerPrice::toString( Character * ) const
{
    ostringstream buf;

    // The space before VICTORY_NAME was missing, which glued the digit onto the
    // pad's root ("5побед|ы|...") and printed "5победы в квестах". Flexer resets
    // at whitespace, so the space also restores that word's declension.
    buf << lifes << " " << LIFE_NAME << " или " << victories << " " << VICTORY_NAME;
    return buf.str( );
}

bool OwnerPrice::canAfford( Character *ch ) const
{
    if (ch->is_npc( ))
        return false;
    
    return getValue( ch->getPC( ) ) - ch->getPC( )->getRemorts( ).owners > 0;
}

int OwnerPrice::getValue( PCharacter *ch ) const
{
    int my_victories, my_lifes, total;
    
    my_victories = ch->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getBonusVictoriesCount();
    my_lifes = ch->getRemorts( ).countBonusLifes();
    total = my_victories / victories + my_lifes / lifes;
    
    return total / 2;
}

void OwnerPrice::induct( Character *ch ) const
{
}

void OwnerPrice::deduct( Character *ch ) const
{
    if (!ch->is_npc( )) {
        ch->getPC( )->getRemorts( ).owners++;
        ::wiznet( WIZ_QUEST, 0, 0, "%1$^C1 приобретает купон владельца.", ch );
    }
}

void OwnerPrice::toStream( Character *ch, ostringstream &buf ) const
{
}

/*---------------------------------------------------------------------------
 * OwnerCouponQI
 *---------------------------------------------------------------------------*/
bool OwnerQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (lifePrice.getValue( client->getPC( ) ) <= 0) {
        say_act( client, questman, _("Извини, $c1, но у тебя не хватает $n2, чтобы владеть этой вещью."), lifePrice.toCurrency( ).c_str( ) );
        return false;
    }

    if (!lifePrice.canAfford( client )) {
        say_act( client, questman, _("Извини, $c1, но ты уже исчерпа$gло|л|ла отведенное тебе количество этих купонов.") );
        return false;
    }

    return true;
}

bool OwnerQuestArticle::visible( Character *client ) const 
{
    return lifePrice.canAfford( client );
}

void OwnerQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman ) 
{
    obj->setOwner( client->getNameC() );    
    lifePrice.deduct( client );
}

/*----------------------------------------------------------------------------
 * WearslotQuestArticle
 *---------------------------------------------------------------------------*/
void WearslotQuestArticle::buy( PCharacter *client, NPCharacter *questman )
{
    client->wearloc.set( wear_personal );

    oldact(_("$C1 учит тебя носить одну любую вещь просто так, для души."), client, 0, questman, TO_CHAR);
    oldact(_("$C1 что-то доверительно шепчет $c3."), client, 0, questman, TO_ROOM);
    say_act( client, questman, _("Теперь ты можешь надеть любую вещь командой {yнадеть {Dвещь{x {yв слот{x и назвать это место командой {yслот имя{x.") );
}

bool WearslotQuestArticle::available( Character *client, NPCharacter *questman ) const
{
    if (client->is_npc( ))
        return false;

    if (client->getWearloc( ).isSet( wear_personal )) {
        say_act( client, questman, _("У тебя уже есть личный слот, $c1.") );
        return false;
    }

    return true;
}

bool WearslotQuestArticle::visible( Character *client ) const
{
    return !client->is_npc( ) && !client->getWearloc( ).isSet( wear_personal );
}

/*----------------------------------------------------------------------------
 * RefitQuestArticle
 *---------------------------------------------------------------------------*/
#define OBJ_VNUM_QUESTRING     95
#define OBJ_VNUM_QUESTWEAPON   96

/** The personalised, level-stamped reward items a remort can strand. Listed by
 *  vnum, the same way PersonalNameRepair enumerates its own set below: girth,
 *  ring, weapon, travelling bag, and the keyring (a girth clone, vnum 119). */
static bool refit_eligible_vnum( int vnum )
{
    switch (vnum) {
    case OBJ_VNUM_QUESTGIRTH:   // 94
    case OBJ_VNUM_QUESTRING:    // 95
    case OBJ_VNUM_QUESTWEAPON:  // 96
    case OBJ_VNUM_QUESTBAG:     // 103
    case OBJ_VNUM_QUESTKEYRING: // 119
        return true;
    }

    return false;
}

/** The qp fee to refit one item. The bag never charges; a five-level gap or less
 *  is free so a death de-level does not nickel-and-dime; otherwise the fee scales
 *  with both the gap and the item's paid tier, landing at a small fraction of the
 *  item's value. Tier is read forward-compatibly from the questTier property
 *  (absent today -> 0), so this needs no change when the upgrade ladder ships. */
static int refit_item_fee( Object *obj, PCharacter *client )
{
    int gap = obj->level - client->getRealLevel( );

    if (gap <= 0)
        return 0;

    if (obj->pIndexData->vnum == OBJ_VNUM_QUESTBAG)
        return 0;

    if (gap <= 5)
        return 0;

    int tier = atoi( obj->getProperty( "questTier" ).c_str( ) );

    return gap * (tier + 1);
}

/** Whether a carried item genuinely needs a refit. Three conditions, all
 *  required. It must be one of the personal reward items. It must NOT be worn:
 *  equipment stays on the `carrying` list with wear_loc set, and a worn item is
 *  by definition already wearable, so touching it would only write its level
 *  field out of sync with its live affects. And it must fail the real wear gate
 *  -- get_wear_level is the exact test canWear consults, so "eligible" means
 *  precisely "unwearable at your real level". A remort-stranded item (stamped
 *  near 100, owner dropped to 1) qualifies; a merely high-stamped but still
 *  wearable one (e.g. a few levels of remort bonus while worn, or an item within
 *  the profession's wear modifier) does not, so the fee is never charged for a
 *  refit that would do nothing. */
static bool refit_needed( Object *obj, PCharacter *client )
{
    if (!refit_eligible_vnum( obj->pIndexData->vnum ))
        return false;

    if (obj->wear_loc != wear_none)
        return false;

    return get_wear_level( client, obj ) > client->getRealLevel( );
}

void RefitQuestArticle::toStream( Character *client, ostringstream &buf ) const
{
    DLString myname = viewerLang(client) != LANG_EN && !rname.empty() ? rname : name;
    buf << "    " << setiosflags( ios::right ) << setw( 7 ) << _("по ур.").getMessage( client )
        << resetiosflags( ios::left )
        << ".........." << descr << " ({D" << myname << "{x)" << endl;
}

bool RefitQuestArticle::available( Character *client, NPCharacter *questman ) const
{
    if (client->is_npc( ))
        return false;

    PCharacter *pch = client->getPC( );

    for (Object *obj = pch->carrying; obj; obj = obj->next_content)
        if (refit_needed( obj, pch ))
            return true;

    say_act( client, questman, _("Мне нечего тебе подгонять, $c1: все твои вещи тебе впору.") );
    return false;
}

bool RefitQuestArticle::purchase( Character *client, NPCharacter *questman, const DLString &, int )
{
    if (client->is_npc( ))
        return false;

    PCharacter *pch = client->getPC( );

    // First pass: total the fee across every carried reward item stranded above
    // the owner's real level. Charge once, then refit all -- carrying does not
    // change between the passes, so no intermediate list is needed.
    int totalFee = 0;
    int count = 0;

    for (Object *obj = pch->carrying; obj; obj = obj->next_content)
        if (refit_needed( obj, pch )) {
            totalFee += refit_item_fee( obj, pch );
            count++;
        }

    if (count == 0)
        return false;

    if (totalFee > 0 && pch->getQuestPoints( ) < totalFee) {
        say_act( client, questman, _("Извини, $c1, но у тебя не хватает квестовых единиц: за подгонку нужно $t."),
                 DLString( totalFee ).c_str( ) );
        return false;
    }

    if (totalFee > 0)
        pch->addQuestPoints( -totalFee );

    // Second pass: refit. equip() re-stamps and rescales on the next wear, so
    // just resetting the level here makes each item wearable again.
    for (Object *obj = pch->carrying; obj; obj = obj->next_content)
        if (refit_needed( obj, pch )) {
            obj->level = pch->getRealLevel( );
            oldact( _("$C1 бережно подгоняет $o4 под твой нынешний уровень."), client, obj, questman, TO_CHAR );
            oldact( _("$C1 бережно подгоняет $o4 под уровень $c2."), client, obj, questman, TO_ROOM );
        }

    if (totalFee > 0)
        say_act( client, questman, _("Подгонка обошлась тебе в $t квестовых единиц."),
                 DLString( totalFee ).c_str( ) );

    PCharacterManager::save( pch );
    return true;
}

void RefitQuestArticle::buy( PCharacter *, NPCharacter * )
{
    // Unused: purchase() is fully overridden for dynamic, per-item pricing. The
    // base declares buy() pure virtual, so a body has to exist.
}

/*----------------------------------------------------------------------------
 * PiercingQuestArticle
 *---------------------------------------------------------------------------*/
void PiercingQuestArticle::buy( PCharacter *client, NPCharacter *tattoer ) 
{
    client->wearloc.set( wear_ears );
    
    oldact(_("$C1 достает огромный гвоздь и молниеносно пробивает дырку в мочке уха $c2."),client,0,tattoer,TO_ROOM);
    oldact(_("$C1 достает огромный гвоздь и молниеносно пробивает дырку в твоей мочке уха. АЙ!"),client,0,tattoer,TO_CHAR);
}

bool PiercingQuestArticle::available( Character *client, NPCharacter *tattoer ) const 
{
    if (!visible( client )) {
        say_act( client, tattoer, _("У тебя уже проколоты уши, $c1.") );
        say_act( client, tattoer, _("Может, тебе еще что-нибудь проколоть?") );
        interpret_raw( tattoer, "smirk" );
        return false;
    }

    if (get_eq_char( client, wear_head )) {
        interpret_raw( tattoer, "bonk", "%s", client->getNameC() );
        say_act( client, tattoer, _("Шляпу сними! Не получается до твоего уха добраться.") );
        return false;
    }

    return true;
}

bool PiercingQuestArticle::visible( Character *client ) const 
{
    return !client->is_npc( ) && !client->getWearloc( ).isSet( wear_ears );
}

/*----------------------------------------------------------------------------
 * TattooQuestArticle 
 *---------------------------------------------------------------------------*/
RELIG(none);

#define OBJ_VNUM_TATTOO 50

void TattooQuestArticle::buy( PCharacter *client, NPCharacter *tattoer )
{
    Object *obj;

    // Use tattoo vnum from religion profile if specified, otherwise use default one.
    DefaultReligion *religion = dynamic_cast<DefaultReligion *>(client->getReligion().getElement());
    int tattooVnum = 0;
    if (religion)
        tattooVnum = religion->tattooVnum;

    if (tattooVnum == 0)
        tattooVnum = OBJ_VNUM_TATTOO;

    obj = create_object( get_obj_index( tattooVnum ), 0 );

    // Bake the deity name into every language slot, taking the name in that same
    // language. The default tattoo (vnum 50) carries a "%s" template per language;
    // the per-religion tattoos (27300+) are authored in full and fmt() leaves them
    // alone. Two things were wrong before: only LANG_DEFAULT was written, so EN/UA
    // readers kept the raw "%s"; and the single-argument setKeyword() re-splits the
    // flattened keyword list by alphabet, which merged the UA keywords into the RU
    // slot and left UA empty. RU now names the deity in Russian instead of Latin.
    // Only slots whose template asks for the name get it -- nothing is appended to
    // a keyword list that did not have a "%s", so targeting is otherwise unchanged.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        // nameRus and nameUa are declinable pads (Кронос||а|у|а|ом|е), not fixed
        // words. Dropped raw into the short descr they would decline in step with
        // the whole phrase, so the deity would follow the case of the tattoo
        // instead of staying genitive after "с изображением". Decline once here,
        // the way score.cpp does it.
        const DLString &deity = client->getReligion( )->getNameFor( lang );
        DLString leaderShort = deity.ruscase( '2' );
        DLString leaderKey = deity.ruscase( '1' );

        obj->setKeyword( fmt(0, obj->getKeyword(lang).c_str(), leaderKey.c_str()), lang );
        obj->setShortDescr( fmt(0, obj->getShortDescr(lang).c_str(), leaderShort.c_str()), lang );
    }

    obj_to_char( obj, client );
    oldact(_("$C1 наносит тебе $o4!"), client, obj, tattoer, TO_CHAR );
    oldact(_("$C1 наносит $c3 $o4!"), client, obj, tattoer, TO_ROOM );

    equip_char( client, obj, wear_tattoo );    
}

bool TattooQuestArticle::available( Character *client, NPCharacter *tattoer ) const 
{
    if (client->is_npc( ))
        return false;

    if (client->getReligion( ) == god_none) {
        say_act( client, tattoer, _("$c1, ты атеист$g||ка и не можешь получить знак религии.") );
        return false;
    }

    if (wear_tattoo->find( client )) {
        say_act( client, tattoer, _("Но у тебя уже есть знак религии, $c1!") );
        return false;
    }

    DefaultReligion *religion = dynamic_cast<DefaultReligion *>(&client->getReligion());
    if (religion && religion->tattooVnum != 0 && !get_obj_index(religion->tattooVnum)) {
        say_act(client, tattoer, _("Я не могу сейчас нанести тебе этот знак религии, приходи позже."));
        LogStream::sendError() << "BUG: no tattoo index data for " << religion->getName() << endl;
        return false;
    }

    return true;
}



/*----------------------------------------------------------------------------
 * PersonalNameRepair
 *---------------------------------------------------------------------------*/
void PersonalNameRepair::initialization( )
{
    eventBus->subscribe( typeid(ItemReadEvent), Pointer(this) );
}

void PersonalNameRepair::destruction( )
{
    eventBus->unsubscribe( typeid(ItemReadEvent), Pointer(this) );
}

void PersonalNameRepair::handleEvent( const type_index &eventType, const Event &event ) const
{
    if (eventType == typeid(ItemReadEvent))
        eventItemRead( static_cast<const ItemReadEvent &>(event) );
}

/** How many substitutions the prototype asks for in this language. */
static int personal_arg_count( const DLString &pattern )
{
    int count = 0;

    for (DLString::size_type pos = pattern.find("%s");
         pos != DLString::npos;
         pos = pattern.find("%s", pos + 2))
        count++;

    return count;
}

/** True when every format spec in the pattern is a plain "%s". Counting "%s"
 *  alone would let a prototype that grew a "%d" through as long as the "%s"
 *  count still matched, and handing printf a string where it wants an int is
 *  undefined. The money objects already carry "%d", so this is not imaginary. */
static bool personal_args_all_strings( const DLString &pattern )
{
    for (DLString::size_type pos = pattern.find('%');
         pos != DLString::npos;
         pos = pattern.find('%', pos + 2))
        if (pos + 1 >= pattern.length( ) || pattern.at(pos + 1) != 's')
            return false;

    return true;
}

/** True when this language slot still needs filling: the instance has nothing
 *  of its own and the prototype behind it is a template we know how to fill. */
static bool personal_slot_needs_repair( Object *obj, lang_t lang )
{
    if (!obj->getRealShortDescr(lang).empty())
        return false;

    const DLString &pattern = obj->pIndexData->short_descr.get(lang);

    return personal_args_all_strings( pattern ) && personal_arg_count( pattern ) > 0;
}

/** Recover which adjective was engraved on a hero item. The buyer's alignment at
 *  purchase time is recorded nowhere else, so read it back off the Russian name.
 *  Returns 0 when the name starts with none of them, which is the signal to
 *  leave the object alone rather than guess. */
static const LangText * personal_engraved_adjective( const DLString &shortRu )
{
    for (int a = 0; a < 3; a++)
        for (int g = 0; g < 3; g++) {
            DLString adjective = HERO_ADJECTIVES[a][g].ru;

            if (adjective.strPrefix( shortRu ))
                return &HERO_ADJECTIVES[a][g];
        }

    return 0;
}

/** The prototypes this repair may touch, and nothing else.
 *
 *  A "%s" in a short descr is not a promise that the missing word is the owner:
 *  across the 34 prototypes in the world that have one it can be a material
 *  (battle poncho), a liquid (puddle), a deity (tattoo), the mob a steak was cut
 *  from, or the victim pictured on an assassin card. An object carrying an owner
 *  field does not settle it either -- the dungeon storage box has both an owner
 *  and a "%s", and they mean the same thing there only by coincidence. So this
 *  is a list of the objects whose template really is "adjective, owner" or
 *  "owner", not a rule inferred from the data.
 *
 *  Hero quest items take two substitutions, hunter clan gear takes one. */
static bool personal_repairable( int vnum )
{
    switch (vnum) {
    case 94: case 95: case 96: case 103:                    // hero girth, ring, weapon, bag
    case 573: case 574: case 575: case 576: case 577:       // hunter clan weapons
    case 578: case 579: case 580: case 581:
    case 582:                                               // hunter breastplate
    case 589:                                               // hunter bow
        return true;
    }

    return false;
}

void PersonalNameRepair::eventItemRead( const ItemReadEvent &event ) const
{
    Object *obj = event.obj;

    if (!personal_repairable( obj->pIndexData->vnum ))
        return;

    if (obj->getOwner( ).empty( ))
        return;

    bool anyGap = false;
    for (int l = LANG_MIN; l < LANG_MAX && !anyGap; l++)
        anyGap = personal_slot_needs_repair( obj, (lang_t)l );

    if (!anyGap)
        return;

    // The owner may well be offline; a memory record declines per language just
    // as a live character does. A record can be genuinely gone (deleted player),
    // and then there is no name to write.
    PCMemoryInterface *owner = PCharacterManager::find( obj->getOwner( ) );
    if (!owner)
        return;

    // Two substitutions means a hero quest item, adjective first and owner
    // second; one means hunter gear, owner only.
    const LangText *adjective = 0;

    if (personal_arg_count( obj->pIndexData->short_descr.get(LANG_DEFAULT) ) > 1) {
        adjective = personal_engraved_adjective( obj->getRealShortDescr(LANG_DEFAULT) );

        if (!adjective) {
            // No adjective means the item was renamed and its Russian slot holds
            // custom text, not an engraving. Dozens of these exist, all restrung
            // before XMLItemRestring::dress learned to write every language
            // (2026-07-24). There is nothing to translate in a custom name, so
            // mirror Russian into the empty slots, exactly as a restring does
            // today. Leaving them empty is not neutral:
            // display falls through to the PROTOTYPE, and the prototype is a
            // template, so the reader gets a raw "%s".
            const DLString &restrung = obj->getRealShortDescr(LANG_DEFAULT);

            if (!restrung.empty( ))
                for (int l = LANG_MIN; l < LANG_MAX; l++)
                    if (obj->getRealShortDescr((lang_t)l).empty( ))
                        obj->setShortDescr( restrung, (lang_t)l );

            return;
        }
    }

    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        if (!personal_slot_needs_repair( obj, lang ))
            continue;

        const DLString &pattern = obj->pIndexData->short_descr.get(lang);
        DLString name = owner->getNameP( '2', lang );

        // Only fill a slot whose template asks for exactly what we have. Together
        // with the all-strings check above, a prototype edited later can neither
        // take the wrong number of arguments nor the wrong kind.
        int args = personal_arg_count( pattern );

        if (adjective && args == 2)
            obj->setShortDescr( fmt(0, pattern.c_str(), adjective->get(lang), name.c_str()), lang );
        else if (!adjective && args == 1)
            obj->setShortDescr( fmt(0, pattern.c_str(), name.c_str()), lang );
    }
}
