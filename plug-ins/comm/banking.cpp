/* $Id$
 *
 * ruffina, 2004
 */
#include "banking.h"
#include "arg_utils.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "act.h"
#include "descriptor.h"

/*------------------------------------------------------------------------
 * BankAction
 *-----------------------------------------------------------------------*/
bool BankAction::handleCommand( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    DLString args = cmdArgs;
    
    if (ch->is_npc( ))
        return false;
    
    if (cmdName == "balance") {
        doBalance( ch->getPC( ) );
        return true;
    }
    
    if (cmdName == "deposit") {
        doDeposit( ch->getPC( ), args );
        return true;
    }

    if (cmdName == "withdraw") {
        doWithdraw( ch->getPC( ), args );
        return true;
    }

    return false;
}

/*
 * 'balance' command
 */
void BankAction::doBalance( PCharacter *ch )
{
    long bank_g, bank_s;

    if ( ch->bank_s + ch->bank_g == 0 )  {
        ch->pecho("У тебя нет никаких денег в банке.");
        return;
    }

    bank_g = ch->getPC( )->bank_g;
    bank_s = ch->getPC( )->bank_s;

    if  (bank_g!=0 && bank_s!=0)
        ch->pecho( "У тебя %ld золот%s и %ld серебрян%s в банке.",
         bank_g,GET_COUNT(bank_g,"ая","ые","ых"),
         bank_s,GET_COUNT(bank_s,"ая монета","ые монеты","ых монет"));

    if  (bank_g!=0 && bank_s == 0)
        ch->pecho( "У тебя %ld золот%s в банке.",
         bank_g,GET_COUNT(bank_g,"ая монета","ые монеты","ых монет"));

    if  (bank_g == 0 && bank_s!= 0)
        ch->pecho( "У тебя %ld серебрян%s в банке.",
         bank_s,GET_COUNT(bank_s,"ая монета","ые монеты","ых монет"));
}

/*
 * 'withdraw' command
 */
void BankAction::doWithdraw( PCharacter *ch, DLString &arguments )
{
    ostringstream buf;
    int amount_s, amount_g, amount;
    DLString argOne, argTwo;
    
    argOne = arguments.getOneArgument( );
    argTwo = arguments.getOneArgument( );
    amount_s = amount_g = amount = 0;
    
    if (argOne.empty( ) 
        || argTwo.empty( )
        || !argOne.isNumber( ) 
        || (!arg_is_gold( argTwo) && !arg_is_silver( argTwo )))
    {
        ch->pecho( "Укажи сумму и денежную единицу. Например: '{lRсосчета 77 золота{lEwithdraw 77 gold{x' или '{lRсосчета 9000 серебра{lEwithdraw 9000 silver{lx'.");
        return;
    }
    
    try {
        amount = argOne.toInt( );
    } catch (const ExceptionBadType &) {
        ch->pecho( "Сумма указана неверно." );
        return;
    }

    if (amount <= 0) {
        ch->pecho( "Очень щедро." );
        return;
    }

    if (arg_is_silver( argTwo ))
        amount_s = amount;
    else
        amount_g = amount;
    
    if (amount_g > ch->bank_g || amount_s > ch->bank_s) {
        ch->pecho("Извини, мы не даем взаймы.");
        return;
    }

    if (amount_s > 0 )
    {
        if (amount_s < 10)
        {
            if (amount_s == 1)
                buf << "Одну";
            else
                buf << amount_s;

            buf << " моне" << GET_COUNT(amount_s, "ту", "ты", "т")
                << "?! Не мало?!" << endl;
        }
        else
        {
            int fee = max( 1, amount_s / 10 );
            ch->bank_s -= amount_s;
            ch->silver += amount_s - fee;

            buf << "Ты снимаешь со счета " << amount_s 
                << " серебрян" << GET_COUNT(amount_s,"ую монету","ые монеты","ых монет")
                << "." << endl << "Услуги банка составили " << fee 
                << " серебрян" << GET_COUNT(fee,"ую монету","ые монеты","ых монет") << endl;
        }
    }

    if (amount_g > 0)
    {
        if (amount_g == 1)
        {
            buf << "Одну золотую моне" << GET_COUNT(amount_g, "ту", "ты", "т")
                << "?! Не мало?!" << endl;
        }
        else
        {
            int fee = max( 1, amount_g / 50 );
            ch->bank_g -= amount_g;
            ch->gold += amount_g - fee;

            buf << "Ты снимаешь со счета " << amount_g 
                << " золот" << GET_COUNT(amount_g,"ую монету","ые монеты","ых монет")
                << "." << endl << "Услуги банка составили " << fee 
                << " золот" << GET_COUNT(fee,"ую монету","ые монеты","ых монет") << endl;
        }
    }

    ch->send_to(buf);
    oldact("$c1 производит банковские операции.",ch,0,0,TO_ROOM);
}

/*
 * 'deposit' command
 */
void BankAction::doDeposit( PCharacter *ch, DLString &arguments )
{
    std::ostringstream mbuf, cbuf, vbuf;
    PCharacter *victim;
    long amount, amount_s, amount_g;
    DLString arg;

    if (arguments.empty( )) {
        ch->pecho("Положить на счет сколько?");
        return;
    }
    
    amount_s = amount_g = 0;
    victim = ch->getPC( );

    while (!( arg = arguments.getOneArgument( ) ).empty( )) {
        if (arg.isNumber( )) {
            try {
                if (( amount = arg.toInt( ) ) <= 0)
                    throw Exception( );
            }
            catch (const Exception &) {
                ch->pecho("Сумма указана неверно.");
                return;
            }

            if (( arg = arguments.getOneArgument( ) ).empty( )) {
                ch->pecho("Укажи денежную единицу: {lRзолото или серебро{lEgold или silver{lx.");
                return;
            }

            if (arg_is_silver( arg ) && amount_s == 0)
                amount_s = amount;
            else if (arg_is_gold( arg ) && amount_g == 0)
                amount_g = amount;
            else {
                ch->pecho("Ты можешь положить на счет только золотые или серебряные монеты.\r\n"
                            "Пример: {lEdeposit 3 silver 18 gold{lRнасчет 3 серебра 18 золота{lx" );
                return;
            }
        }
        else if (victim != ch) {
            ch->pecho("Двоим сразу давать деньги не стоит. Подерутся еще..");
            return;
        }
        else {
            PCMemoryInterface *pci = PCharacterManager::find( arg );

            if (!pci) {
                ch->pecho("Адресат перевода не найден. Укажи имя правильно и полностью.");
                return;
            }

            if (!( victim = dynamic_cast<PCharacter *>( pci ) ) || !ch->can_see( victim )) {
                ch->pecho("Невидимый или отсутствующий в мире адресат не оценит твоей щедрости.");
                return;
            }
        }
    }


    if (amount_g > ch->gold || amount_s > ch->silver) {
        ch->pecho("Это больше, чем есть у тебя в кошельке.");
        return;
    }

    if ( (amount_g + victim->getPC( )->bank_g) > 100000 ) {
        ch->pecho("Размер банковского счета не может превышать 100.000 золотых.");
        return;
    }

    victim->getPC( )->bank_s += amount_s;
    victim->getPC( )->bank_g += amount_g;
    ch->gold -= amount_g;
    ch->silver -= amount_s;
    
    if (victim == ch)
        cbuf << "На твой банковский счет ";
    else {
        cbuf << "На банковский счет {W" << victim->getName( ) << "{x ";
        vbuf << "{W" << ch->getName( ) << "{x переводит на твой банковский счет ";
    }
    
    cbuf << "переведено: ";
    
    if (amount_g > 0) 
        mbuf << "{Y" << amount_g << "{x золот" << GET_COUNT(amount_g,"ая монета","ые монеты","ых монет");
    
    if (amount_s > 0) {
        if (amount_g > 0)
            mbuf << " и ";

        mbuf << "{W" << amount_s << "{x серебрян" << GET_COUNT(amount_s,"ая монета","ые монеты","ых монет");
    }
    
    cbuf << mbuf.str( ) << "." << endl;
    vbuf << mbuf.str( ) << "." << endl;
    ch->send_to( cbuf );
    
    if (victim != ch) 
        victim->send_to( vbuf );

    oldact("$c1 производит банковские операции.", ch, 0, 0, TO_ROOM );
}

/*------------------------------------------------------------------------
 * BankRoom
 *-----------------------------------------------------------------------*/
bool BankRoom::command( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    return handleCommand( ch, cmdName, cmdArgs );
}

/*------------------------------------------------------------------------
 * CreditCard
 *-----------------------------------------------------------------------*/
bool CreditCard::command( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    return handleCommand( ch, cmdName, cmdArgs );
}

bool CreditCard::hasTrigger( const DLString &t )
{
    return (t == "withdraw" || t == "deposit");
}

/*------------------------------------------------------------------------
 * TaxesListener
 *-----------------------------------------------------------------------*/
void TaxesListener::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;

    if (!d->character || !( ch = d->character->getPC( ) ))
        return;

    if (oldState != CON_READ_MOTD || newState != CON_PLAYING) 
        return;

    if (ch->is_immortal( ))
        return;
    
    if (ch->gold > 6000) {
        ch->pecho("С тебя взыскали %d золотых налога на оплату адвокатов мэрии.",
                   (ch->gold - 6000) / 2);
        ch->gold -= (ch->gold - 6000) / 2;
    }
    

    if (ch->bank_g + ch->bank_s / 100 > 80000) {
        long silver;
        
        silver = ch->bank_g + ch->bank_s / 100 - 20000;
        
        ch->pecho( "С тебя взыскали %ld золотых налога на оплату военных расходов Султана.", silver );
    
        if( silver < ch->bank_s / 100 ) {
            ch->bank_s -= ( silver * 100 );
        } else {
            silver -= ( ch->bank_s / 100 );
            ch->bank_s %= 100;
            ch->bank_g -= silver;
        }
    }
}

