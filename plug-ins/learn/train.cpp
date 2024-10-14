/* $Id: train.cpp,v 1.1.2.2.6.10 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */

#include "train.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commandtemplate.h"
#include "hometown.h"
#include "attract.h"
#include "occupations.h"
#include "core/behavior/behavior_utils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

static const int conPriceQP = 150;
HOMETOWN(frigate);

static bool mprog_cant_train( PCharacter *client, NPCharacter *trainer )
{
    FENIA_CALL( trainer, "CantTrain", "C", client );
    FENIA_NDX_CALL( trainer, "CantTrain", "CC", trainer, client );
    return false;
}

Trainer::Trainer( )
{
}

int Trainer::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_TRAINER);
}

void Trainer::doGain( PCharacter *client, DLString & argument )
{
    DLString arg;
    
    if (mprog_cant_train( client, ch ))
        return;

    arg = argument.getOneArgument( );
    
    if (arg_is(arg, "revert")) {
        if (client->train < 1) {
            tell_act( client, ch, "У тебя нет тренировочных сессий." );
            return;
        }

        oldact("$C1 дает тебе {Y10{x практик в обмен на {Y1{x тренировочную сессию.", client, 0, ch, TO_CHAR);
        oldact("$C1 обменивает для $c2 тренировочные сессии на сессии практики.", client, 0, ch, TO_NOTVICT );

        client->practice += 10;
        client->train -=1 ;
        return;
    }

    if (arg_is(arg, "convert")) {
        if (client->practice < 10) {
            tell_act( client, ch, "У тебя недостаточно практик -- надо 10." );
            return;
        }

        oldact("$C1 дает тебе {Y1{x тренировочную сессию в обмен на {Y10{x практик.", client, 0, ch, TO_CHAR );
        oldact("$C1 обменивает для $c2 сессии практики на тренировочные сессии.", client, 0, ch, TO_NOTVICT );

        client->practice -= 10;
        client->train +=1 ;
        return;
    }
}

void Trainer::doTrain( PCharacter *client, DLString & argument )
{
    int cost; 
    DLString argStat, argQP;
    int stat;
    bool fQP;
    
    argStat = argument.getOneArgument( );
    argQP = argument.getOneArgument( );
    stat = -1;
    cost = 1;
    fQP = false;
    
    if (mprog_cant_train( client, ch ))
        return;

    if (!argQP.empty( )) {
        if (argQP != "qp" && argQP != "кп") {
            tell_raw( client, ch, "Чтобы тренироваться за квестовые единицы, напиши {hcтренировать тело кп{x." );
            return;
        }
        else
            fQP = true;
    }

    for (int i = 0; i < stat_table.size; i++)
        if (argStat.strPrefix( stat_table.fields[i].name )
            || argStat.strPrefix( russian_case(stat_table.fields[i].message, '1') )
            || argStat.strPrefix( russian_case(stat_table.fields[i].message, '4') ))
        {
            stat = i;
            break;
        }
    
    if (stat == -1) {
        tell_raw(client, ch, "Я не понимаю, чего ты хочешь.");
        showTrain(client);
        showGain(client);
        return;
    }

    if (client->perm_stat[stat_table.fields[stat].value] 
        >= client->getMaxTrain( stat_table.fields[stat].value )) 
    {
        tell_raw( client, ch, "Ты не можешь дальше тренировать %s (%s).", 
                  russian_case( stat_table.fields[stat].message, '4' ).c_str( ), 
                  stat_table.fields[stat].name );
        return;
    }
    
    if (fQP) {
        if (stat != STAT_CON) {
            tell_raw( client, ch, "За квестовые очки можно тренировать только телосложение." );
            return;
        }

        if (conPriceQP > client->getQuestPoints()) {
            tell_raw( client, ch, "У тебя недостаточно квестовых единиц." );
            return;
        }
    } else if (cost > client->train) {
        tell_raw( client, ch, "У тебя недостаточно тренировочных сессий." );
        return;
    }
    
    if (fQP)
        client->addQuestPoints(-conPriceQP);
    else
        client->train -= cost;

    client->perm_stat[stat_table.fields[stat].value] += 1;

    oldact("Ты повышаешь {W$N4{x!", client, 0, stat_table.fields[stat].message, TO_CHAR );
    oldact("$c1 повышает {W$N4!{x", client, 0, stat_table.fields[stat].message, TO_ROOM );
}


void Trainer::showTotal(PCharacter *client)
{
    tell_raw( client, ch, "У тебя {Y%d{G тренировочн%s.", 
              client->train.getValue( ),
              GET_COUNT(client->train,"ая сессия","ые сессии","ых сессий") );
}

void Trainer::showTrain(PCharacter *client)
{
    ostringstream buf;
    int cnt = 0;

    for (int i = 0; i < stat_table.size; i++)
        if (client->perm_stat[stat_table.fields[i].value] 
                < client->getMaxTrain( stat_table.fields[i].value))
        {
            buf << russian_case( stat_table.fields[i].message, '4' )
                << "(" << stat_table.fields[i].name << ") ";
            cnt++;
        }
    
    if (cnt > 0) {
        tell_raw( client, ch, 
                  "Ты можешь тренировать:%s%s", 
                  (cnt > 3 ? "\r\n" : " " ),
                  buf.str( ).c_str( ) );
        
        bool hideQpPrice = client->getHometown() == home_frigate 
                            && client->getRemorts().size() == 0
                            && client->getQuestPoints() < conPriceQP;
        
        if (client->perm_stat[STAT_CON] < client->getMaxTrain( STAT_CON ) && !hideQpPrice)
            tell_raw( client, ch, "Ты можешь повысить телосложение за {Y%d{G квестовых единиц: {hcтренировать сложение кп{x.", conPriceQP );
    }            
    else {
        /*
         * This message dedicated to Jordan ... you big stud!
         */
        tell_raw( client, ch, "Тебе больше нечего тренировать, %s!",
                  GET_SEX(client, "жеребчик", "дикое животное", "красотка") );
    }
}

void Trainer::showGain(PCharacter *client)
{
    if (client->practice >= 10)
        tell_act( client, ch, "Можно обменять {Y10{G практик на {Y1{G тренировочную сессию: {hcтренировки купить{x." );
    
    if (client->train >= 1)
        tell_act( client, ch, "Можно обменять {Y1{G тренировку на {Y10{G практик: {hcтренировки продать{x" );  
}

CMDRUN( train )
{
    Trainer::Pointer trainer;
    DLString argument = constArguments;

    if (ch->is_npc( )) {
        ch->pecho( "Это бесполезно для тебя." );
        return;
    }
    
    trainer = find_attracted_mob_behavior<Trainer>( ch, OCC_TRAINER );

    if (!trainer) {
        ch->pecho( "Здесь некому тренировать тебя." );
        return;
    }

    if (argument.empty()) {
        trainer->showTotal(ch->getPC());
        trainer->showTrain(ch->getPC());
        trainer->showGain(ch->getPC());
        return;
    }
    
    if (arg_is(argument, "revert")
        || arg_is(argument, "buy")
        || (argument.strPrefix("convert") && argument != "con"))

    {
        trainer->doGain( ch->getPC( ), argument );
        return;
    }
    
    trainer->doTrain( ch->getPC( ), argument );
}

