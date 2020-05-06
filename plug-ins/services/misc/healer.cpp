/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "healer.h"
#include "behavior_utils.h"
#include "attract.h"
#include "occupations.h"
#include "commandtemplate.h"
#include "hometown.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "clanreference.h"

#include "merc.h"
#include "handler.h"
#include "arg_utils.h"
#include "gsn_plugin.h"
#include "magic.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);
HOMETOWN(frigate);

/*------------------------------------------------------------------------
 * Healer 
 *-----------------------------------------------------------------------*/
Healer::Healer( ) : healPets( false )
{
}

int Healer::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_HEALER);
}

bool Healer::canServeClient( Character *client )
{
    if (client->is_npc( ) && !healPets) {
        say_act( client, ch, "Я не лечу домашних животных." );
        return false;
    }

    if (IS_GHOST( client )) {
        say_act( client, getKeeper( ), "Я исцеляю тела, а не души, $c1. Обрети плоть - тогда и поговорим." );
        return false;
    }

    if (getKeeper( )->fighting) {
        say_act( client, getKeeper( ), "Подожди немного, $c1, мне сейчас не до тебя." );
        return false;
    }
    
    if ((!client->is_npc( ) && client->getClan( ) == clan_battlerager)
        || (client->is_npc( ) && client->master && client->master->getClan( ) == clan_battlerager)) 
    {
        act( "$C1 выразительно крутит пальцем у виска, глядя на $c4.", client, 0, getKeeper( ), TO_NOTVICT );
        client->send_to("Напоминаем: ты BattleRager, а не презренный МАГ!\n\r");
        return false;
    }

    return true;
}

void Healer::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), "Извини, $c1, я ничем не смогу тебе помочь." );
}

void Healer::msgListRequest( Character *client ) 
{
    act( "$c1 просит $C4 рассказать, какие болезни $E может вылечить.", client, 0, getKeeper( ), TO_NOTVICT );
    act( "Ты просишь $C4 рассказать, какие болезни $E может вылечить.", client, 0, getKeeper( ), TO_CHAR );
}

void Healer::msgListBefore( Character *client ) 
{
    tell_dim( client, getKeeper( ), "Я предлагаю следующие заклинания: " );
}

void Healer::msgListAfter( Character *client )
{
    tell_dim( client, getKeeper( ), "Используй '{lEheal{lRлечить{lx <тип заклинания>', и я вылечу тебя за указанную цену." );
    if (client->getModifyLevel() < 20)
        tell_dim(client, getKeeper(), "А пока ты не достигнешь 20го уровня, я буду лечить тебя бесплатно, если замечу, что тебе нужна помощь.");
}

void Healer::msgArticleNotFound( Character *client ) 
{
    say_act( client, getKeeper( ), "Я не знаю такого заклинания, $c1." );
    tell_act( client, getKeeper( ), "Используй '{lEheal{lRлечить{lx', чтобы увидеть список известных мне заклинаний." );
}

void Healer::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), "Я могу вылечить только одну болезнь за один раз." );    
}

void Healer::msgBuyRequest( Character *client )
{
    act( "Ты просишь $C4 о помощи.", client, 0, getKeeper( ), TO_CHAR );
    act( "$c1 просит $C4 о помощи.", client, 0, getKeeper( ), TO_NOTVICT );
}

/*------------------------------------------------------------------------
 * HealService
 *-----------------------------------------------------------------------*/
void HealService::toStream( Character *client, ostringstream &buf ) const
{
    DLString myname = client->getConfig()->rucommands && !rname.empty() ? rname : name;
    DLString myprice = price->toString(client);
    buf << dlprintf("  {c%-11s{x: %-30s  %17s\r\n",
           myname.c_str(), descr.c_str(), myprice.c_str());
}

bool HealService::visible( Character * ) const
{
    return true;
}

bool HealService::available( Character *, NPCharacter * ) const
{
    return true;
}

bool HealService::matches( const DLString &argument ) const
{
    if (argument.empty())
        return false;
    
    return arg_oneof(argument, name.c_str(), rname.c_str());
}

int HealService::getQuantity( ) const
{
    return 1;
}

void HealService::purchase( Character *client, NPCharacter *healer, const DLString &, int )
{
    Character *payer;

    payer = client->is_npc( ) && client->master ? client->master : client;

    if (!price->canAfford( payer )) {
        if (payer == client)
            say_act( client, healer, "У тебя не хватает $n2 оплатить мои услуги.", price->toCurrency( ).c_str( ) );
        else
            say_act( client, healer, "У твоего хозяина не хватает $n2 оплатить мои услуги.", price->toCurrency( ).c_str( ) );
        return;
    }
    
    price->deduct( payer );
    payer->setWaitViolence( 1 );
    heal( client, healer );
}

/*------------------------------------------------------------------------
 * SpellHealService 
 *-----------------------------------------------------------------------*/
void SpellHealService::heal( Character *client, NPCharacter *healer )
{
    ::spell( spell, healer->getModifyLevel( ), healer, client, FSPELL_VERBOSE );
}

bool SpellHealService::available( Character *client, NPCharacter *healer ) const
{
    int sn = skillManager->find(spell.getName())->getIndex();

    if (sn == gsn_master_healing && client->hit >= client->max_hit) {
        say_fmt("Ты не нуждаешься в услуге лечения, %2$C1: ты здоров%2$Gо||а, как %2$Gжеребец|жеребец|кобыла!", healer, client);
        return false;
    }
    else if (sn == gsn_refresh && client->move >= client->max_move) {
        say_fmt("Ты не нуждаешься этой услуге, %2$C1: ты и так пол%2$Gон|но|на сил!", healer, client);
        return false;
    }
    else if (sn == gsn_cure_poison && !IS_AFFECTED(client, AFF_POISON)) {
        say_fmt("Ты не нуждаешься этой услуге, %2$C1: ты не отравлен%2$G|о|а!", healer, client);
        return false;
    }
    else if (sn == gsn_cure_disease && !IS_AFFECTED(client, AFF_PLAGUE)) {
        say_fmt("Ты не нуждаешься этой услуге, %2$C1: ты не заражен%2$G|о|а чумой!", healer, client);
        return false;
    }
    else if (sn == gsn_cure_blindness && !IS_AFFECTED(client, AFF_BLIND)) {
        say_fmt("Ты не нуждаешься этой услуге, %2$C1: твое зрение в порядке!", healer, client);
        return false;
    }
    else if(sn == gsn_cure_blindness && !client->isAffected( gsn_blindness )){
        say_fmt("Я не могу помочь тебе с этим типом слепоты, %2$C1.", healer, client);
        return false;      
    }
    else if (sn == gsn_bless && client->isAffected(sn)) {
        say_fmt("Я не могу благословить тебя еще больше, %2$C1.", healer, client);
        return false;
    }

    return true;
}

/*------------------------------------------------------------------------
 * ManaHealService 
 *-----------------------------------------------------------------------*/
void ManaHealService::heal( Character *client, NPCharacter *healer )
{
    act( "$c1 бормочет '$T'.", healer, 0, words.getValue( ).c_str( ), TO_ROOM );

    client->mana += healer->getModifyLevel() * 5 + number_range(50, 100);
    client->mana = std::min( client->mana, client->max_mana );
    client->send_to( "Приятное тепло наполняет твое тело.\n\r" );
}

bool ManaHealService::available( Character *client, NPCharacter *healer ) const
{
    if (client->mana >= client->max_mana) {
        say_act(client, healer, "Твоя мана и так на максимуме, $c1.");
        return false;
    }
    return true;
}

/*------------------------------------------------------------------------
 * 'heal' command 
 *-----------------------------------------------------------------------*/
static void mprog_heal( Character *mob, Character *client, const DLString &args )
{
    FENIA_VOID_CALL(mob, "Heal", "Cs", client, args.c_str());
    FENIA_NDX_VOID_CALL(mob->getNPC(), "Heal", "CCs", mob, client, args.c_str());
}

CMDRUN( heal )
{
    DLString argument = constArguments;
    Healer::Pointer healer;
    
    healer = find_attracted_mob_behavior<Healer>( ch, OCC_HEALER );

    if (!healer) {
        ch->send_to( "Здесь некому тебя вылечить за деньги.\r\n" );
        if (ch->getModifyLevel() < 20) {
            if (!ch->is_npc( ) && ch->getPC( )->getHometown( ) == home_frigate)
                ch->println("Доктор в лазарете вылечит тебя бесплатно, если заметит, что тебе нужна помощь.");
            else
                ch->println("Лекарь в любом храме вылечит тебя бесплатно, если заметит, что тебе нужна помощь.");
        }
        return;
    }

    if (ch->is_npc( ) && !ch->master) {
        ch->send_to( "Извини, тебя никто обслуживать не будет.\r\n" );
        return;
    }

    if (argument.empty( )) 
        healer->doList( ch );
    else
        healer->doBuy( ch, argument );

    mprog_heal(healer->getChar(), ch, argument);
}

/*------------------------------------------------------------------------
 * CustomHealPrice 
 *-----------------------------------------------------------------------*/
CustomHealPrice::CustomHealPrice()
{
}

int CustomHealPrice::toSilver( Character *ch ) const
{
    int gold = 0;
    int lev = ch->getModifyLevel();
    
    if (lev <= 5) gold = 5;
    else if (lev < 10) gold = 10;
    else if (lev < 15) gold = 15;
    else if (lev < 20) gold = 20;
    else if (lev < 25) gold = 30;
    else if (lev < 30) gold = 40;
    else if (lev < 35) gold = 50;
    else if (lev < 40) gold = 80;
    else if (lev < 45) gold = 100;
    else if (lev < 50) gold = 120;
    else if (lev < 55) gold = 150;
    else if (lev < 60) gold = 200;
    else if (lev < 70) gold = 220;
    else if (lev < 80) gold = 250;
    else if (lev < 90) gold = 270;
    else gold = 300;

    return gold * 100;
}

