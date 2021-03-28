/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>

#include "questtrader.h"
#include "xmlattributequestreward.h"
#include "xmlattributequestdata.h"
#include "occupations.h"
#include "defaultreligion.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "merc.h"
#include "mercdb.h"
#include "arg_utils.h"
#include "wiznet.h"
#include "interp.h"
#include "handler.h"
#include "act.h"
#include "def.h"

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
        tell_act( client, getKeeper( ), "Какую именно вещь ты хочешь вернуть?" );
        return;
    }

    article = findArticle( client, arg );

    if (!article) {
        msgArticleNotFound( client );
        return;
    }
    
    personal = article.getDynamicPointer<PersonalQuestArticle>( );
    
    if (!personal)
        tell_act( client, getKeeper( ), "Извини, $c1, я не могу вернуть тебе эту вещь." );
    else
        personal->trouble( client, getKeeper( ) );
}

bool QuestTrader::canServeClient( Character *client )
{
    if (client->is_npc( ))
        return false;

    if (IS_GHOST( client )) {
        say_act( client, getKeeper( ), "Наслажденье жизнью недоступно призракам." );
        return false;
    }

    if (IS_CHARMED(client)) {
        say_act( client, getKeeper( ), "Ты не можешь сделать этого, пока ты не владеешь собой!" );
        return false;
    }
   
    if (getKeeper( )->fighting) {
        say_act( client, getKeeper( ), "Подожди немного, $c1, мне сейчас не до тебя." );
        return false;
    }

    if (!getKeeper( )->can_see( client )) {
        say_act( client, getKeeper( ), "Я не общаюсь с невидимками." );
        return false;
    }
    
    return true;
}

void QuestTrader::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), "Извини, $c1, мне нечего тебе предложить." );
}

void QuestTrader::msgListRequest( Character *client ) 
{
    oldact("$c1 просит $C4 показать список вещей.", client, 0, getKeeper( ), TO_ROOM );
    oldact("Ты просишь $C4 показать список вещей.", client, 0, getKeeper( ), TO_CHAR );
}

void QuestTrader::msgListBefore( Character *client ) 
{
    client->pecho("Перечень квестовых вещей для покупки:");
}

void QuestTrader::msgListAfter( Character *client )
{
    client->pecho( "Для покупки чего-либо используйте {y{lRквест купить{lEquest buy{lx {Dвещь{x." );
}

void QuestTrader::msgArticleNotFound( Character *client ) 
{
    say_act( client, getKeeper( ), "У меня нет этого, $c1." );
}

void QuestTrader::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), "Не жадничай." );
}

void QuestTrader::msgBuyRequest( Character *client )
{
    oldact("$c1 о чем-то просит $C4.", client, 0, getKeeper( ), TO_ROOM );
}

/*----------------------------------------------------------------------------
 * QuestTradeArticle 
 *---------------------------------------------------------------------------*/
void QuestTradeArticle::toStream( Character *client, ostringstream &buf ) const
{
    DLString myname = client->getConfig().rucommands && !rname.empty() ? rname : name;
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
        say_act( client, questman, "Извини, $c1, но у тебя недостаточно $n2 для этого.",
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
    obj->setOwner( client->getNameP( ) );
    
    buyObject( obj, client, questman );

    oldact("$C1 дает $o4 $c3.", client, obj, questman, TO_ROOM );
    oldact("$C1 дает тебе $o4.", client, obj, questman, TO_CHAR );
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

void PersonalQuestArticle::buyObject( Object *obj, PCharacter *client, NPCharacter *questman ) 
{
    switch (gender.getValue( )) {
    default:
    case SEX_NEUTRAL:
        obj->fmtShortDescr( obj->getShortDescr( ),
            IS_GOOD(client)    ? "Священн|ое|ого|ому|ое|ым|ом" :
            IS_NEUTRAL(client) ? "Мерцающ|ее|его|ему|ее|им|ем" :
                                      "Дьявольск|ое|ого|ому|ое|им|ом", 
            client->getNameP( '2' ).c_str());
        break;
    case SEX_MALE:
        obj->fmtShortDescr( obj->getShortDescr( ),
            IS_GOOD(client)    ? "Священн|ый|ого|ому|ый|ым|ом" :
            IS_NEUTRAL(client) ? "Мерцающ|ий|его|ему|ий|им|ем" :
                                 "Дьявольск|ий|ого|ому|ий|им|ом", 
            client->getNameP( '2' ).c_str());
        break;
    case SEX_FEMALE:
        obj->fmtShortDescr( obj->getShortDescr( ),
            IS_GOOD(client)    ? "Священн|ая|ой|ой|ую|ой|ой" :
            IS_NEUTRAL(client) ? "Мерцающ|ая|ей|ей|ую|ей|ей" :
                                 "Дьявольск|ая|ой|ой|ую|ой|ой", 
            client->getNameP( '2' ).c_str());
        break;
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
        tell_act( client, questman, "Извини, $c1, я не могу вернуть тебе эту вещь." );
        return;
    }
    
    obj = get_obj_world_unique( vnum, client );

    if (obj) {
        tell_act( client, questman, "Извини, но у тебя уже есть $o1.", obj );
        // extract_obj( obj ); у вас все было 
        return;
    }

    buy( client, questman );
    tell_act( client, questman, "Я возвращаю тебе эту вещь $t-й раз.", DLString( count ).c_str( ) );
    
    if (count == 3) 
        tell_act( client, questman, "Будь внимательнее! В следующий раз я не смогу помочь тебе." );
    
    attr->setCount( vnum, count + 1 );
}

/*---------------------------------------------------------------------------
 * GoldQI 
 *---------------------------------------------------------------------------*/
void GoldQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    client->gold += amount.getValue( );
    
    oldact("$C1 дает $t золотых монет $c3.", client, DLString(amount.getValue( )).c_str( ), questman, TO_ROOM );
    oldact("$C1 дает тебе $t золотых монет.", client, DLString(amount.getValue( )).c_str( ), questman, TO_CHAR );
}

/*---------------------------------------------------------------------------
 * ConQI 
 *---------------------------------------------------------------------------*/
void ConQuestArticle::buy( PCharacter *client, NPCharacter *questman ) 
{
    client->perm_stat[STAT_CON]++;

    oldact("$C1 повышает сложение $c2.", client, 0, questman, TO_ROOM );
    oldact("$C1 повышает твое сложение.", client, 0, questman, TO_CHAR );
}
    
bool ConQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (client->perm_stat[STAT_CON] < client->getPC( )->getMaxTrain( STAT_CON ))
        return true;
    
    say_act( client, questman, "Извини, $c1, но твое сложение на максимуме." );
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
        oldact("$C1 пришивает карманы на $o4.", client, obj, questman, TO_CHAR);
        oldact("$C1 пришивает $c5 карманы на $o4.", client, obj, questman, TO_ROOM);
    }
}

bool PocketsQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (findBag( client->getPC( ) )) 
        return true;

    say_act( client, questman, "Извини, $c1, но я не вижу у тебя сумки без карманов." );
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
    keyring->setShortDescr( girth->getShortDescr( ) );
    
    if (girth->getRealDescription( ))
        keyring->setDescription( girth->getRealDescription( ) );
        
    if (girth->getRealName( ))
        keyring->setName( girth->getRealName( ) );
        
    if (girth->getRealMaterial( ))
        keyring->setMaterial( girth->getRealMaterial( ) );

    keyring->extra_flags = girth->extra_flags;
    keyring->condition = girth->condition;
    keyring->level = girth->level;

    for (auto &paf: girth->affected)
        affect_to_obj( keyring, paf );

    for (EXTRA_DESCR_DATA *ed = girth->extra_descr; ed != 0; ed = ed->next)
        keyring->addExtraDescr( ed->keyword, ed->description );

    waist = &*girth->wear_loc;
    waist->unequip( girth );
    waist->equip( keyring );
    extract_obj( girth );

    oldact("$C1 прикрепляет огромный брелок к $o3.", client, keyring, questman, TO_CHAR);
    oldact("$C1 прикрепляет огромный брелок к $o3.", client, keyring, questman, TO_ROOM);
}

bool KeyringQuestArticle::available( Character *client, NPCharacter *questman ) const 
{
    if (client->is_npc( ))
        return false;

    if (get_obj_carry_vnum( client, OBJ_VNUM_QUESTGIRTH ))
        return true;

    say_act( client, questman, "Извини, $c1, но я не вижу у тебя пояса без брелков." );
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

DLString OwnerPrice::toString( Character * ) const
{
    ostringstream buf;

    buf << lifes << " " << LIFE_NAME << " или " << victories << VICTORY_NAME;
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
    
    my_victories = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getAllVictoriesCount( );
    my_lifes = ch->getPC( )->getRemorts( ).size( );
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
        ::wiznet( WIZ_QUEST, 0, 0, "%1$^C1 приобретает owner coupon.", ch );
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
        say_act( client, questman, "Извини, $c1, но у тебя не хватает $n2, чтобы владеть этой вещью.", lifePrice.toCurrency( ).c_str( ) );
        return false;
    }

    if (!lifePrice.canAfford( client )) {
        say_act( client, questman, "Извини, $c1, но ты уже исчерпа$gло|л|ла отведенное тебе количество этих талонов." );
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
    lifePrice.deduct( client );
}

/*----------------------------------------------------------------------------
 * PiercingQuestArticle 
 *---------------------------------------------------------------------------*/
void PiercingQuestArticle::buy( PCharacter *client, NPCharacter *tattoer ) 
{
    client->wearloc.set( wear_ears );
    
    oldact("$C1 делает дырку в голове $c2.",client,0,tattoer,TO_ROOM);
    oldact("$C1 делает тебе дырку в голове.",client,0,tattoer,TO_CHAR);
}

bool PiercingQuestArticle::available( Character *client, NPCharacter *tattoer ) const 
{
    if (!visible( client )) {
        say_act( client, tattoer, "У тебя уже проколоты уши, $c1." );
        say_act( client, tattoer, "Может, тебе еще что-нибудь проколоть?" );
        interpret_raw( tattoer, "smirk" );
        return false;
    }

    if (get_eq_char( client, wear_head )) {
        interpret_raw( tattoer, "bonk", client->getNameP( ) );
        say_act( client, tattoer, "Шляпу сними!" );
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
    const char *leader = client->getReligion( )->getShortDescr( ).c_str( );

    // Use tattoo vnum from religion profile if specified, otherwise use default one.
    DefaultReligion *religion = dynamic_cast<DefaultReligion *>(client->getReligion().getElement());
    int tattooVnum = 0;
    if (religion)
        tattooVnum = religion->tattooVnum;

    if (tattooVnum == 0)
        tattooVnum = OBJ_VNUM_TATTOO;

    obj = create_object( get_obj_index( tattooVnum ), 0 );
    obj->fmtName( obj->getName( ), leader );
    obj->fmtShortDescr( obj->getShortDescr( ), leader );

    obj_to_char( obj, client );
    equip_char( client, obj, wear_tattoo );
    
    oldact("$C1 наносит тебе $o4!", client, obj, tattoer, TO_CHAR );
    oldact("$C1 наносит $c3 $o4!", client, obj, tattoer, TO_ROOM );
}

bool TattooQuestArticle::available( Character *client, NPCharacter *tattoer ) const 
{
    if (client->is_npc( ))
        return false;

    if (client->getReligion( ) == god_none) {
        say_act( client, tattoer, "$c1, ты не веришь в бога и не можешь получить знак религии." );
        return false;
    }

    if (wear_tattoo->find( client )) {
        say_act( client, tattoer, "Но у тебя уже есть знак религии, $c1!" );
        return false;
    }

    DefaultReligion *religion = dynamic_cast<DefaultReligion *>(&client->getReligion());
    if (religion && religion->tattooVnum != 0 && !get_obj_index(religion->tattooVnum)) {
        say_act(client, tattoer, "Я не могу сейчас нанести тебе этот знак религии, приходи позже.");
        LogStream::sendError() << "BUG: no tattoo index data for " << religion->getName() << endl;
        return false;
    }

    return true;
}


