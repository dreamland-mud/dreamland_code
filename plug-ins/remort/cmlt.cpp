/* $Id: cmlt.cpp,v 1.1.2.6.4.7 2009/01/01 14:13:18 rufina Exp $
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

#include "cmlt.h"
#include "commonattributes.h"
#include "xmlattributestatistic.h"
#include "victorybonus.h"

#include "class.h"

#include "remortdata.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "pcrace.h"

#include "merc.h"
#include "comm.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

COMMAND(CMlt, "mlt")
{
    PCMemoryInterface *pcm;
    DLString arguments = constArguments; 
    DLString arg = arguments.getOneArgument( );
    
    if (ch->is_npc( )) 
        return;
    
    if (ch->isCoder( ) && !arg.empty( )) {
        try {
            doCount( ch, arg.toInt( ) );
            return;
        } catch (const ExceptionBadType &e) {
        }
    }
    
    pcm = ch->getPC( );
    
    if (!arg.empty( )) 
        if (!( pcm = PCharacterManager::find( arg ) )) {
            ch->println( "В DreamLand нет жителя с таким именем." );
            return;
        }

    if (!ch->is_immortal( ) && ch->getPC( ) != pcm ) {
        ch->send_to("Эта информация скрыта от Вас.\n\r");
        return;
    }
    
    if (pcm != ch->getPC( )) 
        doShowOther( ch, pcm );
    else
        doShowSelf( ch->getPC( ) );
}

void CMlt::doShowOther( Character *ch, PCMemoryInterface *pcm )
{
    Remorts &r = pcm->getRemorts( );

    ch->send_to("{B Раса         Класс        Время игры   Бонус{x\n\r");

    for (unsigned int i = 0; i < r.size( ); i++) {
        LifeData &life = r[i];

        ch->printf( " %-12s %-12s     %5ld      %s\r\n", 
                    life.race.getValue( ).c_str( ),
                    life.classCh.getValue( ).c_str( ),
                    life.time.getValue( ),
                    (life.bonus ? "*" : ""));
    }
    
    for (int i = 0; i < stat_table.size; i++)
        ch->printf("%d %s ", r.stats[i].getValue( ), stat_table.name( i ).c_str( ) );

    ch->printf("\n%d lvl, %d hp, %d mana, %s, %d owners\n",
                r.level.getValue( ),
                r.hp.getValue( ), r.mana.getValue( ),
                (r.pretitle ? "pretitle" : ""),
                r.owners.getValue( ));
}

void CMlt::doShowSelf( PCharacter *ch )
{
    std::basic_ostringstream<char> str;

    XMLAttributeStatistic::Pointer attr = ch->getAttributes( ).findAttr<XMLAttributeStatistic>( "questdata" );
    int victories = attr ? attr->getAllVictoriesCount( ) : 0;
    int vasted = attr ? attr->getVasted( ) : 0;
    
    if (victories > 0) {
        str << fmt( ch, 
                    "Всего ты выполни%1$Gло|л|ла {W%2$d{x персональн%2$Iый|ых|ых квес%2$Iт|та|тов",
                    ch, victories );
        
        if (vasted)
            str << ", обменяв {W" << vasted << "{x из этих побед на плюшки";
        else if (victories >= VictoryPrice::COUNT_PER_LIFE)  
            str << ", пока не обменяв ни одну победу на плюшки";

        str << "." << endl;
    }
    else
        str << fmt( ch, "Ты пока не выполни%1$Gло|л|ла ни одного персонального квеста.", ch )
            << endl;
    
    Remorts &r = ch->getRemorts( );
    int r_cnt = r.size( ), b_cnt = r.countBonusLifes( );

    if (r_cnt > 0) {
        str << fmt( ch, "Ты прожи%1$Gло|л|ла {W%2$d{x жизн%2$Iь|и|ей, ", ch, r_cnt );

        if (r_cnt == b_cnt) {
            if (r_cnt == 1)
                str << "которая дает право на бонусы";
            else
                str << "{Wвсе{x из которых дают право на бонусы";
        }
        else
            str << "{W" << b_cnt << "{x из которых "
                << (b_cnt > 1 ? "дают" : "дает")
                << " право на бонусы";
        str << ":" << endl; 

        for (int i = 0; i < r_cnt; i++) {
            LifeData &life = r[i];
            PCRace *race = raceManager->find( life.race )->getPC( );
            int age = 17 + life.time / 20;

            str << fmt( ch,
                        "     %N1 %N1, переродил%Gось|ся|ась в возрасте %d %s",
                        (ch->getSex( ) == SEX_FEMALE ?
                              race->getFemaleName( ).c_str( )
                            : race->getMaleName( ).c_str( )),
                        professionManager->find( life.classCh )->getRusName( ).c_str( ),
                        ch,
                        age, GET_COUNT(age, "года", "лет", "лет") )
                << endl;
        }

        str << endl;
    }
    else
        str << "Ты живешь первую жизнь." << endl;
    
    if (r_cnt > 0 || vasted > 0) {
        str << endl
            << fmt( ch, "Ты выкупи%1$Gло|л|ла {W%2$d{x owner купо%2$Iн|на|нов и выбра%1$Gло|л|ла в качестве бонусов:", 
                        ch, r.owners.getValue( ) ) 
            << endl;

        str << (r.hp > 0          ? fmt( ch, "     %d здоровья\n", r.hp.getValue( ) ) : "")
            << (r.mana > 0        ? fmt( ch, "     %d маны\n", r.mana.getValue( ) ) : "")
            << (r.level > 0       ? fmt( ch, "     %1$d уров%1$Iень|ня|ней \n", r.level.getValue( ) ) : "")
            << (r.pretitle        ?          "     цветной претитул\n" : "");

        for (int i = 0; i < stat_table.size; i++)
            if (r.stats[i] > 0)
                str << fmt( ch, "     +%d к %s\n", 
                                r.stats[i].getValue( ), 
                                stat_table.message( i, '3' ).c_str( ) );
    }

    ch->send_to( str );
}


void CMlt::doCount( Character* ch, int n )
{
    std::basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator pos;
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    int cnt = 0;
    
    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;
        
        if (pcm->getRemorts( ).size( ) == (unsigned)n) {
            buf << pcm->getName( );
            
            buf << endl;
            cnt++;
        }
    }
    
    if (cnt > 80)
        ch->printf( "Их слишком много. %d тел.\r\n", cnt );
    else if (!buf.str( ).empty( )) {
        buf << "Count: " << cnt << endl;
        ch->send_to( buf );
    }
    else
        ch->send_to( "Никого нет.\r\n" );
}

