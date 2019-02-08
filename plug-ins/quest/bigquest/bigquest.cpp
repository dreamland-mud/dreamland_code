#include "bigquest.h"
#include "bigvictimbehavior.h"
#include "bandamobile.h"
#include "questexceptions.h"
#include "profflags.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "handler.h"
#include "mercdb.h"
#include "merc.h"
#include "act.h"
#include "save.h"
#include "def.h"

BigQuest::BigQuest( )
{
}

void BigQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    Room *pRoom;
    int time;
    MobileList victims;
    
    charName.setValue( pch->getName( ) );
    scenName = BigQuestRegistrator::getThis()->getWeightedRandomScenario(pch);
    BigQuestScenario &scenario = getScenario();

    AreaList areas = findAreas(pch);
    if (areas.empty())
        throw QuestCannotStartException();

    struct area_data *targetArea = areas[number_range(0, areas.size() - 1)];

    RoomList rooms = findClientRooms(pch, targetArea);
    if (rooms.size() < 5)
        throw QuestCannotStartException();

    mobsTotal = number_range(10, 15);
    mobsKilled = 0;
    mobsDestroyed = 0;
    objsTotal = 0;
    areaName = targetArea->name;

    for (int i = 0; i < mobsTotal; i++) {
        Room *targetRoom = getRandomRoom(rooms);
        
        NPCharacter *mob= createMobile<BandaMobile>(28012);
        scenario.getRandomMobile().dress(mob);
        char_to_room(mob, targetRoom);
    }
   
    scenario.onQuestStart(pch, questman, targetArea, mobsTotal); 
    setTime( pch, 60 );
}

BigQuestScenario & BigQuest::getScenario( )
{
    static BigQuestScenario zero;
    QuestScenario::Pointer scenario = BigQuestRegistrator::getThis()->getScenario( scenName );

    if (!scenario) 
        return zero;

    BigQuestScenario *bqs = scenario.getStaticPointer<BigQuestScenario>( );
    if (!bqs)
        return zero;

    return *bqs;
}

void BigQuest::destroy( ) 
{
    destroyMobiles<BandaMobile>( );
    destroyItems<BandaItem>( );
}

bool BigQuest::isComplete( ) 
{
    return (state == QSTAT_FINISHED);
}

Quest::Reward::Pointer BigQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    Reward::Pointer r( NEW );
    
    if (mobsKilled == mobsTotal)
        tell_fmt("Ты доблестно уничтожи%1$Gо||а всех преступников.", ch, questman);
    else 
        tell_fmt("Ты доблестно уничтожил%1$Gо||а %3$d преступник%3$Iа|ов|ов из %4$d.", 
                 ch, questman, mobsKilled.getValue(), mobsTotal.getValue());
    if (mobsDestroyed > 0)
        tell_fmt("Еще %3$d умерли неизвестной смертью.", ch, questman, mobsDestroyed.getValue());

    int objsCarried = getItemsList<BandaItem>(ch->carrying).size();
    if (objsCarried > 0) 
        tell_fmt("Я вижу, что тебе также удалось собрать %3$d доказательст%3$Iво|ва|в злостных намерений!",
                  ch, questman, objsCarried);

    
    r->gold = number_range( 8, 12 );
    r->points = number_range( 8, 10 ) + mobsKilled * number_range(2, 3) + objsCarried * number_range(1, 2);
    r->wordChance = 10;
    r->scrollChance = 10;
    if (chance(5))
        r->prac = number_range(1, 2);

    if (ch->getClan( )->isDispersed( )) 
        r->points *= 2;
    else
        r->clanpoints = r->points;

    r->exp = (r->points + r->clanpoints) * 10;
    return Reward::Pointer( r );
}

void BigQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( ))
        buf << "Твое задание {YВЫПОЛНЕНО{x!" << endl
            << "Вернись за вознаграждением, до того как выйдет время!" << endl;
    else {
        getScenario().onQuestInfo(ch, mobsTotal, buf);
        buf << "Последний раз их видели в местности '" << areaName << "'." << endl;
        if (mobsKilled > 0)
            buf << "Уже убито " << mobsKilled << ", осталось совсем немного." << endl;
    }
}

void BigQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( ))
        buf << "Вернуться к квестору за наградой.";
    else
        buf << fmt(0, "Уничтожить %1$d преступник%1$Iа|ов|ов из '%2$s' (убито %3$d).",
                    mobsTotal.getValue(), areaName.c_str(), mobsKilled.getValue());
}

Room * BigQuest::helpLocation( )
{
    return findMobileRoom<BandaMobile>( );
}


void BigQuest::mobKilled(PCMemoryInterface *hero, Character *killer)
{
    mobsKilled++;
// TODO count globally
    int mobsLeft = mobsTotal - mobsKilled - mobsDestroyed;

    if (mobsLeft <= 0) {
        state = QSTAT_FINISHED;
        notifyNoMore(hero);        
        return;
    }

    if (hero->isOnline())
        hero->getPlayer()->printf("{YОсталось уничтожить %d из %d.{x\r\n", mobsLeft, mobsTotal.getValue());
}

void BigQuest::mobDestroyed(PCMemoryInterface *hero)
{
// TODO count globally
    mobsDestroyed++;
    if (mobsKilled + mobsDestroyed >= mobsTotal) {
        state = QSTAT_FINISHED;
        notifyNoMore(hero);        
    }
}

void BigQuest::notifyNoMore(PCMemoryInterface *hero)
{
    if (hero->isOnline())
        hero->getPlayer()->println("{YВсе разбежались или были уничтожены, вернись за наградой.{x");
}

bool BigQuest::hasPartialRewards() const
{
    return mobsKilled >= (mobsTotal / 3);
}

bool BigQuestScenario::applicable( PCharacter *ch ) const
{
    bool rc = criteria.allow(ch);
    return rc;
}

QuestMobileAppearence & BigQuestScenario::getRandomMobile()
{
    if (mobiles.empty())
        throw QuestCannotStartException();

    return mobiles[number_range(0, mobiles.size() - 1)];    
}

void BigQuestScenario::onQuestStart(PCharacter *pch, NPCharacter *questman, struct area_data *targetArea, int mobsTotal)
{
    XMLStringVector::const_iterator s;

    for (s = msgStart.begin(); s != msgStart.end(); s++)
        tell_fmt(s->c_str(), pch, questman, targetArea->name, mobsTotal);            
}

void BigQuestScenario::onQuestInfo(PCharacter *pch, int mobsTotal, ostream &buf)
{
    buf << fmt(0, msgInfo.c_str(), pch, 0, mobsTotal) << endl;
}

int BigQuestScenario::getPriority() const
{
    return priority;
}

BigQuestRegistrator * BigQuestRegistrator::thisClass = 0;

BigQuestRegistrator::BigQuestRegistrator( )
{
    thisClass = this;
}

BigQuestRegistrator::~BigQuestRegistrator( )
{
    thisClass = 0;
}

bool BigQuestRegistrator::applicable( PCharacter *ch, bool fAuto ) const
{
    return ch->is_immortal() || ch->getAttributes( ).isAvailable( "tester" );
}

