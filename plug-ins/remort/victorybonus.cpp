/* $Id$
 *
 * ruffina, 2004
 */
#include "victorybonus.h"
#include "xmlattributestatistic.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*
 * Koschey
 */
void Koschey::greet( Character *victim )
{
    if (victim->is_npc( ))
        return;

}

bool Koschey::command( Character *victim, const DLString &cmdName, const DLString &cmdArgs )
{
    if (victim->is_npc( ))
        return false;

    if (arg_oneof(cmdName, "buy", "купить")) {
        doBuy( victim, cmdArgs.quote( ) );
        return true;
    }

    if (arg_is_list(cmdName)) {
        doList( victim );
        return true;
    }

    if (arg_oneof(cmdName, "sell", "продать") || arg_oneof("value", "цена")) {
        tell_dim( victim, ch, "Мне твое жалкое добро ни к чему." );
        return true;
    }
    
    return false;
}

bool Koschey::canServeClient( Character * )
{
    return true;
}

void Koschey::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, ch, "А будешь жадничать - отгрызу руку." );
}

void Koschey::msgListEmpty( Character *client )
{
    say_act( client, ch, "Ступай отсюда, $c1, подобру-поздорову." );
}

void Koschey::msgListAfter( Character *client )
{
    tell_dim( client, ch, "Это все. Бери то, за чем приш$gло|ел|ла, и чтоб духу твоего здесь не было!" );
    oldact("$C1 что-то монотонно цедит сквозь зубы, обращаясь к $c3.", client, 0, ch, TO_NOTVICT );
}

void Koschey::msgListBefore( Character *client )
{
    oldact("$C1 поднимает на тебя раздраженный взгляд.", client, 0, ch, TO_CHAR );
    oldact("$C1 поднимает на $c4 раздраженный взгляд.", client, 0, ch, TO_ROOM );
    oldact("$C1 скрипучим голосом произносит '{gВот какое счастье тебе привалило:{x'", client, 0, ch, TO_CHAR );
}

void Koschey::msgBuyRequest( Character *client )
{
    oldact("$c1 торгуется с $C5.", client, 0, ch, TO_NOTVICT );
}

void Koschey::msgArticleNotFound( Character *client )
{
    oldact("$C1 в ярости восклицает '{g$c1, ты сюда шутки приш$gло|ел|ла шутить?!{x'", client, 0, ch, TO_ALL );
}

/*
 * VictoryPrice
 */
const int VictoryPrice::COUNT_PER_LIFE = 500;
const DLString VictoryPrice::CURRENCY_NAME = "побед|ы||ам|ы|ами|ах";

DLString VictoryPrice::toCurrency( ) const
{
    return CURRENCY_NAME;
}

DLString VictoryPrice::toString( Character * ) const
{
    DLString str;

    str << count << " " << GET_COUNT( count, "победа", "победы", "побед" );
    return str;
}

bool VictoryPrice::canAfford( Character *ch ) const
{
    XMLAttributeStatistic::Pointer attr;
    int avail;
    
    if (ch->is_npc( ))
        return false;
        
    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeStatistic>( "questdata" );
    if (!attr)
        return false;

    int bonusLifes = ch->getPC()->getRemorts().countBonusLifes();
    int bonusLifesLeft = Remorts::MAX_BONUS_LIFES - bonusLifes;
    int questVictories = attr->getBonusVictoriesCount();
    // Ensure that bonuses from victories plus bonuses from remorts never exceed MAX_BONUS_LIFES.
    avail = min(bonusLifesLeft * COUNT_PER_LIFE, questVictories);

    return avail - attr->getWasted( ) >= count.getValue( );
}

void VictoryPrice::deduct( Character *ch ) const
{
    if (!ch->is_npc( )) {
        XMLAttributeStatistic::Pointer attr;
        
        attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeStatistic>( "questdata" );
        attr->setWasted( attr->getWasted( ) + count.getValue( ) );
    }
}

void VictoryPrice::induct( Character *ch ) const
{
}

void VictoryPrice::toStream( Character *ch, ostringstream &buf ) const
{
    buf << toString( ch );
}

