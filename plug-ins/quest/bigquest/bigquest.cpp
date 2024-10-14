#include "bigquest.h"
#include "bandamobile.h"
#include "questexceptions.h"
#include "profflags.h"

#include "player_utils.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "loadsave.h"

#include "merc.h"
#include "act.h"
#include "save.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * BigQuest
 *--------------------------------------------------------------------------*/
BigQuest::BigQuest( )
{
}

void BigQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    charName.setValue( pch->getName( ) );
    scenName = BigQuestRegistrator::getThis()->getWeightedRandomScenario(pch);
    const BigQuestScenario &scenario = getScenario();

    AreaList areas = findAreas(pch);
    if (areas.empty())
        throw QuestCannotStartException();

    struct AreaIndexData *targetArea = areas[number_range(0, areas.size() - 1)];

    RoomList rooms = findVictimRooms(pch, targetArea);
    if (rooms.size() < 5)
        throw QuestCannotStartException();

    mobsTotal = number_range(10, 15);
    mobsKilled = 0;
    mobsDestroyed = 0;
    objsTotal = 10;
    areaName = targetArea->getName();
    
    try {
        for (int m = 0, o = 0; m < mobsTotal; m++, o++) {
            Room *targetRoom = getRandomRoom(rooms);
            
            NPCharacter *mob= createMobile<BandaMobile>(28012);
            scenario.getRandomMobile().dress(mob);
            char_to_room(mob, targetRoom);

            if (o < objsTotal) {
                Object *obj = createItem<BandaItem>(28003);
                scenario.item.dress(obj);
                obj_to_char(obj, mob);
            }
        }
       
        scenario.onQuestStart(pch, questman, targetArea, mobsTotal); 
    } catch (const QuestCannotStartException &e) {
        destroy();
        throw e;
    }

    setTime( pch, 60 );

    wiznet( "", "%s, %d victims in area %s",
                 scenName.c_str(),
                 mobsTotal,
                 targetArea->getName().c_str());
}

const BigQuestScenario & BigQuest::getScenario( ) const
{
    return *BigQuestRegistrator::getThis()->getMyScenario<BigQuestScenario>( scenName );
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

QuestReward::Pointer BigQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );
    return r;
}

void BigQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( ))
        buf << "Твое задание {YВЫПОЛНЕНО{x!" << endl
            << "Сообщи об этом квестору командой {yквест сдать{x, до того как выйдет время!" << endl;
    else {
        getScenario().onQuestInfo(ch, mobsTotal, buf);
        buf << "Последний раз их видели в местности '{hh" << areaName << "{hx'." << endl;
        if (mobsKilled > 0)
            buf << "Уже убито " << mobsKilled << ", осталось совсем немного." << endl;
    }
}

void BigQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( ))
        buf << "Сообщить квестору о выполнении задания.";
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

    if (hero->isOnline()) {
        if (hasPartialRewards()) 
            hero->getPlayer()->pecho("{YТебе осталось уничтожить %d из %d, или же сообщить квестору о частичном выполнении задания.{x", 
                                       mobsLeft, mobsTotal.getValue());
        else
            hero->getPlayer()->pecho("{YТебе осталось уничтожить %d из %d.{x", mobsLeft, mobsTotal.getValue());
    }
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
        hero->getPlayer()->pecho("{YВсе разбежались или были уничтожены, сообщи квестору о выполнении задания.{x");
}

bool BigQuest::hasPartialRewards() const
{
    return mobsKilled >= (mobsTotal / 3);
}

bool BigQuest::checkRoomClient( PCharacter *pch, Room *room )
{
    if (RoomUtils::isWaterOrAir(room))
        return false;

    if (!VictimQuestModel::checkRoomClient( pch, room ))
        return false;

    return true;
}

/*----------------------------------------------------------------------------
 * BigQuestScenario
 *--------------------------------------------------------------------------*/
bool BigQuestScenario::applicable( PCharacter *ch ) const
{
    bool rc = criteria.allow(ch);
    return rc;
}

const QuestMobileAppearence & BigQuestScenario::getRandomMobile() const
{
    if (mobiles.empty())
        throw QuestCannotStartException();

    return mobiles[number_range(0, mobiles.size() - 1)];    
}

void BigQuestScenario::onQuestStart(PCharacter *pch, NPCharacter *questman, struct AreaIndexData *targetArea, int mobsTotal) const
{
    XMLStringVector::const_iterator s;

    for (s = msgStart.begin(); s != msgStart.end(); s++)
        tell_fmt(s->c_str(), pch, questman, targetArea->getName().c_str(), mobsTotal);            
}

void BigQuestScenario::onQuestInfo(PCharacter *pch, int mobsTotal, ostream &buf) const
{
    buf << fmt(0, msgInfo.c_str(), pch, 0, mobsTotal) << endl;
}

int BigQuestScenario::getPriority() const
{
    return priority;
}

/*----------------------------------------------------------------------------
 * BigQuestRegistrator
 *--------------------------------------------------------------------------*/
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
    return true;
}

