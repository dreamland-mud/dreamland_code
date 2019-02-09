#include "bandamobile.h"
#include "bigquest.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "act.h"
#include "merc.h"
#include "def.h"

BandaMobile::BandaMobile()
               : configured(false)
{
}

bool BandaMobile::death(Character *killer)
{
    PCMemoryInterface *pcm;
    BigQuest::Pointer quest;

    if (!( pcm = getHeroMemory( ) ))
        return false;

    if (!( quest = getMyQuest<BigQuest>( pcm ) ))
        return false;
    
    if (!pcm->isOnline()) {
        quest->mobDestroyed(pcm);
        return false;
    }

    killer = quest->getActor( killer );

    if (ourHero( killer )) {
        if (chance(33)) 
            killer->pecho("{YТы уничтожил%1$Gо||а очередную жертву, браво.{x", killer);
        else if (chance(50))
            killer->pecho("{YМолодец, тебе удалось избавить мир от очередной напасти.{x", killer);
        else
            killer->pecho("{YПротив тебя у %2$P4 не было никаких шансов!{x", killer, ch);
        quest->mobKilled(pcm, killer);
        return false;
    }

    if (ourHeroGroup(killer)) {
        killer->pecho("{YТы помог%1$Gло||ла согрупнику уничтожить очередную жертву, браво.{x", killer);
        pcm->getPlayer()->println("{YТвой согрупник помог тебе уничтожить очередную жертву.{x");
        quest->mobKilled(pcm, killer);
        return false;
    }

    pcm->getPlayer()->println("{YС кем-то из них что-то случилось без твоего участия.{x");
    quest->mobDestroyed(pcm);
    return false;
}

void BandaMobile::config(PCharacter *hero)
{
    if (configured)
        return;
    
    int level = hero->getModifyLevel();
    level += level / 10;

    ch->setLevel( level );
    ch->hitroll = level * 2;
    ch->damroll = (short) ( level * 3 / 2 ); 
    ch->max_mana = ch->mana = level * 10;
    ch->max_move = ch->move = 1000;

    if (level < 30)
        ch->max_hit = level * 20 + 2 * number_fuzzy(level);
    else if (level < 60)
        ch->max_hit = level * 50 + 10 * number_fuzzy(level);
    else
        ch->max_hit = level * 100 + 20 * number_fuzzy(level);

    ch->hit = ch->max_hit;
    ch->armor[0] = -level * 5;
    ch->armor[1] = -number_fuzzy(level) * 6; 
    ch->armor[2] = -number_fuzzy(level) * 6; 
    ch->armor[3] = -number_fuzzy(level) * 6; 
    ch->saving_throw = -level / 2;
    if (IS_NEUTRAL(hero))
        ch->alignment = number_range(-750, -350);
    else
        ch->alignment = -hero->alignment;

    configured = true;
}

void BandaMobile::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ))
        buf << "{R[ЦЕЛЬ] {x";
}

BandaItem::~BandaItem()
{
}


void BandaItem::getByOther( Character *ch ) 
{
    ch->pecho( "%1$O1 не принадлежит тебе, и ты бросаешь %1$P2.", obj );
}

void BandaItem::getByHero( PCharacter *ch ) 
{
    BigQuest::Pointer quest = getMyQuest<BigQuest>(ch);
    if (!quest)
        return;

    int carries = count_obj_list(obj->pIndexData, ch->carrying);

    if (carries == quest->objsTotal) {
        obj->getRoom()->echo(POS_RESTING, quest->getScenario().msgJoin.c_str());
        ch->pecho("Он вспыхивает и исчезает, оставив на своем месте {C1000{x очков опыта.");
        ch->gainExp(1000);
        quest->destroyItems<BandaItem>();
        return;
    }

    act( "Мерцающая аура окружает $o4.", ch, obj, 0, TO_CHAR );
}

